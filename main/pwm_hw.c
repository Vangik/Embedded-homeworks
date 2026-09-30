#include "pwm_hw.h"
#include "driver/gpio.h"
#include "esp_rom_gpio.h"
#include "soc/gpio_sig_map.h"
#include "soc/ledc_struct.h"
#include "soc/dport_reg.h"
#include "soc/dport_access.h"


#define PWM_RES_BITS 13
#define PWM_PERIOD_TICKS (1u << PWM_RES_BITS)
#define PWM_HZ 50
#define APB_HZ 80000000u
#define PERIOD_US 20000u

#define CLK_DIV ((uint32_t)(((uint64_t)APB_HZ << 8) / (PWM_HZ * PWM_PERIOD_TICKS)))


static void pwm_hw_enable_clock(void)
{
    /* --- такт блоку LEDC (міст DPORT на ESP32) --- */
    DPORT_SET_PERI_REG_MASK(DPORT_PERIP_CLK_EN_REG, DPORT_LEDC_CLK_EN);
    DPORT_SET_PERI_REG_MASK(DPORT_PERIP_RST_EN_REG, DPORT_LEDC_RST);
    DPORT_CLEAR_PERI_REG_MASK(DPORT_PERIP_RST_EN_REG, DPORT_LEDC_RST);
    LEDC.conf.apb_clk_sel = 1; /* APB 80 МГц; на ESP32 тактування модуля вмикає лише DPORT */
}

void pwm_hw_init(int gpio)
{
    pwm_hw_enable_clock();
    /* --- таймер: період кадру 20 мс, далі не змінюється (порядок як в IDF) --- */
    LEDC.timer_group[1].timer[0].conf.clock_divider = CLK_DIV;
    LEDC.timer_group[1].timer[0].conf.tick_sel = 1; /* джерело такту = SLOW_CLK (APB) */
    LEDC.timer_group[1].timer[0].conf.duty_resolution = PWM_RES_BITS;
    LEDC.timer_group[1].timer[0].conf.low_speed_update = 1; /* зафіксувати дільник+розрядність */
    LEDC.timer_group[1].timer[0].conf.pause = 0;            /* resume */
    LEDC.timer_group[1].timer[0].conf.rst = 1;              /* імпульс скидання лічильника */
    LEDC.timer_group[1].timer[0].conf.rst = 0;
    /* --- канал: HIGH від відліку 0, період бере з таймера 0 --- */
    LEDC.channel_group[1].channel[0].hpoint.hpoint = 0;
    LEDC.channel_group[1].channel[0].conf0.timer_sel = 0;
    LEDC.channel_group[1].channel[0].conf0.sig_out_en = 1;
    LEDC.channel_group[1].channel[0].conf0.idle_lv = 0;
    LEDC.channel_group[1].channel[0].conf0.low_speed_update = 1;
    /* --- матриця: вихід каналу 0 на вибраний GPIO (лінія уставки) --- */
    gpio_reset_pin(gpio);
    gpio_set_direction(gpio, GPIO_MODE_OUTPUT);
    esp_rom_gpio_connect_out_signal((uint32_t)gpio, LEDC_LS_SIG_OUT0_IDX, false, false);
}

void pwm_hw_set_pulse_us(uint32_t pulse_us)
{
    /* --- ширина HIGH = уставка кута, мкс; частота таймера не чіпається --- */
    if (pulse_us > PERIOD_US)
    {
        pulse_us = PERIOD_US;
    }
    /* 1500 мкс / 20000 мкс · 8192 ≈ 614 відліків; у полі duty — зі зсувом 4 */
    const uint32_t duty = (pulse_us * PWM_PERIOD_TICKS) / PERIOD_US;
    LEDC.channel_group[1].channel[0].duty.duty = duty << 4;
    /* статичний рівень = 1 крок масштабом 0: без цих полів (num=0) duty не застосується */
    LEDC.channel_group[1].channel[0].conf1.duty_inc = 1;
    LEDC.channel_group[1].channel[0].conf1.duty_num = 1;
    LEDC.channel_group[1].channel[0].conf1.duty_cycle = 1;
    LEDC.channel_group[1].channel[0].conf1.duty_scale = 0;
    LEDC.channel_group[1].channel[0].conf0.sig_out_en = 1;
    /* duty_start самоскидний — дочекатись завершення попередньої зміни */
    while (LEDC.channel_group[1].channel[0].conf1.duty_start)
    {
    }
    LEDC.channel_group[1].channel[0].conf1.duty_start = 1;
    LEDC.channel_group[1].channel[0].conf0.low_speed_update = 1;
}