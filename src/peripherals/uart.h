#pragma once

#include <samd10.h>
#include "gpio.h"
#include <string>

#define CALC_BAUD(x) (65536 * (1 - (8 * (x / F_CPU))))

class uart {
private:
    Sercom* m_sercom;

    uint8_t rxbuf[512];
    uint16_t rxhead;
    uint16_t rxtail;
    bool rxfull;

public:
    uart(Sercom* sercom, gpio_t tx, gpio_t rx, uint8_t txpad, uint8_t rxpad, uint16_t speed);
    void write(char c);
    void print(const char* str);
    void print(std::string str);
    char getc();
    void gets(char *s, int n);
    bool rxempty();
    
    void sercom_handler();
};