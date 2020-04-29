#include "gpio.h"
#include <samd10.h>

gpio::gpio(const int _port, const int _pin, const bool is_input) {
    m_port = _port;
    m_pin = _pin;

    if(is_input) {
        PORT->Group[m_port].DIRCLR.reg = 1 << m_pin;
    } else {
        PORT->Group[m_port].DIRSET.reg = 1 << m_pin;
    }

    PORT->Group[m_port].PINCFG[m_pin].bit.INEN = is_input;
}

void gpio::set_drvstr(bool en) {
    PORT->Group[m_port].PINCFG[m_pin].bit.DRVSTR = en;
}

void gpio::set_pullup(bool en) {
    PORT->Group[m_port].PINCFG[m_pin].bit.PULLEN = en;
}

void gpio::set(bool en) {
    if(en)
        PORT->Group[m_port].OUTSET.reg = 1 << m_pin;
    else
        PORT->Group[m_port].OUTCLR.reg = 1 << m_pin;
}

bool gpio::read() {
    return (PORT->Group[m_port].IN.reg & (1 << m_pin)) != 0;
}

void gpio::toggle() {
    PORT->Group[m_port].OUTTGL.reg = 1 << m_pin;
}