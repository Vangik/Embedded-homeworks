#include <stdio.h>
#include <stdbool.h>
#include <math.h>

#include "driver/gpio.h"
#include "driver/gptimer.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_timer.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "config.h"

static const char *TAG = "m2-traffic";

/* ---------- Стани денного циклу ---------- */
typedef enum {
    ST_GREEN,
    ST_GREEN_BLINK,
    ST_YELLOW,
    ST_RED,
    ST_RED_YELLOW,
    ST_COUNT
} traffic_state_t;

typedef struct {
    uint32_t dur_ms;
    bool r, y, g;
    bool blink;
} state_cfg_t;

static const state_cfg_t TABLE[ST_COUNT] = {
    /* ST_GREEN       */ { T_GREEN,       false, false, true,  false },
    /* ST_GREEN_BLINK */ { T_GREEN_BLINK, false, false, true,  true  },
    /* ST_YELLOW      */ { T_YELLOW,      false, true,  false, false },
    /* ST_RED         */ { T_RED,         true,  false, false, false },
    /* ST_RED_YELLOW  */ { T_RED_YELLOW,  true,  true,  false, false },
};

/* ---------- Спільне з ISR ---------- */
static volatile bool btn_irq_pending = false;

/* ---------- ADC ---------- */
static adc_oneshot_unit_handle_t adc_handle;
static adc_channel_t             ldr_channel;

/* ---------- Buzzer через GPTimer (без LEDC) ---------- */
static gptimer_handle_t buzzer_timer;
static volatile bool    buzzer_on = false;   // спільне з ISR -> volatile

/* ---------- Прототипи ---------- */
static void setup_one_led(int pin);
static void setup_button_irq(void);
static void setup_ldr_adc(void);
static void setup_buzzer(void);

static void IRAM_ATTR gpio_isr_handler(void *arg)
{
    (void)arg;
    btn_irq_pending = true;      // ISR — лише прапорець
}

/* ISR таймера buzzer: перемикає пін -> квадратна хвиля = тон.
   Спрацьовує щопівперіоду; якщо buzzer_on=false — тримає пін у 0 (тиша). */
static bool IRAM_ATTR buzzer_timer_cb(gptimer_handle_t timer,
                                      const gptimer_alarm_event_data_t *edata,
                                      void *user_ctx)
{
    (void)timer; (void)edata; (void)user_ctx;
    static int level = 0;
    level = buzzer_on ? (level ^ 1) : 0;   // тон -> перемикаємо, тиша -> 0
    gpio_set_level(BUZZER_PIN, level);
    return false;
}

/* ==================== Налаштування заліза ==================== */
static void setup_one_led(int pin)
{
    gpio_reset_pin((gpio_num_t)pin);
    gpio_set_direction((gpio_num_t)pin, GPIO_MODE_OUTPUT);
    gpio_set_level((gpio_num_t)pin, 0);
}

static void setup_button_irq(void)
{
    gpio_config_t io = {
        .pin_bit_mask = 1ULL << BTN_PIN,
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,     // кнопка на GND -> у спокої HIGH
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_NEGEDGE,       // натиск = спадний фронт
    };
    ESP_ERROR_CHECK(gpio_config(&io));
    ESP_ERROR_CHECK(gpio_install_isr_service(ESP_INTR_FLAG_IRAM));
    ESP_ERROR_CHECK(gpio_isr_handler_add(BTN_PIN, gpio_isr_handler, NULL));
}

static void setup_ldr_adc(void)
{
    /* GPIO -> (ADC unit + channel) автоматично, під будь-яку плату */
    adc_unit_t unit;
    ESP_ERROR_CHECK(adc_oneshot_io_to_channel(LDR_PIN, &unit, &ldr_channel));

    adc_oneshot_unit_init_cfg_t ucfg = { .unit_id = unit };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&ucfg, &adc_handle));

    adc_oneshot_chan_cfg_t ccfg = {
        .atten    = ADC_ATTEN_DB_12,  // повний діапазон ~0..3.3V (старі IDF: DB_11)
        .bitwidth = ADC_BITWIDTH_12,
    };
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc_handle, ldr_channel, &ccfg));
}

