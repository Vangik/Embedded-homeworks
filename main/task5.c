#include <stdio.h>
#include <stdbool.h>
#include "driver/gpio.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "2.5-rc-test";

#define BTN_GPIO GPIO_NUM_21
#define POLL_MS  10     // опитуємо часто, щоб ЛОВИТИ брязкіт (не фільтрувати його)

static uint32_t edge_count = 0;

static void setup_button(void)
{
    gpio_config_t io = {
        .pin_bit_mask = 1ULL << BTN_GPIO,
        .mode = GPIO_MODE_INPUT,
        // ВАЖЛИВО: pull-up лишаємо, бо він потрібен разом із RC (пояснення нижче)
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    ESP_ERROR_CHECK(gpio_config(&io));
}

void app_main(void)
{
    setup_button();
    ESP_LOGI(TAG, "ready: BTN%d (RC test, NO software debounce)", (int)BTN_GPIO);

    int prev = 1;   // попередній рівень; 1 = відпущена (pull-up)

    while (1)
    {
        int level = gpio_get_level(BTN_GPIO);

        // рахуємо КОЖЕН перехід 1->0 (спадний фронт = "натиск")
        // без RC брязкіт дасть багато фронтів на одне натискання
        // з RC брязкіт згладиться і фронтів буде менше (в ідеалі один)
        if (prev == 1 && level == 0)
        {
            edge_count++;
            ESP_LOGI(TAG, "edge #%lu", (unsigned long)edge_count);
        }
        prev = level;

        vTaskDelay(pdMS_TO_TICKS(POLL_MS));
    }
}