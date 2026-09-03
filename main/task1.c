
#include <stdio.h>
#include <stdbool.h>
#include "driver/gpio.h"
#include "esp_intr_alloc.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"


static const char *TAG = "2.4-irq";

#define BTN_GPIO GPIO_NUM_21

#define DEBOUNCE_MS 40
#define RELEASE_GUARD_MS 40



static volatile bool btn_irq_pending = false; /* прапорець з ISR */
static uint32_t press_count = 0;


/* ISR: лише сигнал, без debounce і без delay */
static void IRAM_ATTR gpio_isr_handler(void *arg)
{
    (void)arg;
    btn_irq_pending = true;
}


static void setup_button_irq(void)
{
    gpio_config_t io = {
        .pin_bit_mask = 1ULL << BTN_GPIO, /* маска: біт N = GPIO N */
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE, /* софтверний pull-up */
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_NEGEDGE, /* фронт натиску (на GND) */
    };
    ESP_ERROR_CHECK(gpio_config(&io));
    ESP_ERROR_CHECK(gpio_install_isr_service(ESP_INTR_FLAG_IRAM));
    ESP_ERROR_CHECK(gpio_isr_handler_add(BTN_GPIO, gpio_isr_handler, NULL));
}
/* Debounce у задачі: підтвердити натиск, toggle, дочекатися відпускання */
static void handle_button_if_needed(void)
{
    if (!btn_irq_pending)
    {
        return;
    }

    btn_irq_pending = false;
    press_count++;

    ESP_LOGI(TAG, "press #%lu led=%s",
             (unsigned long)press_count,
             "click");
}
void app_main(void)
{
    setup_button_irq();
    ESP_LOGI(TAG, "ready: BTN%d", (int)BTN_GPIO);
    while (1)
    {
        handle_button_if_needed();
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}