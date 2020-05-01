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
    uart_t(SERCOM0, {{GPIO_PORTA, 24}, {GPIO_PORTA, 25}, UART_SER1_TXPO_PA24, UART_SER1_RXPO_PA25}, CALC_BAUD(9600))
);

volatile uint32_t test;

int main() {
    systick_init();
    //gpio::set(led, true);

    while(1) {
        gpio::toggle(led);
        uint32_t last_millis = millis();
        while(millis() - last_millis < 250);
        //delay_usec(250000);
    }
}