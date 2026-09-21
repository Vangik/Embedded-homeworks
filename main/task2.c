
#include "driver/gpio.h"
#include "esp_intr_alloc.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdbool.h>
#include <stdio.h>

static const char *TAG = "2.4-irq";

#define BTN_GPIO GPIO_NUM_21

#define DEBOUNCE_MS 50

static volatile bool btn_irq_pending = false;
static volatile TickType_t last_irq_tick = 0;
static uint32_t press_count = 0;

static void IRAM_ATTR gpio_isr_handler(void *arg) {
  (void)arg;
  last_irq_tick = xTaskGetTickCountFromISR();
  btn_irq_pending = true;
}

static void setup_button_irq(void) {
  gpio_config_t io = {
      .pin_bit_mask = 1ULL << BTN_GPIO, /* маска: біт N = GPIO N */
      .mode = GPIO_MODE_INPUT,
      .pull_up_en = GPIO_PULLUP_ENABLE, /* софтверний pull-up */
      .pull_down_en = GPIO_PULLDOWN_DISABLE,
      .intr_type = GPIO_INTR_ANYEDGE, /* фронт натиску будь який */
  };
  ESP_ERROR_CHECK(gpio_config(&io));
  ESP_ERROR_CHECK(gpio_install_isr_service(ESP_INTR_FLAG_IRAM));
  ESP_ERROR_CHECK(gpio_isr_handler_add(BTN_GPIO, gpio_isr_handler, NULL));
}

/* Debounce у задачі: підтвердити натиск, toggle, дочекатися відпускання */
static void handle_button_if_needed(void) {
  if (!btn_irq_pending) {
    return;
  }

  static TickType_t last_accepted = 0;

  btn_irq_pending = false;

  TickType_t now = last_irq_tick;

  /* press_count: перший натиск після старту не відсікаємо */
  if (press_count && (now - last_accepted) < pdMS_TO_TICKS(DEBOUNCE_MS))
    return;

  last_accepted = now;
  press_count++;

  ESP_LOGI(TAG, "press #%lu led=%s", (unsigned long)press_count, "click");

  btn_irq_pending = false;
}

void app_main(void) {
  setup_button_irq();
  ESP_LOGI(TAG, "ready: BTN%d", (int)BTN_GPIO);
  while (1) {
    handle_button_if_needed();
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}