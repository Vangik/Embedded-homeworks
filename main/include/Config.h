#ifndef CONFIG_H
#define CONFIG_H

// ================= Logging =================
#define DEBUG 1                 // 1 = друкувати lux/стан у лог

// ================= Піни: LED світлофора =================
#define LED_RED_PIN     15      // машини: червоний
#define LED_YELLOW_PIN  2       // машини: жовтий
#define LED_GREEN_PIN   5       // машини: зелений
#define LED_PED_RED_PIN 18      // пішоходи: червоний
#define LED_PED_GRN_PIN 19      // пішоходи: зелений

// ================= Піни: входи / виходи =================
#define BTN_PIN     21          // кнопка: ручний нічний режим
#define LDR_PIN     4           // фоторезистор -> ADC
#define BUZZER_PIN  23          // пасивний buzzer -> PWM (LEDC)

// ================= LDR: дільник і калібрування (GL5528) =================
#define ADC_RESOLUTION_BITS 12
#define ADC_MAX ((float)((1 << ADC_RESOLUTION_BITS) - 1)) // 2^12 - 1

#define VCC   3.3f
#define R1    10000.0f          // резистор дільника, 10k
#define GAMMA 0.7f              // нахил кривої GL5528
#define RL10  50000.0f          // опір LDR при 10 lux (калібрування)

#define LDR_ON_TOP 1

// Поріг "темно" з гістерезисом (щоб не блимало на межі):
#define LUX_DARK_ON   30.0f     // lux нижче цього -> НІЧ
#define LUX_DARK_OFF  60.0f     // lux вище цього  -> ДЕНЬ

// ================= Buzzer (пасивний, тон через GPTimer) =================
#define BUZZER_FREQ_HZ 2700     // висота тону (пік), Гц
#define BEEP_HALF_MS   150      // ритм піків під час зеленого пішоходам

// ================= Час =================
#define LUX_INTERVAL  100       // період опитування LDR, мс
#define BLINK_HALF_MS 250       // півперіод мигання LED (~2 Гц)
#define DEBOUNCE_MS   40        // антидребезг кнопки, мс

// ================= Тривалості станів циклу, мс =================
#define T_GREEN       5000
#define T_GREEN_BLINK 3000
#define T_YELLOW      2000
#define T_RED         5000
#define T_RED_YELLOW  2000

#endif // CONFIG_H