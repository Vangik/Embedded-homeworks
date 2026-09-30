#pragma once

#include <stdint.h>

void pwm_hw_init(int gpio);
void pwm_hw_set_pulse_us(uint32_t pulse_us);