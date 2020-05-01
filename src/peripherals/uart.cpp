#include "uart.h"
#include "gpio.h"
#include <samd10.h>
#include <stdio.h>
#include <sys/unistd.h>

static uart_t* ser0_hdl;
static uart_t* ser1_hdl;

/*int _write(int file, char* data, int len) {
    if(!file || file == STDOUT_FILENO || file == STDERR_FILENO) return 0;

    for(int i = 0; i < len; i++)
        ((uart_t*)file)->write(*data++);
    
    return len;
}*/

uart_t::uart_t(Sercom* sercom, const uart_pincfg_t& pincfg, uint16_t speed) {
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

    gpio::pmuxen(pincfg.tx, pincfg.tx_alt ? GPIO_PMUX_D : GPIO_PMUX_C);
    gpio::pmuxen(pincfg.rx, pincfg.rx_alt ? GPIO_PMUX_D : GPIO_PMUX_C);

    sercom->USART.CTRLA.bit.ENABLE = 0;
    sercom->USART.CTRLA.bit.SWRST = 1;
    while(sercom->USART.CTRLA.bit.SWRST);

    sercom->USART.CTRLA.bit.MODE = SERCOM_USART_CTRLA_MODE_USART_INT_CLK_Val;
    sercom->USART.CTRLA.bit.DORD = 1;
    sercom->USART.CTRLA.bit.TXPO = pincfg.tx_pad;
    sercom->USART.CTRLA.bit.RXPO = pincfg.rx_pad;
    sercom->USART.BAUD.reg = speed;
    sercom->USART.CTRLB.reg = SERCOM_USART_CTRLB_TXEN | SERCOM_USART_CTRLB_RXEN;

    m_rxhead = 1;
    sercom->USART.INTENSET.bit.RXC = 1;
    if(sercom == SERCOM0)
        NVIC_EnableIRQ(SERCOM0_IRQn);
    else if(sercom == SERCOM1)
        NVIC_EnableIRQ(SERCOM1_IRQn);

    sercom->USART.CTRLA.bit.ENABLE = 1;
}

void uart_t::sercom_handler() {
    if(!m_rxfull) {
		m_rxbuf[m_rxhead] = m_sercom->USART.DATA.reg;
		m_rxhead = (m_rxhead+1) % sizeof(m_rxbuf);
		m_rxfull = m_rxtail == m_rxhead;
	} else {
		(void)m_sercom->USART.DATA.reg; // Clear RX flag
	}
}

void uart_t::write(char c) {
    m_sercom->USART.DATA.reg = c;
	while(!m_sercom->USART.INTFLAG.bit.DRE);
}

void uart_t::print(const char* str) {
    while(*str) write(*str++);
}

void uart_t::print(const std::string& str) {
    print(str.c_str());
}

char uart_t::getc() {
	while(rxempty());
	
	uint8_t c = m_rxbuf[m_rxtail];
	
	m_rxfull = false;
	m_rxtail = (m_rxtail+1) % sizeof(m_rxbuf);
	
	return c;
}

bool uart_t::rxempty() {
	return (!m_rxfull && (m_rxhead == m_rxtail));
}

void uart_t::gets(char *s, int n) {
	char c = 1;
	while((c != 0) && (c != '\r') && (c != '\n') && n-- > 1) {
		c = (char)getc();
		*s++ = c;
	}
	*s = 0;
}

char uart_t::peek(int pos) {
    if(rxempty() || pos > available()) return '\0';

    if(pos == -1)
        return m_rxbuf[m_rxtail];
    else
        return m_rxbuf[(m_rxtail + pos) % sizeof(m_rxbuf)];
}

int uart_t::find(const char* str) {
    for(int i = 0; i < available(); i++) {
        int search_idx = 0;
        while(str[search_idx] == peek(i + search_idx))
            if(!str[search_idx]) return i;
    }

    return -1;
}

void SERCOM0_Handler() {
    if(ser0_hdl) ser0_hdl->sercom_handler();
}

void SERCOM1_Handler() {
    if(ser1_hdl) ser1_hdl->sercom_handler();
}