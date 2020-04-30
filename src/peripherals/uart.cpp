#include "uart.h"
#include "gpio.h"
#include <samd10.h>

uart* ser0_hdl;
uart* ser1_hdl;

uart::uart(Sercom* sercom, gpio_t tx, gpio_t rx, uint8_t txpad, uint8_t rxpad, uint16_t speed) {
    m_sercom = sercom;

    if(sercom == SERCOM1) {
        ser1_hdl = this;
        PM->APBCMASK.reg |= PM_APBCMASK_SERCOM1;
        GCLK->CLKCTRL.reg = GCLK_CLKCTRL_ID(SERCOM1_GCLK_ID_CORE) |
            GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN(0);
    } else {
        ser0_hdl = this;
        PM->APBCMASK.reg |= PM_APBCMASK_SERCOM0;
        GCLK->CLKCTRL.reg = GCLK_CLKCTRL_ID(SERCOM0_GCLK_ID_CORE) |
            GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN(0);
    }

    gpio::pmuxen(tx, GPIO_PMUX_C);
    gpio::pmuxen(rx, GPIO_PMUX_C);

    sercom->USART.CTRLA.bit.ENABLE = 0;
    sercom->USART.CTRLA.bit.SWRST = 1;
    while(sercom->USART.CTRLA.bit.SWRST);

    sercom->USART.CTRLA.bit.MODE = SERCOM_USART_CTRLA_MODE_USART_INT_CLK_Val;
    sercom->USART.CTRLA.bit.DORD = 1;
    sercom->USART.CTRLA.bit.TXPO = txpad;
    sercom->USART.CTRLA.bit.RXPO = rxpad;
    sercom->USART.BAUD.reg = speed;
    sercom->USART.CTRLB.reg = SERCOM_USART_CTRLB_TXEN | SERCOM_USART_CTRLB_RXEN;

    rxhead = 1;
    sercom->USART.INTENSET.bit.RXC = 1;
    if(sercom == SERCOM0)
        NVIC_EnableIRQ(SERCOM0_IRQn);
    else if(sercom == SERCOM1)
        NVIC_EnableIRQ(SERCOM1_IRQn);

    sercom->USART.CTRLA.bit.ENABLE = 1;
}

void uart::sercom_handler() {
    if(!rxfull) {
		rxbuf[rxhead] = m_sercom->USART.DATA.reg;
		rxhead = (rxhead+1) % sizeof(rxbuf);
		rxfull = rxtail == rxhead;
	} else {
		(void)m_sercom->USART.DATA.reg; // Clear RX flag
	}
}

void uart::write(char c) {
    m_sercom->USART.DATA.reg = c;
	while(!m_sercom->USART.INTFLAG.bit.DRE);
}

void uart::print(const char* str) {
    while(*str) write(*str++);
}

void uart::print(std::string str) {
    print(str.c_str());
}

char uart::getc() {
	while(rxempty());
	
	uint8_t c = rxbuf[rxtail];
	
	rxfull = false;
	rxtail = (rxtail+1) % sizeof(rxbuf);
	
	return c;
}

bool uart::rxempty() {
	return (!rxfull && (rxhead == rxtail));
}

void uart::gets(char *s, int n) {
	char c = 1;
	while((c != 0) && (c != '\r') && (c != '\n') && n--) {
		c = (char)getc();
		*s++ = c;
	}
	*s = 0;
}

void SERCOM0_Handler() {
    if(ser0_hdl) ser0_hdl->sercom_handler();
}

void SERCOM1_Handler() {
    if(ser1_hdl) ser1_hdl->sercom_handler();
}