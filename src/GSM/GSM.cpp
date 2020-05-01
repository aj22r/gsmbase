#include "GSM.h"
#include <stdio.h>
extern "C" {
#include <systick.h>
}

GSM::GSM(const gpio_t pwrkey, const uart_t uart) :
    m_pwrkey(pwrkey), m_uart(uart)
{
    gpio::mode(m_pwrkey, GPIO_DIR_OUT); // Set pwrkey pin to output
    gpio::set(m_pwrkey, true); // Set pwrkey to high
}

GSM::~GSM() {
    
}

bool GSM::Command(const char* cmd, const char* result) {
    uint32_t start;

    m_uart.flush_rx();

    m_uart.print(cmd);
    m_uart.write('\r');
    // Allow up to 3 seconds to get a response
    start = millis();
    while(!m_uart.available())
        if(millis() - start >= 3000)
            return false;
    
    delay_usec(100000); // wait 100 ms to get the whole response

    if(!result) return true;
    return m_uart.find(result) != -1;
}

bool GSM::RepeatCommand(const char* cmd, const char* result, int repeats) {
    for(int i = 0; i < repeats; i++) {
        if(Command(cmd, result))
            return true;
        
        delay_usec(500000);
    }
    return false;
}

bool GSM::PowerOn() {
    m_uart.write('\r');
    
    // Fail after 3 attempts to power cycle
    for(int i = 0; i < 3; i++) {
        gpio::set(m_pwrkey, false); // Set pwrkey to low
        delay_usec(500000); // Wait for SIM800C to register it
        gpio::set(m_pwrkey, true); // Set pwrkey to high
        delay_usec(1000000);

        // Wait for response to AT command
        if(RepeatCommand("AT", "OK", 6))
            return true;
    }
    return false;
}

bool GSM::Init() {
    if(!PowerOn())
        return false;

    Command("ATE0");
    RepeatCommand("AT+CREG?", "+CREG: 0,1", 25);
    Command("AT+CMGF=1"); // SMS text mode

    m_uart.flush_rx();
}