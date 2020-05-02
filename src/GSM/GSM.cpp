#include "GSM.h"
#include <stdio.h>
#include <string.h>
extern "C" {
#include <systick.h>
}

GSM::GSM(const gpio_t pwrkey, uart_t* uart) :
    m_pwrkey(pwrkey), m_uart(uart)
{
    gpio::mode(m_pwrkey, GPIO_DIR_OUT); // Set pwrkey pin to output
    gpio::set(m_pwrkey, true); // Set pwrkey to high
}

GSM::~GSM() {
    
}

bool GSM::Command(const char* cmd, const char* result, int response_time) {
    uint32_t start;

    m_uart->flush_rx();

    m_uart->print(cmd);
    m_uart->write('\r');
    // Allow up to response_time milliseconds to get a response
    start = millis();
    while(!m_uart->available())
        if(millis() - start >= response_time)
            return false;
    
    delay_usec(100000); // wait 100 ms to get the whole response

    if(!result) return true;
    return m_uart->find(result) != -1;
}

bool GSM::RepeatCommand(const char* cmd, const char* result, int repeats, int response_time) {
    for(int i = 0; i < repeats; i++) {
        if(Command(cmd, result, response_time))
            return true;
        
        delay_usec(500000);
    }
    return false;
}

bool GSM::PowerOn() {
    m_uart->write('\r');
    
    // Fail after 3 attempts to power cycle
    for(int i = 0; i < 3; i++) {
        gpio::set(m_pwrkey, false); // Set pwrkey to low
        delay_usec(1000000); // Wait for SIM800C to register it
        gpio::set(m_pwrkey, true); // Set pwrkey to high
        delay_usec(1000000);

        // Wait for response to AT command
        if(RepeatCommand("AT", "OK", 6, 500))
            return true;
    }
    return false;
}

bool GSM::Init() {
    if(!PowerOn())
        return false;

    Command("ATE0");
    if(!RepeatCommand("AT+CREG?", "+CREG: 0,1", 25))
        return false;
    Command("AT+CMGF=1"); // SMS text mode

    m_uart->flush_rx();

    return true;
}

void GSM::Poll() {
    if(!m_uart->available()) return;

    // Wait for the whole message to come in (if it hasn't)
    delay_usec(100000);

    char* data = m_uart->read();

    asm volatile("nop");
    if(strstr(data, "RING")) {
        Command("ATH"); // Disconnect call
    } else if(strstr(data, "+CMTI")) {
        // Speed doesn't matter, we can call strstr twice
        int idx = -1;
        sscanf(strstr(data, "+CMTI"), "+CMTI: %*s %d", &idx);
        if(idx != -1) ProcessSMS(idx);
    }

    free(data);
}

void GSM::ProcessSMS(int index) {

}