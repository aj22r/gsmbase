#pragma once

#include <samd10.h>
#include <gpio.h>
#include <string>

#define CALC_BAUD(x) (uint16_t)(65536.0 * (1.0 - (16.0 * ((double)x / (double)F_CPU))))

#define UART_SER1_RXPO_PA30 0
#define UART_SER0_RXPO_ALT_PA04 0
#define UART_SER0_RXPO_PA15 1
#define UART_SER1_RXPO_PA31 1
#define UART_SER0_RXPO_ALT_PA05 1
#define UART_SER0_RXPO_PA04 2
#define UART_SER1_RXPO_PA08 2
#define UART_SER1_RXPO_PA24 2
#define UART_SER0_RXPO_ALT_PA08 2
#define UART_SER1_RXPO_ALT_PA30 2
#define UART_SER0_RXPO_PA05 3
#define UART_SER1_RXPO_PA09 3
#define UART_SER1_RXPO_PA25 3
#define UART_SER0_RXPO_ALT_PA09 3
#define UART_SER1_RXPO_ALT_PA31 3

#define UART_SER1_TXPO_PA30 0
#define UART_SER0_TXPO_ALT_PA04 0
#define UART_SER0_TXPO_PA04 1
#define UART_SER1_TXPO_PA08 1
#define UART_SER1_TXPO_PA24 1
#define UART_SER0_TXPO_ALT_PA08 1
#define UART_SER1_TXPO_ALT_PA30 1

struct uart_pincfg_t {
    gpio_t tx;
    gpio_t rx;
    uint8_t tx_pad;
    uint8_t rx_pad;
    bool tx_alt = false;
    bool rx_alt = false;
};

class uart_t {
private:
    Sercom* m_sercom = NULL;

    volatile uint8_t m_rxbuf[512];
    volatile uint16_t m_rxhead = 0;
    volatile uint16_t m_rxtail = 0;
    volatile bool m_rxfull = false;

    bool m_used = false;

public:
    uart_t(Sercom* sercom) { m_sercom = sercom; }

    void init(const uart_pincfg_t& pincfg, uint16_t speed);
    void write(char c);
    void print(const char* str, int len = -1);
    void print(const std::string& str);
    char getc();
    void gets(char *s, int n);

    // Allocates a buffer and reads all available data into it
    char* read();

    bool rxempty();
    void flush_rx() {
        m_rxhead = 0;
        m_rxtail = 0;
        m_rxfull = false;
    }
    uint16_t available() {
        if(m_rxhead >= m_rxtail)
            return m_rxhead - m_rxtail;
        else
            return sizeof(m_rxbuf) - m_rxtail + m_rxhead;
    }
    char peek(int pos = -1);
    int find(const char* str);
    
    void sercom_handler();
};

extern uart_t g_uart0;
extern uart_t g_uart1;