#include "uart.h"
#include "gpio.h"
#include <samd10.h>
#include <stdio.h>
#include <sys/unistd.h>

uart_t g_uart0(SERCOM0);
uart_t g_uart1(SERCOM1);

/*int _write(int file, char* data, int len) {
    if(!file || file == STDOUT_FILENO || file == STDERR_FILENO) return 0;

    for(int i = 0; i < len; i++)
        ((uart_t*)file)->write(*data++);
    
    return len;
}*/

void uart_t::init(const uart_pincfg_t& pincfg, uint16_t speed) {

    if(m_sercom == SERCOM1) {
        PM->APBCMASK.reg |= PM_APBCMASK_SERCOM1;
        GCLK->CLKCTRL.reg = GCLK_CLKCTRL_ID(SERCOM1_GCLK_ID_CORE) |
            GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN(0);
    } else {
        PM->APBCMASK.reg |= PM_APBCMASK_SERCOM0;
        GCLK->CLKCTRL.reg = GCLK_CLKCTRL_ID(SERCOM0_GCLK_ID_CORE) |
            GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN(0);
    }

    gpio::pmuxen(pincfg.tx, pincfg.tx_alt ? GPIO_PMUX_D : GPIO_PMUX_C);
    gpio::pmuxen(pincfg.rx, pincfg.rx_alt ? GPIO_PMUX_D : GPIO_PMUX_C);

    m_sercom->USART.CTRLA.bit.ENABLE = 0;
    m_sercom->USART.CTRLA.bit.SWRST = 1;
    while(m_sercom->USART.CTRLA.bit.SWRST);

    m_sercom->USART.CTRLA.bit.MODE = SERCOM_USART_CTRLA_MODE_USART_INT_CLK_Val;
    m_sercom->USART.CTRLA.bit.DORD = 1;
    m_sercom->USART.CTRLA.bit.TXPO = pincfg.tx_pad;
    m_sercom->USART.CTRLA.bit.RXPO = pincfg.rx_pad;
    m_sercom->USART.BAUD.reg = speed;
    m_sercom->USART.CTRLB.reg = SERCOM_USART_CTRLB_TXEN | SERCOM_USART_CTRLB_RXEN;

    m_sercom->USART.INTENSET.bit.RXC = 1;
    if(m_sercom == SERCOM0)
        NVIC_EnableIRQ(SERCOM0_IRQn);
    else if(m_sercom == SERCOM1)
        NVIC_EnableIRQ(SERCOM1_IRQn);

    m_sercom->USART.CTRLA.bit.ENABLE = 1;

    m_used = true;
}

void uart_t::sercom_handler() {
    if(!m_used) return;

    if(!m_rxfull) {
		m_rxbuf[m_rxhead] = m_sercom->USART.DATA.reg;
		m_rxhead = (m_rxhead+1) % sizeof(m_rxbuf);
		m_rxfull = m_rxtail == m_rxhead;
	} else {
		(void)m_sercom->USART.DATA.reg; // Clear RX flag
	}
}

void uart_t::write(char c) {
    if(!m_used) return;
    
    m_sercom->USART.DATA.reg = c;
	while(!m_sercom->USART.INTFLAG.bit.DRE);
}

void uart_t::print(const char* str) {
    if(!m_used) return;
    
    while(*str) write(*str++);
}

void uart_t::print(const std::string& str) {
    if(!m_used) return;
    
    print(str.c_str());
}

char uart_t::getc() {
    if(!m_used) return '\0';
    
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
    if(!m_used) return;
    
	char c = 1;
	while((c != 0) && (c != '\r') && (c != '\n') && n-- > 1) {
		c = (char)getc();
		*s++ = c;
	}
	*s = 0;
}

char uart_t::peek(int pos) {
    if(!m_used) return '\0';
    
    if(rxempty() || pos >= available()) return '\0';

    if(pos == -1)
        return m_rxbuf[m_rxtail];
    else
        return m_rxbuf[(m_rxtail + pos) % sizeof(m_rxbuf)];
}

int uart_t::find(const char* str) {
    if(!m_used) return -1;
    
    for(int i = 0; i < available(); i++) {
        int search_idx = 0;
        while(str[search_idx] == peek(i + search_idx))
            if(!str[++search_idx]) return i;
    }

    return -1;
}

char* uart_t::read() {
    if(!m_used) return NULL;
    
    size_t avail = available();
    if(!avail) return NULL;

    char* data = (char*)malloc(avail);
    if(!data) return NULL;

    for(size_t i = 0; i < avail; i++)
        data[i] = getc();
    
    return data;
}

void SERCOM0_Handler() {
    g_uart0.sercom_handler();
}

void SERCOM1_Handler() {
    g_uart1.sercom_handler();
}