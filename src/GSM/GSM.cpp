#include "GSM.h"
#include <stdio.h>
#include <string.h>
#include <strings.h>
extern "C" {
#include <systick.h>
}

static int find_num(const char* str) {
    while(*str) {
        if(isdigit(*str))
            return atoi(str);
        str++;
    }
    return 0;
}

static void func_exec(GSM* gsm, const char* sender, const char* args) {
    if(!gsm->Command(args)) {
        gsm->SendSMS(sender, "Command timed out");
    } else {
        char* resp = gsm->m_uart->read();
        if(!resp) {
            gsm->SendSMS(sender, "uart->read() returned null");
        } else {
            gsm->SendSMS(sender, resp);

            free(resp);
        }
    }
}

GSM::GSM(const gpio_t pwrkey, uart_t* uart) :
    m_pwrkey(pwrkey), m_uart(uart)
{
    gpio::mode(m_pwrkey, GPIO_DIR_OUT); // Set pwrkey pin to output
    gpio::set(m_pwrkey, true); // Set pwrkey to high

    m_smsfuncs.push_back({"exec", func_exec, 0});
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
    Command("AT+CMGDA=\"DEL ALL\""); // Delete all sms

    m_uart->flush_rx();

    return true;
}

void GSM::Poll() {
    if(!m_uart->available()) return;

    // Wait for the whole message to come in (if it hasn't)
    delay_usec(100000);

    char* data = m_uart->read();
    if(!data) return;

    if(strstr(data, "RING")) {
        Command("ATH"); // Disconnect call
    } else if(strstr(data, "+CMTI")) {
        // Speed doesn't matter, we can call strstr twice
        ReadSMS(find_num(strstr(data, "+CMTI")));
    }

    free(data);
}

void GSM::ReadSMS(int index) {
    // TODO
    char buf[64];
    snprintf(buf, sizeof(buf), "AT+CMGR=%d", index);
    if(!Command(buf, "+CMGR"))
        return;

    // +CMGR: "REC UNREAD","+37250000000",.....\r\nDATA
    char* data = m_uart->read();
    if(!data) return;

    char* cmgr_start = strstr(data, "+CMGR"); // this shouldn't return null

    char* text = strchr(cmgr_start, '\n');
    if(!text) {
        free(data);
        return;
    }
    text += 1; // ignore \n
    char* text_end = strstr(text, "\r\n\r\nOK");
    if(!text_end) {
        free(data);
        return;
    }
    *text_end = '\0';

    char* sender = strstr(cmgr_start, ",\"");
    if(!sender) {
        free(data);
        return;
    }
    sender += 2;

    if(!strchr(sender, '"')) {
        free(data);
        return;
    }
    *strchr(sender, '"') = '\0';

    ProcessSMS(text, sender);

    free(data);
}

void GSM::ProcessSMS(const char* text, const char* sender) {
    char* args_start = strchr(text, ' ');
    if(args_start) {
        *args_start = '\0';
        args_start += 1;
    }

    for(auto& func : m_smsfuncs) {
        if(strcasecmp(func.key, text) == 0) {
            func.callback(this, sender, args_start);
            break;
        }
    }
}

bool GSM::SendSMS(const char* number, const char* text) {
    // TODO
    char buf[64];
    snprintf(buf, sizeof(buf), "AT+CMGS=\"%s\"", number);
    if(!Command(buf))
        return false;
    
    m_uart->print(text);

    return Command("\x1A", "+CMGS", 60000); // AT+CMGS max response time is 60 sec
}