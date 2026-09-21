
#include <stdio.h>
#include <stdbool.h>
#include "driver/gpio.h"
#include "esp_intr_alloc.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"


static const char *TAG = "2.4-irq";

#define BTN_GPIO GPIO_NUM_21

/* Завдання 1: без debounce — рахуємо кожен фронт прямо в ISR */
static volatile uint32_t press_count = 0;


/* ISR: лише сигнал, без debounce і без delay */
static void IRAM_ATTR gpio_isr_handler(void *arg)
{
    (void)arg;
    press_count++;
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

void app_main(void)
{
    setup_button_irq();
    unsigned long last_press_count = 0;
    ESP_LOGI(TAG, "ready: BTN%d", (int)BTN_GPIO);
    while (1)
    {
        /* друкуємо КОЖЕН фронт, а не лише останній — видно пачку на один клік */
        while (last_press_count != press_count)
        {
            last_press_count++;
            ESP_LOGI(TAG, "irq #%lu", last_press_count);
        }

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}