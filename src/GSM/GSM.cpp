#include "GSM.h"
#include <stdio.h>
#include <string.h>
#include <strings.h>
extern "C" {
#include <systick.h>
#include <flash.h>
}

#define VERSION_MAJ "1"
#define VERSION_MIN "0"
#define VERSION VERSION_MAJ "." VERSION_MIN " built " __DATE__ " " __TIME__

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

static void func_setnum(GSM* gsm, const char* sender, const char* c_args) {
    char* num = NULL;
    char* level = NULL;
    char* args = NULL;

    if(c_args) {
        args = (char*)malloc(strlen(c_args));
        if(!args) {
            gsm->SendSMS(sender, "malloc failed");
            return;
        }
        strcpy(args, c_args);

        num = strtok(args, " ");
        level = strtok(NULL, " ");
    }

    if(num && level) {
        gsm->SetNumLevel(num, *level);

        gsm->SendSMS(sender, "OK");
    } else {
        gsm->SendSMS(sender, "Usage: setnum <number> <level>");
    }

    if(args) free(args);
}

static void func_delnum(GSM* gsm, const char* sender, const char* args) {
    if(!args) {
        gsm->SendSMS(sender, "Usage: delnum <number>");
        return;
    }

    gsm->RemoveNum(args);
}

static void func_listnum(GSM* gsm, const char* sender, const char* args) {
    Str str = "List:\n";
    Vector<char*> list;
    gsm->GetAllNum(&list);
    for(auto num : list) {
        str.appendf("%s: %c\n", num, gsm->GetNumLevel(num));
        free(num);
    }

    gsm->SendSMS(sender, str.c_str());
}

static void func_listcmd(GSM* gsm, const char* sender, const char* args) {
    Str str;

    for(auto& cmd : gsm->m_smsfuncs) {
        str.appendf("%s\n", cmd.key);
    }

    gsm->SendSMS(sender, str.c_str());
}

static void func_update(GSM* gsm, const char* sender, const char* c_args) {
    /*char* server = NULL;
    char* username = NULL;
    char* pass = NULL;
    char* args = NULL;

    if(c_args) {
        args = (char*)malloc(strlen(c_args));
        if(!args) {
            gsm->SendSMS(sender, "malloc failed");
            return;
        }
        strcpy(args, c_args);

        server = strtok(args, " ");
        username = strtok(NULL, " ");
        pass = strtok(NULL, " ");
    }

    if(server && username && pass) {
        if(gsm->PerformUpdate(server, username, pass))
            gsm->SendSMS(sender, "OK");
        else
            gsm->SendSMS(sender, "Update failed");
            
    } else {
        gsm->SendSMS(sender, "Usage: update <server> <username> <password>");
    }

    if(args) free(args);*/

    if(!gsm->PerformUpdate())
        gsm->SendSMS(sender, "Update failed");
}

GSM::GSM(const gpio_t pwrkey, uart_t* uart) :
    m_pwrkey(pwrkey), m_uart(uart)
{
    gpio::mode(m_pwrkey, GPIO_DIR_OUT); // Set pwrkey pin to output
    gpio::set(m_pwrkey, true); // Set pwrkey to high

    m_smsfuncs.push_back({"exec", func_exec, LEVEL_ADMIN});
    m_smsfuncs.push_back({"setnum", func_setnum, LEVEL_ADMIN});
    m_smsfuncs.push_back({"delnum", func_delnum, LEVEL_ADMIN});
    m_smsfuncs.push_back({"listnum", func_listnum, LEVEL_USER});
    m_smsfuncs.push_back({"update", func_update, LEVEL_ADMIN});
    m_smsfuncs.push_back({"help", func_listcmd, LEVEL_USER});
    m_smsfuncs.push_back({"setftpcreds", (SMSFuncCallback)&GSM::cmd_SetFTPCreds, LEVEL_USER});
}

GSM::~GSM() {
    
}

bool GSM::Command(const char* cmd, const char* result, unsigned int response_time) {
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

    // Check if the GSM module is already powered on
    if(Command("AT", "OK", 500))
        return true;
    
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

    // Reset cause: system means reset after firmware update
    if(PM->RCAUSE.bit.SYST) {
        Vector<char*> list;
        GetAllNum(&list);
        for(auto num : list) {
            SendSMS(num, "Update successful, version " VERSION);
            free(num);
        }
    }

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
            if(GetNumLevel(sender) >= func.level || GetAllNum() == 0)
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

    bool success = Command("\x1A", "+CMGS", 60000); // AT+CMGS max response time is 60 sec
    m_uart->flush_rx();
    return success;
}

