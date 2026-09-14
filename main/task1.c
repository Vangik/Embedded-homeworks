#include <stdio.h>
#include <stdbool.h>
#include "driver/gpio.h"
#include "driver/gptimer.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "esp_task_wdt.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "2.5-tmr";

#define TIMER_RESOLUTION_HZ 1000000u
#define POLL_PERIOD_MS 1000

#define FAN_ALARM_PERIOD 3600000000ull
#define FAN_WORK_TIME 900000000ull

#define RELAY_PIN GPIO_NUM_23
#define LED_PIN GPIO_NUM_5
#define LED2_PIN GPIO_NUM_19

static gptimer_handle_t gptimer_alarm;

static volatile bool fan_running = false;

static bool IRAM_ATTR on_timer_alarm(gptimer_handle_t timer,
                                     const gptimer_alarm_event_data_t *edata,
                                     void *user_ctx)
{
    (void)timer;
    (void)edata;
    (void)user_ctx;
    alarm_count++;

    fan_running = !fan_running;
    uint64_t next;

    if (fan_running)
    {
        gpio_set_level(RELAY_PIN, 1);
        gpio_set_level(LED2_PIN, 1);
        gpio_set_level(LED_PIN, 0);
        next = FAN_WORK_TIME; 
    }
    else
    {
        gpio_set_level(RELAY_PIN, 0);
        gpio_set_level(LED2_PIN, 0);
        gpio_set_level(LED_PIN, 1);
        next = FAN_ALARM_PERIOD;
    }

    gptimer_alarm_config_t alarm = {
        .reload_count = 0,
        .alarm_count = next,
        .flags.auto_reload_on_alarm = true,
    };
    gptimer_set_alarm_action(gptimer_alarm, &alarm);
    return false;
}

static void setup_relay(void)
{
    gpio_reset_pin(RELAY_PIN);
    gpio_set_direction(RELAY_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(RELAY_PIN, 0);
}

static void setup_leds(void)
{
    gpio_reset_pin(LED_PIN);
    gpio_set_direction(LED_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(LED_PIN, 0);

    gpio_reset_pin(LED2_PIN);
    gpio_set_direction(LED2_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(LED2_PIN, 0);
}

static void setup_gptimer_alarm(void)
{
    gptimer_config_t config = {
        .clk_src = GPTIMER_CLK_SRC_DEFAULT,
        .direction = GPTIMER_COUNT_UP,
        .resolution_hz = TIMER_RESOLUTION_HZ,
    };

    ESP_ERROR_CHECK(gptimer_new_timer(&config, &gptimer_alarm));

    gptimer_event_callbacks_t cbs = {
        .on_alarm = on_timer_alarm,
    };

    ESP_ERROR_CHECK(gptimer_register_event_callbacks(gptimer_alarm, &cbs, NULL));

    gptimer_alarm_config_t alarm = {
        .reload_count = 0,
        .alarm_count = FAN_ALARM_PERIOD,
        .flags.auto_reload_on_alarm = true,
    };

    ESP_ERROR_CHECK(gptimer_set_alarm_action(gptimer_alarm, &alarm));
    ESP_ERROR_CHECK(gptimer_enable(gptimer_alarm));
    ESP_ERROR_CHECK(gptimer_start(gptimer_alarm));
}

void setupAll(void)
{
    setup_relay();
    setup_leds();
    setup_gptimer_alarm();
    ESP_LOGI(TAG, "Starting main loop, initial alarm_count=%u", alarm_count);
}

void app_main(void)
{
    setupAll();

    gpio_set_level(LED_PIN, 1);

    esp_task_wdt_config_t wdt = {
        .timeout_ms = 5000,
        .idle_core_mask = 0,
        .trigger_panic = true,
    };
    esp_task_wdt_init(&wdt);
    esp_task_wdt_add(NULL);

    while (1)
    {
        ESP_LOGI(TAG, "reley=%d, led=%d, led2=%d, alarm_count=%u",
                 gpio_get_level(RELAY_PIN),
                 gpio_get_level(LED_PIN),
                 gpio_get_level(LED2_PIN),
                 alarm_count);

        esp_task_wdt_reset();
        vTaskDelay(pdMS_TO_TICKS(POLL_PERIOD_MS));
    }
}