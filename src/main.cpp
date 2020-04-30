#include <samd10.h>
#include <gpio.h>
extern "C" {
#include <systick.h>
}

const gpio_t pin{ GPIO_PORTA, 4 };

int main() {
    systick_init();
    gpio::set(pin, true);

    while(1) {
        gpio::toggle(pin);
        delay_usec(250000);
    }
}