char GSM::GetNumLevel(const char* num) {
    // TODO
    char buf[64];
    snprintf(buf, sizeof(buf), "AT+FSREAD=C:\\%s.txt,0,1,0", num);
    if(!Command(buf, "OK"))
        return 0;
    if(m_uart->available() < 3) return 0;

    m_uart->getc(); m_uart->getc(); // skip \r\n

    char level = m_uart->getc();
    m_uart->flush_rx();

    asm volatile("nop");

    return level;
}

void GSM::SetNumLevel(const char* num, char level) {
    // TODO
    char buf[64];
    snprintf(buf, sizeof(buf), "AT+FSCREATE=C:\\%s.txt", num);
    Command(buf);

    snprintf(buf, sizeof(buf), "AT+FSWRITE=C:\\%s.txt,0,1,1", num);
    if(!Command(buf, ">"))
        return;

    m_uart->write(level);

    Command("\r\n");
}

void GSM::RemoveNum(const char* num) {
    // TODO
    char buf[64];
    snprintf(buf, sizeof(buf), "AT+FSDEL=C:\\%s.txt", num);
    Command(buf);
}

int GSM::GetAllNum(Vector<char*>* list) {
    if(!Command("AT+FSLS=C:\\"))
        return 0;

    char* data = m_uart->read();
    if(!data) return 0;

    int count = 0;

    char* entry = strtok(data, "\r\n");
    while(entry) {
        if(strlen(entry) > 4 && strcmp(entry + strlen(entry) - 4, ".txt") == 0) {
            count++;
            if(list) {
                char* file = (char*)calloc(strlen(entry) - 4 + 1, 1);
                memcpy(file, entry, strlen(entry) - 4);
                list->push_back(file);
            }
        }
        entry = strtok(NULL, "\r\n");
    }

    free(data);

    return count;
}

bool GSM::InitInternet() {
    Command("AT+SAPBR=3,1,\"APN\",\"internet\"");
    Command("AT+SAPBR=1,1"); // Open bearer
    // Wait until we are connected
    if(!RepeatCommand("AT+SAPBR=2,1", "+SAPBR: 1,1", 25)) {
        Command("AT+SAPBR=0,1"); // Close bearer
        return false;
    }

    return true;
}

void GSM::cmd_SetFTPCreds(const char* sender, const char* args) {
    Command("AT+FSCREATE=C:\\ftp.dat");

    char buf[64];
    snprintf(buf, sizeof(buf), "AT+FSWRITE=C:\\ftp.dat,0,%d,1", strlen(args));
    if(!Command(buf, ">")) {
        SendSMS(sender, "Error");
        return;
    }

    m_uart->print(args);

    Command("\r\n");

    SendSMS(sender, "OK");
}

bool GSM::SetupFTP() {
    if(!Command("AT+FSREAD=C:\\ftp.dat,0,64,0", "OK"))
        return false;

    if(m_uart->available() < 3) return 0;

    m_uart->getc(); m_uart->getc(); // skip \r\n

    char* buf = m_uart->read();
    if(!buf) return false;

    char* host = NULL;
    char* uname = NULL;
    char* pass = NULL;

    host = strtok(buf, " ");
    uname = strtok(NULL, " ");
    pass = strtok(NULL, " \r");

    if(!host || !uname || !pass) {
        free(buf);
        return false;
    }

    m_uart->print("AT+FTPSERV=");
    m_uart->print(host);
    Command("");

    m_uart->print("AT+FTPUN=");
    m_uart->print(uname);
    Command("");

    m_uart->print("AT+FTPPW=");
    m_uart->print(pass);
    Command("");

    free(buf);
    return true;
}

bool GSM::FTPWrite(const char* fname, const char* text) {
    if(!SetupFTP()) return false;

	Command("AT+FTPPUTPATH=/");
    m_uart->print("AT+FTPPUTNAME=");
	Command(fname);
    
	Command("AT+FTPPUTOPT=\"APPE\"");
    
    if(!InitInternet())
        return false;

    // Max response time: 75 seconds (In case no response is received from server)
    Command("AT+FTPPUT=1");
    if(!Command("", "+FTPPUT: 1,1,", 75000)) {
        Command("AT+SAPBR=0,1"); // Close bearer
        return false;
    }

    char buf[64];
    snprintf(buf, sizeof(buf), "AT+FTPPUT=2,%d", strlen(text));
    Command(buf);
    
    m_uart->print(text);
    //Command("\r\n");

    //Command("AT+FTPPUT=2,0");
    
    if(!Command("AT+FTPPUT=2,0", "+FTPPUT: 1,0", 75000)) {
        Command("AT+SAPBR=0,1"); // Close bearer
        return false;
    }
    
    Command("AT+SAPBR=0,1"); // Close bearer

    return true;
}

