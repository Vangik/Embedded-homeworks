#include <stdio.h>
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "esp_log.h"
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

// Октава 3
#define NOTE_C3  131
#define NOTE_CS3 139
#define NOTE_D3  147
#define NOTE_DS3 156
#define NOTE_E3  165
#define NOTE_F3  175
#define NOTE_FS3 185
#define NOTE_G3  196
#define NOTE_GS3 208
#define NOTE_A3  220
#define NOTE_AS3 233
#define NOTE_B3  247
// Октава 4
#define NOTE_C4  262
#define NOTE_CS4 277
#define NOTE_D4  294
#define NOTE_DS4 311
#define NOTE_E4  330
#define NOTE_F4  349
#define NOTE_FS4 370
#define NOTE_G4  392
#define NOTE_GS4 415
#define NOTE_A4  440
#define NOTE_AS4 466
#define NOTE_B4  494
// Октава 5
#define NOTE_C5  523
#define NOTE_CS5 554
#define NOTE_D5  587
#define NOTE_DS5 622
#define NOTE_E5  659
#define NOTE_F5  698
#define NOTE_FS5 740
#define NOTE_G5  784
#define NOTE_GS5 831
#define NOTE_A5  880
#define NOTE_AS5 932
#define NOTE_B5  988
#define REST     0

typedef struct
{
    int hz;
    int ms;
} note_t;

static const note_t k_theme[] = {
    // "Фраза 1" — висхідний біг угору
    {NOTE_C4, 150},
    {NOTE_E4, 150},
    {NOTE_G4, 150},
    {NOTE_C5, 300},
    {NOTE_G4, 150},
    {NOTE_C5, 450},
    {REST,    150},

    // "Фраза 2" — відповідь зверху вниз
    {NOTE_A4, 150},
    {NOTE_F4, 150},
    {NOTE_D4, 150},
    {NOTE_G4, 300},
    {NOTE_E4, 150},
    {NOTE_C4, 450},
    {REST,    300},

    // "Фраза 3" — коротка стрибуча
    {NOTE_E4, 150},
    {NOTE_E4, 150},
    {NOTE_G4, 150},
    {NOTE_G4, 150},
    {NOTE_A4, 300},
    {NOTE_G4, 300},

    // Кінцівка
    {NOTE_F4, 150},
    {NOTE_E4, 150},
    {NOTE_D4, 300},
    {NOTE_C4, 600},
    {REST,    400},
};

static void buzz_off(void)
{
    ESP_ERROR_CHECK(ledc_set_duty(PWM_MODE, PWM_CHANNEL, 0));
    ESP_ERROR_CHECK(ledc_update_duty(PWM_MODE, PWM_CHANNEL));
}

static void play_note(int hz, int ms)
{
    if (hz <= 0)
    {
        buzz_off();
    }
    else
    {
        ESP_ERROR_CHECK(ledc_set_freq(PWM_MODE, PWM_TIMER, (uint32_t)hz));
        ESP_ERROR_CHECK(ledc_set_duty(PWM_MODE, PWM_CHANNEL, PWM_DUTY_HALF));
        ESP_ERROR_CHECK(ledc_update_duty(PWM_MODE, PWM_CHANNEL));
        ESP_LOGI(TAG, "%d Hz %d ms", hz, ms);
    }
    vTaskDelay(pdMS_TO_TICKS(ms));
    buzz_off();
    vTaskDelay(pdMS_TO_TICKS(20));
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
        for (size_t i = 0; i < sizeof(k_theme) / sizeof(k_theme[0]); i++)
        {
            play_note(k_theme[i].hz, k_theme[i].ms);
        }
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}