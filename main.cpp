#include <samd10.h>
#include "gpio.h"
extern "C" {
#include "systick.h"
}

gpio pin(GPIO_PORTA, 4);

int main() {
    systick_init();
    pin.set(true);

    while(1) {
        pin.toggle();
        delay_usec(250000);
    }
}