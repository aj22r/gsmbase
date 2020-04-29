#pragma once

#include <stdint.h>

__attribute__ ( ( section ( ".ramfunc" ) ) ) __attribute__ ((noinline)) void delay_usec(uint32_t n);
void systick_init();
uint64_t systick_cycles();
uint32_t micros();
uint32_t millis();