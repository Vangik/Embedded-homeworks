#include <stdio.h>
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "pwm_hw.h"
#include "driver/pulse_cnt.h"
#include "driver/ledc.h"

static const char *TAG = "3.7-sg90";

#define GPIO_A GPIO_NUM_19
#define GPIO_B GPIO_NUM_21
#define BTN_GPIO GPIO_NUM_18
#define BUZZ_GPIO GPIO_NUM_15
#define SERVO_GPIO GPIO_NUM_5
#define LED1 GPIO_NUM_22

#define PPR 20
#define DECODE_X 4
#define STEPS_PER_REV (PPR * DECODE_X)
#define SAMPLE_MS 200
#define GLITCH_NS 1000

#define US_AT_0 500
#define US_AT_180 2400
#define HOLD_MS 10

#define LOG_DELAY 1000000

#define SERVO_STEP1 10.0f
#define SERVO_STEP2 5.0f

#define LONG_PRESS_DELAY_MS 2000

#define PWM_TIMER LEDC_TIMER_1
#define PWM_CHANNEL LEDC_CHANNEL_1
#define PWM_MODE LEDC_LOW_SPEED_MODE
#define PWM_RES LEDC_TIMER_8_BIT
#define PWM_DUTY_HALF 32

int64_t lastLog = 0;

static pcnt_unit_handle_t s_pcnt;

static volatile bool btn_irq_pending = false;
bool isRotationReduced = false;
bool returnToCenter = false;
bool playBuzzer = false;
bool edgeLed = false;

static float angle = 90.0f;

static void IRAM_ATTR gpio_isr_handler(void *arg)
{
    (void)arg;
    btn_irq_pending = true;
}

static void setup_button_irq(void)
{
    gpio_config_t io = {
        .pin_bit_mask = 1ULL << BTN_GPIO,
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_NEGEDGE,
    };
    ESP_ERROR_CHECK(gpio_config(&io));
    ESP_ERROR_CHECK(gpio_install_isr_service(ESP_INTR_FLAG_IRAM));
    ESP_ERROR_CHECK(gpio_isr_handler_add(BTN_GPIO, gpio_isr_handler, NULL));
}

static void setup_leds(void)
{
    gpio_reset_pin(LED1);
    gpio_set_direction(LED1, GPIO_MODE_OUTPUT);
    gpio_set_level(LED1, 0);
}

static void buzz_off(void)
{
    ESP_ERROR_CHECK(ledc_set_duty(PWM_MODE, PWM_CHANNEL, 0));
    ESP_ERROR_CHECK(ledc_update_duty(PWM_MODE, PWM_CHANNEL));
}

static void buzz_on(void)
{
    ESP_ERROR_CHECK(ledc_set_duty(PWM_MODE, PWM_CHANNEL, PWM_DUTY_HALF));
    ESP_ERROR_CHECK(ledc_update_duty(PWM_MODE, PWM_CHANNEL));
}

static void led_on(gpio_num_t gpio)
{
    ESP_ERROR_CHECK(gpio_set_level(gpio, 1));
}

static void led_off(gpio_num_t gpio)
{
    ESP_ERROR_CHECK(gpio_set_level(gpio, 0));
}

static void setup_pwm(void)
{
    ledc_timer_config_t timer = {
        .speed_mode = PWM_MODE,
        .duty_resolution = PWM_RES,
        .timer_num = PWM_TIMER,
        .freq_hz = 2000,
        .clk_cfg = LEDC_AUTO_CLK,
    };

    ESP_ERROR_CHECK(ledc_timer_config(&timer));

    ledc_channel_config_t ch = {
        .gpio_num = BUZZ_GPIO,
        .speed_mode = PWM_MODE,
        .channel = PWM_CHANNEL,
        .timer_sel = PWM_TIMER,
        .duty = 0,
        .hpoint = 0,
        .sleep_mode = LEDC_SLEEP_MODE_KEEP_ALIVE,
        .flags.output_invert = 0,
    };
    ESP_ERROR_CHECK(ledc_channel_config(&ch));
}

static uint32_t deg_to_us(int deg)
{
    if (deg < 0)
    {
        deg = 0;
    }
    if (deg > 180)
    {
        deg = 180;
    }
    return US_AT_0 + ((uint32_t)deg * (US_AT_180 - US_AT_0)) / 180;
}

static void servo_write_deg(int deg)
{
    pwm_hw_set_pulse_us(deg_to_us(deg));
}

static void hold_deg(int deg)
{
    servo_write_deg(deg);
    vTaskDelay(pdMS_TO_TICKS(HOLD_MS));
}

static float encoder_ticks_to_angle(int ticks)
{
    static int acc = 0;
    float step = isRotationReduced ? SERVO_STEP2 : SERVO_STEP1;

    acc += ticks;

    while (acc >= DECODE_X)
    {
        acc -= DECODE_X;
        angle += step;
        if (angle > 180.0f)
        {
            angle = 180.0f;
            playBuzzer = true;
            edgeLed = true;
        }
    }
    while (acc <= -DECODE_X)
    {
        acc += DECODE_X;
        angle -= step;
        if (angle < 0.0f)
        {
            angle = 0.0f;
            playBuzzer = true;
            edgeLed = true;
        }
    }

    return angle;
}

