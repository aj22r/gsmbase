#include <samd10.h>
#include <gpio.h>
#include <uart.h>
#include <GSM.h>
extern "C" {
#include <systick.h>
}

static const gpio_t led{ GPIO_PORTA, 4 };

static GSM gsm(
    // PWRKEY on PA2
    {GPIO_PORTA, 2},
    // TX on PA24 and RX on PA25
    &g_uart1
);

int main() {
    g_uart1.init({{GPIO_PORTA, 24}, {GPIO_PORTA, 25}, UART_SER1_TXPO_PA24, UART_SER1_RXPO_PA25}, CALC_BAUD(115200));

    gpio::mode(led, GPIO_DIR_OUT);
    
    if(!gsm.Init()) {
        while(1) {
            gpio::toggle(led);
            delay_usec(200000);
        }
    }

    while(1) {
        gsm.Poll();
        
        gpio::toggle(led);
        //auto start = millis();
        //while(millis() - start < 500);
    }
}