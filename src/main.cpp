#include <samd10.h>
#include <gpio.h>
#include <uart.h>
#include <GSM.h>
#include <sensornet.h>
#include <sensors.h>
#define STR_IMPLEMENTATION
#include <Str.h>
#include <RF24.h>
#include <spi.h>
extern "C" {
#include <systick.h>
}

void SetupRTC();
void UpdateRTC();

static volatile int rtc_count = 10;

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
    Str str = "List:\n";
    
    for(auto& node : net.m_nodes) {
        str.appendf("Node:\n ID: %d\n Name: %s\n Type: %d\n Last seen: %d sec ago\n",
            node.data.id, node.data.name, node.data.type, (millis() - node.last_seen) / 1000);

        switch(node.data.type) {
            case Sensors::TYPE_SOIL:
                str.appendf(" Soil moisture sensor:\n  Moisture: %d\n", ((Sensors::Soil*)node.data.data)->moisture);
                break;
            case Sensors::TYPE_TEMP_HUM:
                str.appendf(" Temperature + humidity sensor:\n  Temperature: %d\n  Humidity: %d\n",
                    ((Sensors::TempHum*)node.data.data)->temperature, ((Sensors::TempHum*)node.data.data)->humidity);
                break;
            case Sensors::TYPE_TEMP:
                str.appendf(" Temperature sensor:\n  Temperature: %d\n",
                    ((Sensors::Temp*)node.data.data)->temperature);
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
    gpio::set(led, false);

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

    gsm.AddSMSFunc({ "info", cmd_info, GSM::LEVEL_USER });
    gsm.AddSMSFunc({ "setname", [](GSM* gsm, const char* sender, const char* args) { net.CMDSetName(gsm, sender, args); }, GSM::LEVEL_USER });
    gsm.AddSMSFunc({ "clear", [](GSM* gsm, const char* sender, const char* args) { net.m_nodes.clear(); }, GSM::LEVEL_USER });

    SetupRTC();
    UpdateRTC();

    while(1) {
        gsm.Poll();
        net.Poll();
        
        gpio::toggle(led);
        //auto start = millis();
        //while(millis() - start < 500);

        if(rtc_count >= 10) {
            rtc_count = 0;
            UpdateRTC();
            if(net.m_nodes.size()) {
                Str str;
                for(auto& node : net.m_nodes) {
                    str.appendf("%02d/%02d/%02d %02d:%02d - ID %d name %.8s type %d",
                        RTC->MODE2.CLOCK.bit.YEAR, RTC->MODE2.CLOCK.bit.MONTH, RTC->MODE2.CLOCK.bit.DAY,
                        RTC->MODE2.CLOCK.bit.HOUR, RTC->MODE2.CLOCK.bit.MINUTE,
                        node.data.id, node.data.name, node.data.type);
                    switch(node.data.type) {
                        case Sensors::TYPE_SOIL:
                        case Sensors::TYPE_TEMP:
                            str.appendf(" value %d", *(uint16_t*)node.data.data);
                            break;
                        case Sensors::TYPE_NODEBASE:
                            str.appendf(" voltage %d light %d rain %d",
                                ((Sensors::Nodebase*)node.data.data)->voltage,
                                ((Sensors::Nodebase*)node.data.data)->light,
                                ((Sensors::Nodebase*)node.data.data)->rain);
                            break;
                    }
                    str.append('\n');
                }
                gsm.FTPWrite("data.txt", str.c_str());
            }
        }
    }
}

void SetupRTC() {
    GCLK->GENDIV.reg =
        GCLK_GENDIV_ID(1) | // Select generator 1
        GCLK_GENDIV_DIV(32); // Set the division factor to 32
        GCLK->GENCTRL.reg =
        GCLK_GENCTRL_ID(1) | // Select generator 1
        (3 << 8) | // Select source OSCULP32K
        GCLK_GENCTRL_GENEN; // Enable this generic clock generator
	while (GCLK->STATUS.bit.SYNCBUSY); // Wait for synchronization

	GCLK->CLKCTRL.reg =
        GCLK_CLKCTRL_ID_RTC | // Target is RTC
        GCLK_CLKCTRL_GEN(1) | // Select generator1 as source.
        GCLK_CLKCTRL_CLKEN; // Enable the RTC
	while (GCLK->STATUS.bit.SYNCBUSY); // Wait for synchronization

	RTC->MODE2.CTRL.reg =
        RTC_MODE2_CTRL_MODE_CLOCK |
        RTC_MODE2_CTRL_PRESCALER_DIV1024;

    // Interrupt every minute at 00 seconds
    RTC->MODE2.Mode2Alarm->ALARM.reg = 0;
	RTC->MODE2.Mode2Alarm->MASK.reg = RTC_MODE2_MASK_SEL_SS;
	RTC->MODE2.INTENSET.bit.ALARM0 = 1;
	NVIC_EnableIRQ(RTC_IRQn);
	
	RTC->MODE2.CTRL.bit.ENABLE = 1;
}

void UpdateRTC() {
    if(!gsm.Command("AT+CCLK?", "+CCLK: "))
        return;

    char* resp = gsm.m_uart->read();
    if(!resp) return;
    char* data = strstr(resp, "+CCLK: ");
    if(!data) {
        free(resp);
        return;
    }
    data += 8; // Skip +CCLK: "

    RTC->MODE2.CLOCK.bit.YEAR = atoi(data);
	RTC->MODE2.CLOCK.bit.MONTH = atoi(data + 3);
	RTC->MODE2.CLOCK.bit.DAY = atoi(data + 6);
	RTC->MODE2.CLOCK.bit.HOUR = atoi(data + 9);
	RTC->MODE2.CLOCK.bit.MINUTE = atoi(data + 12);
	RTC->MODE2.CLOCK.bit.SECOND = atoi(data + 15);

    free(resp);
}

void RTC_Handler() {
	// Clear interrupt flag
	RTC->MODE2.INTFLAG.reg = RTC_MODE2_INTFLAG_ALARM0;
	rtc_count++;
}