static void pcnt_encoder_init(void)
{
    pcnt_unit_config_t unit_cfg = {
        .high_limit = 32767,
        .low_limit = -32768,
    };
    ESP_ERROR_CHECK(pcnt_new_unit(&unit_cfg, &s_pcnt));
    pcnt_glitch_filter_config_t filter_cfg = {
        .max_glitch_ns = GLITCH_NS,
    };
    ESP_ERROR_CHECK(pcnt_unit_set_glitch_filter(s_pcnt, &filter_cfg));
    pcnt_chan_config_t ch_a_cfg = {
        .edge_gpio_num = GPIO_A,
        .level_gpio_num = GPIO_B,
    };
    pcnt_channel_handle_t ch_a;

    ESP_ERROR_CHECK(pcnt_new_channel(s_pcnt, &ch_a_cfg, &ch_a));
    ESP_ERROR_CHECK(pcnt_channel_set_edge_action(
        ch_a, PCNT_CHANNEL_EDGE_ACTION_DECREASE, PCNT_CHANNEL_EDGE_ACTION_INCREASE));
    ESP_ERROR_CHECK(pcnt_channel_set_level_action(
        ch_a, PCNT_CHANNEL_LEVEL_ACTION_KEEP, PCNT_CHANNEL_LEVEL_ACTION_INVERSE));
    pcnt_chan_config_t ch_b_cfg = {
        .edge_gpio_num = GPIO_B,
        .level_gpio_num = GPIO_A,
    };
    pcnt_channel_handle_t ch_b;
    ESP_ERROR_CHECK(pcnt_new_channel(s_pcnt, &ch_b_cfg, &ch_b));
    ESP_ERROR_CHECK(pcnt_channel_set_edge_action(
        ch_b, PCNT_CHANNEL_EDGE_ACTION_INCREASE, PCNT_CHANNEL_EDGE_ACTION_DECREASE));
    ESP_ERROR_CHECK(pcnt_channel_set_level_action(
        ch_b, PCNT_CHANNEL_LEVEL_ACTION_KEEP, PCNT_CHANNEL_LEVEL_ACTION_INVERSE));
    ESP_ERROR_CHECK(pcnt_unit_enable(s_pcnt));
    ESP_ERROR_CHECK(pcnt_unit_clear_count(s_pcnt));
    ESP_ERROR_CHECK(pcnt_unit_start(s_pcnt));
}

static void handle_button_if_needed(void)
{
    static bool pressed = false;
    static int64_t press_start_time = 0;
    static bool action_executed = false;

    bool is_low = (gpio_get_level(BTN_GPIO) == 0);

    if (pressed)
    {
        btn_irq_pending = false;

        if (is_low)
        {
            int64_t elapsed = (esp_timer_get_time() - press_start_time) / 1000;

            if (!action_executed && (elapsed >= LONG_PRESS_DELAY_MS))
            {
                action_executed = true;
                returnToCenter = true;
                ESP_LOGI(TAG, "Long press detected! led=%s", "toggle");
            }
        }
        else
        {
            if (!action_executed)
                isRotationReduced = !isRotationReduced;
            pressed = false;
            action_executed = false;
            returnToCenter = false;
            ESP_LOGI(TAG, "Button released");
        }
        return;
    }

    if (!btn_irq_pending)
    {
        return;
    }
    btn_irq_pending = false;

    if (!is_low)
    {
        return;
    }

    pressed = true;
    action_executed = false;
    press_start_time = esp_timer_get_time();
}

void app_main(void)
{
    setup_pwm();
    buzz_off();

    pwm_hw_init(SERVO_GPIO);
    servo_write_deg(90);
    ESP_LOGI(TAG, "GPIO%d LEDC regs 50 Hz 13 bit APB", (int)SERVO_GPIO);

    setup_button_irq();

    setup_leds();
    pcnt_encoder_init();

    ESP_LOGI(TAG, "PCNT X4 CLK=GPIO%d DT=GPIO%d PPR=%d", (int)GPIO_A, (int)GPIO_B, PPR);
    int pos = 0;

    while (true)
    {
        int d = 0;

        ESP_ERROR_CHECK(pcnt_unit_get_count(s_pcnt, &d));
        ESP_ERROR_CHECK(pcnt_unit_clear_count(s_pcnt));
        pos += d;
        const char *dir = (d > 0) ? "+" : (d < 0) ? "-"
                                                  : "0";
        const int rpm = (d * 60 * 1000) / (STEPS_PER_REV * SAMPLE_MS);
        ESP_LOGI(TAG, "pos=%d d=%+d dir=%s rpm=%d", pos, d, dir, rpm);

        encoder_ticks_to_angle(d);

        const int64_t now = esp_timer_get_time();
        if (now - lastLog >= LOG_DELAY)
        {
            lastLog = now;
            ESP_LOGI(TAG, "angle=%.1f deg (from left)", angle);
        }

        if (returnToCenter)
        {
            angle = 90.0f;
            returnToCenter = false;
        }

        if (playBuzzer)
        {
            buzz_on();
            vTaskDelay(pdMS_TO_TICKS(60));
            buzz_off();
            playBuzzer = false;
        }

        if (edgeLed)
        {
            led_on(LED1);
            vTaskDelay(pdMS_TO_TICKS(500));
            led_off(LED1);
            edgeLed = false;
        }

        hold_deg((int)angle);

        handle_button_if_needed();

        vTaskDelay(pdMS_TO_TICKS(SAMPLE_MS));
    }
}
