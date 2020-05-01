#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <gpio.h>
#include <uart.h>

class GSM {
private:
    gpio_t m_pwrkey;
    uart_t m_uart;

public:
    GSM(const gpio_t pwrkey, const uart_t uart);
    ~GSM();

    bool PowerOn();
    bool Command(const char* cmd, const char* result = NULL);
    // Repeat a command until we get the desired result
    bool RepeatCommand(const char* cmd, const char* result, int repeats);
    bool Init();
};