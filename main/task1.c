#include <stdio.h>
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "pwm-march";

#define BUZZ_GPIO GPIO_NUM_4

#define PWM_TIMER LEDC_TIMER_0

#define PWM_CHANNEL LEDC_CHANNEL_0

#define PWM_MODE LEDC_LOW_SPEED_MODE

#define PWM_RES LEDC_TIMER_8_BIT

#define PWM_DUTY_HALF 128
#define REST 0

unsigned long lastPlay = 0;
unsigned long playTime = 1000000; // 1 second in microseconds
bool isPlaying = false;

static void buzz_off(void)
{
    ESP_ERROR_CHECK(ledc_set_duty(PWM_MODE, PWM_CHANNEL, 0));
    ESP_ERROR_CHECK(ledc_update_duty(PWM_MODE, PWM_CHANNEL));
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

void app_main(void)
{
    setup_pwm();
    buzz_off();
    ESP_LOGI(TAG, "==== PWM IMPERIAL MARCH GPIO%d ====", (int)BUZZ_GPIO);

    while (1)
    {
        int64_t now = esp_timer_get_time();

        if (now - lastPlay >= playTime)
        {
            lastPlay = now;        
            isPlaying = !isPlaying; 

            if (isPlaying)
            {
                ESP_LOGI(TAG, "ON");
                ESP_ERROR_CHECK(ledc_set_freq(PWM_MODE, PWM_TIMER, 100));
                ESP_ERROR_CHECK(ledc_set_duty(PWM_MODE, PWM_CHANNEL, PWM_DUTY_HALF));
                ESP_ERROR_CHECK(ledc_update_duty(PWM_MODE, PWM_CHANNEL));
            }
            else
            {
                ESP_LOGI(TAG, "OFF");
                buzz_off();
            }
        }

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}