bool GSM::PerformUpdate(/*const char* server, const char* username, const char* password*/) {
    Str cmd;

    if(!SetupFTP())
        return false;

    Command("AT+SAPBR=3,1,\"APN\",\"internet\"");
    /*cmd.setf("AT+FTPSERV=%s", server);
	Command(cmd.c_str());
    cmd.setf("AT+FTPUN=%s", username);
	Command(cmd.c_str());
    cmd.setf("AT+FTPPW=%s", password);
	Command(cmd.c_str());*/
	Command("AT+FTPGETPATH=/");
	Command("AT+FTPGETNAME=update.bin");

    if(!InitInternet())
        return false;

    // Max response time: 75 seconds (In case no response is received from server)
    Command("AT+FTPGETTOFS=0,update.bin"); // Get "OK" response first
    if(!Command("", "+FTPGETTOFS: 0,", 75000)) {
        Command("AT+SAPBR=0,1"); // Close bearer
        return false;
    }

    Command("AT+SAPBR=0,1"); // Close bearer

    __disable_irq(); // Disable all interrupts

    WriteUpdate();

    return true;
}

// Make sure that the string is stored in ram
static char fsread[] = "AT+FSREAD=C:\\User\\FTP\\update.bin,1,64,";
//static char cpowd[] = "AT+CPOWD=1";

void GSM::WriteUpdate() {
    flash_setup_write();

    bool done = false;
    for(unsigned int row = 0; row < FLASH_SIZE / (FLASH_PAGE_SIZE * 4); row++) {
        flash_erase_page((row * FLASH_PAGE_SIZE * 4) >> 1); // shift right for some reason
        for(unsigned int page = 0; page < 4; page++) {
            RAMFunc_print(fsread);
            RAMFunc_print_int(row * FLASH_PAGE_SIZE * 4 + page * FLASH_PAGE_SIZE);
            RAMFunc_write('\r');

            // Skip \r\n
            for(int i = 0; i < 2; i++) {
                RAMFunc_wait_rx();
                (void)m_uart->m_sercom->USART.DATA.reg; // Clear RXC flag
            }

            uint32_t acc;
            for(unsigned int i = 0; i < FLASH_PAGE_SIZE; i++) {
                if(!done) done = !RAMFunc_wait_rx();

                acc &= ~(0xFF << ((i & 3) * 8));
                if(!done) acc |= uint8_t(m_uart->m_sercom->USART.DATA.reg) << ((i & 3) * 8);

                if((i & 3) == 3) *(uint32_t*)(row * FLASH_PAGE_SIZE * 4 + page * FLASH_PAGE_SIZE + i - 3) = acc;
            }
            if(done) break;

            // Skip \r\nOK\r\n
            while(RAMFunc_wait_rx()) (void)m_uart->m_sercom->USART.DATA.reg;
        }
        if(done) break;
    }
    
    /*for(unsigned int i = 0; i < FLASH_SIZE / (FLASH_PAGE_SIZE * 4); i++) {
        flash_erase_page(i * FLASH_PAGE_SIZE * 4 >> 1);
        if(i) break;
        for(unsigned int j = 0; j < FLASH_PAGE_SIZE * 4; j += 4) {
            *(uint32_t*)(i * FLASH_PAGE_SIZE * 4 + j) = 0;
        }
    }*/

    //RAMFunc_print(cpowd); // Power down GSM module
    NVIC_SystemReset();
    while(1);
}

void GSM::RAMFunc_write(const char c) {
    m_uart->m_sercom->USART.DATA.reg = c;
	while(!m_uart->m_sercom->USART.INTFLAG.bit.DRE);
}

void GSM::RAMFunc_print(const char* c) {
    while(*c) RAMFunc_write(*c++);
}

static uint16_t subtractors[] = {10000, 1000, 100, 10, 1};
void GSM::RAMFunc_print_int(uint16_t u) {
	char n;
	uint16_t *sub = subtractors;
	uint8_t  i = 5;
	while (i > 1 && u < *sub) {
		i--;
		sub++;
	}
	while (i--) {
		n = '0';
		while (u >= *sub) {
			u -= *sub;
			n++;
		}
		RAMFunc_write(n);
		sub++;
	}
}

bool GSM::RAMFunc_wait_rx() {
    int timeout = 0;
    while(!m_uart->m_sercom->USART.INTFLAG.bit.RXC) {
        if(timeout++ >= 1000000) return false;
    }
    return true;
}