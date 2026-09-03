#include <stdio.h>
#include <stdbool.h>
#include "driver/gpio.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "2.4-poll-fsm";

#define BTN_GPIO GPIO_NUM_21
#define POLL_MS 10      // опитуємо кожні 5 мс
#define DEBOUNCE_MS 40 // стільки стан має бути стабільним

static uint32_t press_count = 0;

typedef enum
{
    BTN_RELEASED,   // кнопка відпущена, чекаємо натиску
    BTN_DEBOUNCING, // побачили натиск, перевіряємо що це не брязкіт
    BTN_PRESSED,    // натиск підтверджено, чекаємо відпускання
} btn_state_t;

static void setup_button(void)
{
    gpio_config_t io = {
        .pin_bit_mask = 1ULL << BTN_GPIO,
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE, // pull-up: не натиснута = 1
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE, // переривання ВИМКНені — це polling
    };
    ESP_ERROR_CHECK(gpio_config(&io));
}

void app_main(void)
{
    setup_button();
    ESP_LOGI(TAG, "ready: BTN%d (polling FSM)", (int)BTN_GPIO);

    btn_state_t state = BTN_RELEASED;
    TickType_t t_change = 0; // коли ми зайшли в DEBOUNCING

    while (1)
    {
        int level = gpio_get_level(BTN_GPIO); // 0 = натиснута, 1 = відпущена
        bool pressed = (level == 0);
        TickType_t now = xTaskGetTickCount();

        switch (state)
        {
        case BTN_RELEASED:
            if (pressed) // побачили можливий натиск
            {
                state = BTN_DEBOUNCING;
                t_change = now; // запускаємо відлік стабільності
            }
            break;

        case BTN_DEBOUNCING:
            if (!pressed) // відпустили під час перевірки — брязкіт
            {
                state = BTN_RELEASED; // повертаємось, натиску не було
            }
            else if ((now - t_change) >= pdMS_TO_TICKS(DEBOUNCE_MS))
            {
                // пін тримається у 0 вже DEBOUNCE_MS підряд — це реальний натиск
                state = BTN_PRESSED;
                press_count++;
                ESP_LOGI(TAG, "press #%lu", (unsigned long)press_count);
            }
            break;

        case BTN_PRESSED:
            if (!pressed) // кнопку відпустили
            {
                state = BTN_RELEASED; // готові до наступного натиску
            }
            break;
        }

        vTaskDelay(pdMS_TO_TICKS(POLL_MS)); // крок опитування, віддаємо CPU
    }
}