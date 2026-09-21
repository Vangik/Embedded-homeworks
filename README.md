# Interrupt in ESP32 — Button Debounce

Домашнє завдання: реалізувати та порівняти різні методи усунення брязкоту
механічної кнопки на ESP32 (ESP-IDF, Framework: ESP-IDF).

**Платформа:** ESP32 (DevKit)
**Фреймворк:** ESP-IDF v6.1-beta1
**Кнопка:** GPIO21 (активна в 0, pull-up)
**Світлодіод:** GPIO19

---

## Зміст

1. [Схема підключення](#схема-підключення)
2. [Завдання 1 — Без debounce](#завдання-1--без-debounce-базова-реалізація)
3. [Завдання 2 — Time-based debounce](#завдання-2--time-based-debounce)
4. [Завдання 3 — State-based debounce](#завдання-3--state-based-debounce)
5. [Завдання 4 — Polling + FSM](#завдання-4--polling--debounce-машина-станів)
6. [Завдання 5 — Hardware RC-фільтр](#завдання-5--hardware-debounce-rc-фільтр)
7. [Завдання 6 — Порівняльна таблиця](#завдання-6--порівняльна-таблиця)

---

## Схема підключення

### Базове підключення (Завдання 1–4, без RC)

```
3.3V ──┐
       │
     [pull-up]  (внутрішній GPIO_PULLUP_ENABLE)
       │
GPIO21 ●──────[ КНОПКА ]──────── GND
       │
    (вхід, читаємо рівень: 1 = відпущена, 0 = натиснута)<img width="4080" height="3072" alt="PXL_20260903_131702721" src="https://github.com/user-attachments/assets/b432da2a-ee30-43b5-a271-f2d938df41eb" />

```

- Кнопка не натиснута → pull-up тримає GPIO21 у стані **HIGH (1)**
- Кнопка натиснута → пін замикається на GND → **LOW (0)**
- Переривання / подія — на **спадному фронті** (NEGEDGE)

### Підключення з RC-фільтром (Завдання 5)

```
3.3V ──[ 10k pull-up ]──┬─────[ 100R ]─────● GPIO21
                        │                    │
                     [КНОПКА]            [ 100n ]
                        │                    │
                       GND                  GND
```

### Фото зібраної схеми

<!-- Додай сюди фото макетки/плати -->
Схема без RC
<img width="4080" height="3072" alt="PXL_20260903_131702721" src="https://github.com/user-attachments/assets/3fbee504-253d-4598-b827-1f72fa1ccde4" />

Схема з RC-фільтром
<img width="4080" height="3072" alt="PXL_20260903_145556728" src="https://github.com/user-attachments/assets/d7710332-4b06-48c0-b086-45693dfc078f" />

## Завдання 1 — Без debounce (базова реалізація)

**Метод:** GPIO interrupt на FALLING, інкремент лічильника у кожному
перериванні, вивід у лог. Без жодного антибрязкоту.

**Очікування:** 1 фізичне натискання → кілька interrupt; interrupt
викликається і при відпусканні.

**Результат (лог):**

<img width="956" height="557" alt="task1" src="https://github.com/user-attachments/assets/99bb123e-0c97-4afd-b6c4-531bbaf38885" />

---

## Завдання 2 — Time-based debounce

**Метод:** ISR лише ставить прапорець. Поза ISR ігноруємо подію, якщо з
моменту попередньої прийнятої минуло менше `DEBOUNCE_MS` (порівняння часу
через `xTaskGetTickCount()`).

**Очікування:** менше хибних спрацювань; release ще може реєструватися.

**Результат (лог):**

<img width="925" height="365" alt="Screenshot 2026-09-21 at 21 01 24" src="https://github.com/user-attachments/assets/39cc6ba7-f5cc-43c1-b4f2-1013820f7beb" />

---

## Завдання 3 — State-based debounce

**Метод:** ISR сигналізує «подія». У задачі приймаємо подію лише якщо кнопка
**досі натиснута** (перевірка рівня після короткої затримки), потім чекаємо
відпускання перед наступним прийомом.

**Очікування:** рівно 1 реакція на 1 натискання; release не викликає дії.

**Результат (лог):**

<img width="835" height="306" alt="Screenshot 2026-09-21 at 21 05 20" src="https://github.com/user-attachments/assets/edb81a82-0c01-41ec-b528-51238f62e351" />

---

## Завдання 4 — Polling + debounce (машина станів)

**Метод:** переривання прибрано повністю. Опитуємо пін кожні `POLL_MS`.
Debounce реалізовано як машину станів (FSM) з трьома станами:

```
RELEASED ──(pressed)──► DEBOUNCING ──(стабільно 0 протягом DEBOUNCE_MS)──► PRESSED
    ▲                        │                                                │
    │                   (відпустили                                      (відпустили)
    └───────────────────  = брязкіт) ◄──────────────────────────────────────┘
```

- Реакція відбувається **рівно один раз** — у момент переходу
  `DEBOUNCING → PRESSED`.
- Утримання кнопки **не** дає повторних спрацювань (сидимо в PRESSED).
- Брязкіт не проходить, бо вимагається стабільний рівень 0 протягом усього
  `DEBOUNCE_MS`.

**Очікування:** найстабільніша поведінка; трохи більша затримка реакції.

**Результат (лог):**
<img width="1095" height="441" alt="task4" src="https://github.com/user-attachments/assets/42d9274d-6c42-4ec3-860b-68d9322a9a6a" />
---

## Завдання 5 — Hardware debounce (RC-фільтр)

**Метод:** додано RC-фільтр (100n + 100R, pull-up 10k) і повторено варіанти
1–4 з тим самим кодом, але з апаратним згладжуванням фронту.

**Результати (логи):**

Без debounce + RC
<img width="1094" height="356" alt="task5_1" src="https://github.com/user-attachments/assets/141adbe1-4061-4bc3-849d-5d784f07defa" />
Time-based + RC
<img width="1091" height="354" alt="task5_2" src="https://github.com/user-attachments/assets/43b5fc36-1f12-4bdb-b1b8-80aeafb84205" />
State-based + RC
<img width="1091" height="357" alt="task5_3" src="https://github.com/user-attachments/assets/09c7691a-27ab-476d-96da-32d53eb7e114" />
Polling FSM + RC
<img width="1088" height="357" alt="task5_4" src="https://github.com/user-attachments/assets/7ddb9dc8-f1e6-498b-bb81-843bf8fdf742" />
---

## Завдання 6 — Порівняльна таблиця

| Метод | Кількість хибних спрацювань | Затримка | Складність |
|---|---|---|---|
| Без debounce | багато (5–20 на клік + пачка при відпусканні) | ~0 | мінімальна |
| Time-based (50 мс) | рідко, лишається спрацювання на відпусканні | ~10 мс (реакція на перший фронт і обробка прапорця) + 40–50 мс DEBOUNCE_MS до спрацювання постійного фронту з кнопки| низька |
| State-based | ~0 (release не дає реакції) | ≤ 10 мс (період опитування) | низька |
| Polling FSM | ~0 (release не дає реакції) | 40–50 мс (DEBOUNCE_MS + POLL_MS) | середня |
| Hardware RC | 0 | +τ ≈ 10 мкс | код простий, потрібне залізо |