static void setup_buzzer(void)
{
    /* пін buzzer як звичайний вихід — смикати його буде ISR таймера */
    gpio_reset_pin((gpio_num_t)BUZZER_PIN);
    gpio_set_direction((gpio_num_t)BUZZER_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level((gpio_num_t)BUZZER_PIN, 0);

    gptimer_config_t cfg = {
        .clk_src       = GPTIMER_CLK_SRC_DEFAULT,
        .direction     = GPTIMER_COUNT_UP,
        .resolution_hz = 1000000,              // 1 тік = 1 мкс
    };
    ESP_ERROR_CHECK(gptimer_new_timer(&cfg, &buzzer_timer));

    gptimer_event_callbacks_t cbs = { .on_alarm = buzzer_timer_cb };
    ESP_ERROR_CHECK(gptimer_register_event_callbacks(buzzer_timer, &cbs, NULL));

    /* перемикаємо пін щопівперіоду -> частота хвилі = BUZZER_FREQ_HZ */
    uint64_t half_period_us = 1000000ULL / (2ULL * BUZZER_FREQ_HZ);
    gptimer_alarm_config_t al = {
        .reload_count = 0,
        .alarm_count  = half_period_us,
        .flags.auto_reload_on_alarm = true,    // періодично
    };
    ESP_ERROR_CHECK(gptimer_set_alarm_action(buzzer_timer, &al));
    ESP_ERROR_CHECK(gptimer_enable(buzzer_timer));
    ESP_ERROR_CHECK(gptimer_start(buzzer_timer));  // таймер біжить завжди
}

/* ==================== Buzzer ==================== */
static void buzzer_tone(bool on)
{
    /* лише вмикаємо/вимикаємо тон; хвилю робить ISR таймера */
    buzzer_on = on;
}

/* ==================== LDR -> lux ==================== */
static float read_lux(void)
{
    int raw = 0;
    adc_oneshot_read(adc_handle, ldr_channel, &raw);   // &raw — вихідний параметр

    float vout = ((float)raw / ADC_MAX) * VCC;         // напруга на NODE
    if (vout <= 0.0f)   vout = 0.0001f;                 // захист від /0
    if (vout >= VCC)    vout = VCC - 0.0001f;

    /* опір LDR з дільника напруги */
    float rldr;
#if LDR_ON_TOP
    /* 3.3 -[LDR]-NODE-[R1]-GND :  vout = VCC * R1/(R1+Rldr) */
    rldr = R1 * (VCC - vout) / vout;
#else
    /* 3.3 -[R1]-NODE-[LDR]-GND :  vout = VCC * Rldr/(R1+Rldr) */
    rldr = R1 * vout / (VCC - vout);
#endif
    if (rldr < 1.0f) rldr = 1.0f;

    /* GL5528: R = RL10*(lux/10)^-GAMMA  =>  lux = 10*(RL10/R)^(1/GAMMA) */
    float lux = 10.0f * powf(RL10 / rldr, 1.0f / GAMMA);
    return lux;
}

/* нічний режим з гістерезисом (памʼятає попередній стан) */
static bool is_night(float lux)
{
    static bool night = false;
    if (lux < LUX_DARK_ON)       night = true;   // стало темно
    else if (lux > LUX_DARK_OFF) night = false;  // стало світло
    return night;                                 // між порогами — без змін
}

/* ==================== Лампи ==================== */
static void set_lights(bool r, bool y, bool g)
{
    gpio_set_level(LED_RED_PIN,    r);
    gpio_set_level(LED_YELLOW_PIN, y);
    gpio_set_level(LED_GREEN_PIN,  g);

    /* пішоходам зелений лише коли машинам ЧЕРВОНИЙ і НЕ горить жовтий */
    bool ped_go = r && !y;
    gpio_set_level(LED_PED_GRN_PIN, ped_go);
    gpio_set_level(LED_PED_RED_PIN, !ped_go);
}

/* resolve: якими мають бути r/y/g для стану + фази мигання (вихідні параметри) */
static void resolve_lights(traffic_state_t st, bool blink_on,
                           bool *r, bool *y, bool *g)
{
    const state_cfg_t *c = &TABLE[st];
    bool show = c->blink ? blink_on : true;
    *r = c->r && show;
    *y = c->y && show;
    *g = c->g && show;
}

/* неблокуючий антидребезг: true = валідний натиск */
static bool button_pressed(void)
{
    static int64_t last_us = 0;
    if (!btn_irq_pending) return false;
    btn_irq_pending = false;

    int64_t now = esp_timer_get_time();
    if (now - last_us < DEBOUNCE_MS * 1000) return false;
    last_us = now;
    return true;
}



static void setup_all(void)
{
    setup_one_led(LED_RED_PIN);
    setup_one_led(LED_YELLOW_PIN);
    setup_one_led(LED_GREEN_PIN);
    setup_one_led(LED_PED_RED_PIN);
    setup_one_led(LED_PED_GRN_PIN);
    setup_button_irq();
    setup_ldr_adc();
    setup_buzzer();
    ESP_LOGI(TAG, "Setup complete");
} 

/* ==================== main ==================== */
void app_main(void)
{
    setup_all();

    traffic_state_t st = ST_RED;                 // старт з безпечного стану
    int64_t state_start = esp_timer_get_time();
    int64_t last_lux_us = 0;
    bool  ped_request   = false;                 // пішохід натиснув кнопку
    float lux           = 1000.0f;               // старт як "день"

    while (1) {
        int64_t  now    = esp_timer_get_time();
        uint32_t now_ms = (uint32_t)(now / 1000);

        /* 1) LDR — читаємо раз на LUX_INTERVAL, не щоцикл */
        if (now - last_lux_us >= LUX_INTERVAL * 1000) {
            last_lux_us = now;
            lux = read_lux();
#if DEBUG
            ESP_LOGI(TAG, "lux=%.1f", lux);
#endif
        }

        /* 2) кнопка = запит пішохода "хочу перейти" (ISR лише поставив прапорець) */
        if (button_pressed()) {
            ped_request = true;
            ESP_LOGI(TAG, "pedestrian request");
        }

        bool night = is_night(lux);   // день/ніч вирішує ТІЛЬКИ LDR
        bool blink_on = ((now_ms / BLINK_HALF_MS) % 2) == 0;

        /* 3) вирахувати лампи */
        bool r, y, g;
        if (night) { r = false; y = blink_on; g = false; }  // жовтий миготливий
        else       resolve_lights(st, blink_on, &r, &y, &g);

        set_lights(r, y, g);

        /* 4) buzzer для незрячих: пікає, поки пішоходам зелений */
        bool ped_green = r && !y;                            // = машинам червоний
        bool beep = ped_green && (((now_ms / BEEP_HALF_MS) % 2) == 0);
        buzzer_tone(beep);

        /* 5) рух по циклу — лише вдень */
        if (!night) {
            if (st == ST_RED) {
                ped_request = false;         // вже червоне: пішоходи йдуть -> запит виконано
            } else if (ped_request && st == ST_GREEN) {
                st = ST_GREEN_BLINK;         // їде зелене -> скорочуємо його (безпечно, через жовте)
                state_start = now;
            }

            uint32_t elapsed = (uint32_t)((now - state_start) / 1000);
            if (elapsed >= TABLE[st].dur_ms) {
                st = (traffic_state_t)((st + 1) % ST_COUNT);
                state_start = now;
            }
        } else {
            st = ST_RED;              // повернеться день -> почнемо з безпеки
            state_start = now;
        }

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}