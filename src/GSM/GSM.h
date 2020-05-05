#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <gpio.h>
#include <uart.h>
#include <vector.h>
#include <Str.h>

class GSM;
typedef void(*SMSFuncCallback)(GSM*, const char*, const char*);

struct SMSFunc {
    const char* key;
    SMSFuncCallback callback;
    char level;
};

class GSM {
private:
    gpio_t m_pwrkey;
    Vector<SMSFunc> m_smsfuncs;

public:
    enum : char {
        LEVEL_USER = 'a',
        LEVEL_ADMIN = 'g'
    };

    uart_t* m_uart;

    GSM(const gpio_t pwrkey, uart_t* uart);
    ~GSM();

    bool PowerOn();
    bool Init();

    bool Command(const char* cmd, const char* result = NULL, unsigned int response_time = 3000);
    // Repeat a command until we get the desired result
    bool RepeatCommand(const char* cmd, const char* result, int repeats, int response_time = 3000);

    void Poll();

    void ReadSMS(int index);
    void ProcessSMS(const char* text, const char* sender);
    bool SendSMS(const char* number, const char* text);

    void AddSMSFunc(const SMSFunc& func) { m_smsfuncs.push_back(func); }

    // Get access level of a phone number
    char GetNumLevel(const char* num);
    void SetNumLevel(const char* num, char level);
    void RemoveNum(const char* num);
    int GetAllNum(Vector<char*>* list = NULL);
};