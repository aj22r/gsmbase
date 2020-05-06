#include <stdint.h>
#include <samd10.h>

__always_inline void flash_set_addr(uint32_t addr) {
    NVMCTRL->ADDR.reg = addr;
}

__always_inline void flash_erase_page(uint32_t addr) {
    flash_set_addr(addr);
    NVMCTRL->CTRLA.reg = NVMCTRL_CTRLA_CMD_ER | NVMCTRL_CTRLA_CMDEX_KEY;
}

__always_inline void flash_write_dword(uint32_t data, uint32_t addr) {
    *(uint32_t*)(addr) = data;
}

__always_inline void flash_setup_write() {
    NVMCTRL->CTRLB.bit.MANW = 0;
}