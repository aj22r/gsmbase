#pragma once

#include <stdint.h>
#include <samd10.h>

#define GPIO_PORTA       0
#define GPIO_PORTB       1
#define GPIO_PORTC       2

#define GPIO_PMUX_A      0
#define GPIO_PMUX_B      1
#define GPIO_PMUX_C      2
#define GPIO_PMUX_D      3
#define GPIO_PMUX_E      4
#define GPIO_PMUX_F      5
#define GPIO_PMUX_G      6
#define GPIO_PMUX_H      7
#define GPIO_PMUX_I      8

class gpio {
private:
    int m_port;
    int m_pin;
public:
    gpio(const int _port, const int _pin, const bool is_input = false);
    void set_pullup(bool en);
    void set_drvstr(bool en);
    void set(bool en);
    void set() { this->set(true); }
    void clear() { this->set(false); }
    bool read();
    void toggle();
};