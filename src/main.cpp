#include <samd10.h>
#include <gpio.h>
#include <uart.h>
#include <GSM.h>
#define STR_IMPLEMENTATION
#include <sensornet.h>
#include <sensors.h>
#include <Str.h>
#include <RF24.h>
#include <spi.h>
extern "C" {
#include <systick.h>
}

static const gpio_t led{ GPIO_PORTA, 4 };

static GSM gsm(
    // PWRKEY on PA2
    {GPIO_PORTA, 2},
    &g_uart1
);

// PA14 - CE
// PA15 - CS
// SPI pinout in main
static Sensornet net(RF24({ GPIO_PORTA, 14 }, { GPIO_PORTA, 15 }, g_spi0));

static void cmd_info(GSM* gsm, const char* sender, const char* args) {
    Str str;

    for(auto& node : net.m_nodes) {
        str.appendf("Node:\n ID: %d\n Name: %s\n Type: %d\n Last seen: %d sec ago\n",
            node.data.id, node.data.name, node.data.type, (millis() - node.last_seen) / 1000);

        switch(node.data.type) {
            case sensors::TYPE_SOIL:
                str.appendf(" Soil moisture sensor:\n  Moisture: %d\n", ((sensors::Soil*)node.data.data)->moisture);
                break;
            case sensors::TYPE_TEMP_HUM:
                str.appendf(" Temperature + humidity sensor:\n  Temperature: %d\n  Humidity: %d\n",
                    ((sensors::TempHum*)node.data.data)->temperature, ((sensors::TempHum*)node.data.data)->humidity);
                break;
            case sensors::TYPE_TEMP:
                str.appendf(" Temperature sensor:\n  Temperature: %d\n",
                    ((sensors::Temp*)node.data.data)->temperature);
                break;
            default:
                break;
        }
    }

    gsm->SendSMS(sender, str.c_str());
}

int main() {
    // TX on PA24 and RX on PA25
    g_uart1.init({{GPIO_PORTA, 24}, {GPIO_PORTA, 25}, UART_SER1_TXPO_PA24, UART_SER1_RXPO_PA25}, CALC_BAUD(115200));

    g_spi0.Init({
        { GPIO_PORTA, 5 }, // MISO on PA5
        { GPIO_PORTA, 8 }, // MOSI on PA8
        { GPIO_PORTA, 9 }, // SCK on PA9
        true, // PA5 alternative pad
        true, // PA8 alternative pad
        true, // PA9 alternative pad
        1, // MISO on PA5
        1 // MOSI on PA8 and SCK on PA9
    }, 20);

    gpio::mode(led, GPIO_DIR_OUT);

    if(!net.begin()) {
        while(1) {
            gpio::toggle(led);
            delay_usec(800000);
        }
    }
    
    if(!gsm.Init()) {
        while(1) {
            net.Poll();
            gpio::toggle(led);
            delay_usec(200000);
        }
    }

    gsm.AddSMSFunc({ "info", cmd_info, 0 });

    while(1) {
        gsm.Poll();
        net.Poll();
        
        gpio::toggle(led);
        //auto start = millis();
        //while(millis() - start < 500);
    }
}