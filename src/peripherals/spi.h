#pragma once

#include <samd10.h>
#include "gpio.h"

#define SPI_BAUD(x) (F_CPU / (2 * (x + 1)))

typedef struct {
    gpio_t miso;
    gpio_t mosi;
    gpio_t sck;
    bool miso_alt;
    bool mosi_alt;
    bool sck_alt;
    uint8_t misopad;
    uint8_t mosisckpad;
} spi_pincfg_t;

class spi_t {
private:
    Sercom* m_sercom;

public:
    spi_t(Sercom* sercom);

    void Init(spi_pincfg_t pincfg, uint16_t speed);
    uint8_t Transfer(uint8_t b);
};

extern spi_t g_spi0;
extern spi_t g_spi1;