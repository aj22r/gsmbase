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

struct gpio_t {
    const int port;
    const int pin;

    inline bool operator==(gpio_t b) {
        return port == b.port && pin == b.pin;
    }
    inline bool operator!=(gpio_t b) {
        return !operator==(b);
    }
};

enum gpio_dir {
    GPIO_DIR_IN,
    GPIO_DIR_OUT
};

namespace gpio {
    inline void mode(const int port, const int pin, const gpio_dir dir) {
        if(dir == GPIO_DIR_IN) {
            PORT->Group[port].DIRCLR.reg = 1 << pin;
            PORT->Group[port].PINCFG[pin].bit.INEN = 1;
        } else {
            PORT->Group[port].DIRSET.reg = 1 << pin;
            PORT->Group[port].PINCFG[pin].bit.INEN = 0;
        }
    }
    inline void mode(const gpio_t& gp, const gpio_dir dir) { mode(gp.port, gp.pin, dir); }

    inline void pullup(const int port, const int pin, const bool en) {
        PORT->Group[port].PINCFG[pin].bit.PULLEN = en;
    }
    inline void pullup(const gpio_t& gp, const bool en) { pullup(gp.port, gp.pin, en); }

    inline void drvstr(const int port, const int pin, const bool en) {
        PORT->Group[port].PINCFG[pin].bit.DRVSTR = en;
    }
    inline void drvstr(const gpio_t& gp, const bool en) { drvstr(gp.port, gp.pin, en); }

    inline void set(const int port, const int pin, const bool en) {
        if(en)
            PORT->Group[port].OUTSET.reg = 1 << pin;
        else
            PORT->Group[port].OUTCLR.reg = 1 << pin;
    }
    inline void set(const gpio_t& gp, const bool en) { set(gp.port, gp.pin, en); }

    inline void toggle(const int port, const int pin) {
        PORT->Group[port].OUTTGL.reg = 1 << pin;
    }
    inline void toggle(const gpio_t& gp) { toggle(gp.port, gp.pin); }

    inline bool read(const int port, const int pin) {
        return (PORT->Group[port].IN.reg & (1 << pin)) != 0;
    }
    inline bool read(const gpio_t& gp) { return read(gp.port, gp.pin); }

    inline void pmuxen(const int port, const int pin, const uint8_t mux) {
        if(pin & 1)
            PORT->Group[port].PMUX[pin / 2].bit.PMUXO = mux;
        else
            PORT->Group[port].PMUX[pin / 2].bit.PMUXE = mux;
            
        PORT->Group[port].PINCFG[pin].bit.PMUXEN = 1;
    }
    inline void pmuxen(const gpio_t& gp, const uint8_t mux) { pmuxen(gp.port, gp.pin, mux); }

    inline void pmuxdis(const int port, const int pin) {
        PORT->Group[port].PINCFG[pin].bit.PMUXEN = 0;
    }
    inline void pmuxdis(const gpio_t& gp) { pmuxdis(gp.port, gp.pin); }
}