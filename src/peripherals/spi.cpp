#include "spi.h"

void spi::init(Sercom* sercom, spi_pincfg_t pincfg, uint16_t speed) {
    m_sercom = sercom;

    if(sercom == SERCOM1) {
        /*PM->APBCMASK.reg |= PM_APBCMASK_SERCOM1;
        GCLK->CLKCTRL.reg = GCLK_CLKCTRL_ID(SERCOM1_GCLK_ID_CORE) |
            GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN(0);*/
        PM->APBCMASK.bit.SERCOM1_ = 1;
        GCLK->CLKCTRL.reg = GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_ID_SERCOM1_CORE;
        while(GCLK->STATUS.bit.SYNCBUSY);
    } else {
        PM->APBCMASK.bit.SERCOM0_ = 1;
        GCLK->CLKCTRL.reg = GCLK_CLKCTRL_ID(SERCOM0_GCLK_ID_CORE) |
            GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN(0);
        while(GCLK->STATUS.bit.SYNCBUSY);
    }
    gpio::pmuxen(pincfg.miso, pincfg.miso_alt ? GPIO_PMUX_D : GPIO_PMUX_C);
    gpio::pmuxen(pincfg.mosi, pincfg.mosi_alt ? GPIO_PMUX_D : GPIO_PMUX_C);
    gpio::pmuxen(pincfg.sck, pincfg.sck_alt ? GPIO_PMUX_D : GPIO_PMUX_C);

    sercom->SPI.CTRLA.bit.ENABLE = 0;
	sercom->SPI.CTRLA.bit.SWRST = 1;
	while(sercom->SPI.CTRLA.bit.SWRST);

	sercom->SPI.CTRLA.bit.MODE = SERCOM_SPI_CTRLA_MODE_SPI_MASTER_Val;
	sercom->SPI.CTRLA.bit.DOPO = pincfg.mosisckpad;
	sercom->SPI.CTRLA.bit.DIPO = pincfg.misopad;

    /* synchronization busy */
    while(sercom->SPI.SYNCBUSY.bit.CTRLB);
    /* SPI receiver is enabled */
    sercom->SPI.CTRLB.bit.RXEN = 1;
    /* synchronization busy */
    while(sercom->SPI.SYNCBUSY.bit.CTRLB);

    /* baud register value corresponds to the SPI speed */
    sercom->SPI.BAUD.reg = speed;
    /* SERCOM peripheral enabled */
    sercom->SPI.CTRLA.bit.ENABLE = 1;
    /* synchronization busy */
    while(sercom->SPI.SYNCBUSY.bit.ENABLE);
}

spi::spi(Sercom* sercom, spi_pincfg_t pincfg, uint16_t speed) {
    init(sercom, pincfg, speed);
}

uint8_t spi::transfer(uint8_t b) {
    if(!m_sercom) return 0xFF;

    while(!m_sercom->SPI.INTFLAG.bit.DRE); // Wait for data register empty
    //m_sercom->SPI.INTFLAG.bit.RXC = 1; // Clear RXC flag by setting it to 1
    m_sercom->SPI.DATA.reg = b;
	while(!m_sercom->SPI.INTFLAG.bit.RXC);
	return m_sercom->SPI.DATA.reg;
}