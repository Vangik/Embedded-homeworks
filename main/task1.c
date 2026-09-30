#include <stdio.h>
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "pwm_hw.h"
#include "esp_adc/adc_oneshot.h"

static const char *TAG = "3.5-sg90";

#define SERVO_GPIO 18
#define US_AT_0 500
#define US_AT_180 2400
#define HOLD_MS 10

#define LOG_DELAY 1000000 /* esp_timer_get_time() is in microseconds: 1 s */

#define ADC_GPIO GPIO_NUM_4
#define ADC_BITWIDTH ADC_BITWIDTH_12
#define ADC_ATTEN ADC_ATTEN_DB_12
#define N_AVG 4

#define T 270
#define RAW_LEFT 0
#define RAW_RIGHT 4095
#define RAW_180 (RAW_LEFT + (RAW_RIGHT - RAW_LEFT) * 180 / T)

int64_t lastLog = 0;

static adc_oneshot_unit_handle_t s_adc;
static adc_channel_t s_channel;

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

static void setup_adc(void)
{
    adc_unit_t unit = 0;
    ESP_ERROR_CHECK(adc_oneshot_io_to_channel(ADC_GPIO, &unit, &s_channel));
    adc_oneshot_unit_init_cfg_t init = {
        .unit_id = unit,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init, &s_adc));

    adc_oneshot_chan_cfg_t ch = {
        .bitwidth = ADC_BITWIDTH,
        .atten = ADC_ATTEN,
    };
    ESP_ERROR_CHECK(adc_oneshot_config_channel(s_adc, s_channel, &ch));
}

static int read_raw_avg(void)
{
    int acc = 0;
    for (int i = 0; i < N_AVG; i++)
    {
        int raw = 0;
        ESP_ERROR_CHECK(adc_oneshot_read(s_adc, s_channel, &raw));
        acc += raw;
    }
    return acc / N_AVG;
}

static float adc_to_angle(int adc_raw)
{
    float a = (float)(adc_raw - RAW_LEFT) * 180.0f / (float)(RAW_180 - RAW_LEFT);
    if (a < 0.0f)
        a = 0.0f;
    if (a > 180.0f)
        a = 180.0f;
    return a;
}

void app_main(void)
{
    pwm_hw_init(SERVO_GPIO);
    servo_write_deg(90);
    ESP_LOGI(TAG, "GPIO%d LEDC regs 50 Hz 13 bit APB", (int)SERVO_GPIO);

    setup_adc();

    while (true)
    {
        const int raw = read_raw_avg();
        const float angle = adc_to_angle(raw);

        const int64_t now = esp_timer_get_time();
        if (now - lastLog >= LOG_DELAY)
        {
            lastLog = now;
            ESP_LOGI(TAG, "angle=%.1f deg (from left)", angle);
        }

        hold_deg((int)angle);
        
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}