/**
 * \file
 *
 * \brief gcc starttup file for SAMD10
 *
 * Copyright (c) 2016 Atmel Corporation,
 *                    a wholly owned subsidiary of Microchip Technology Inc.
 *
 * \asf_license_start
 *
 * \page License
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the Licence at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * \asf_license_stop
 *
 */

#include <samd10.h>
#include <systick.h>

/* Initialize segments */
extern uint32_t _etext;
extern uint32_t _data;
extern uint32_t _edata;
extern uint32_t _bss;
extern uint32_t _ebss;
extern uint32_t _stack_top;
extern uint32_t _sfixed;

/** \cond DOXYGEN_SHOULD_SKIP_THIS */
int main(void);
/** \endcond */

void __libc_init_array(void);

/* Default empty handler */
void Dummy_Handler(void);

/* Cortex-M0+ core handlers */
void NMI_Handler             ( void ) __attribute__ ((weak, alias("Dummy_Handler"))) __attribute__((__used__));
void HardFault_Handler       ( void ) __attribute__ ((weak, alias("Dummy_Handler"))) __attribute__((__used__));
void SVC_Handler             ( void ) __attribute__ ((weak, alias("Dummy_Handler"))) __attribute__((__used__));
void PendSV_Handler          ( void ) __attribute__ ((weak, alias("Dummy_Handler"))) __attribute__((__used__));
void SysTick_Handler         ( void ) __attribute__ ((weak, alias("Dummy_Handler"))) __attribute__((__used__));

/* Peripherals handlers */
void PM_Handler              ( void ) __attribute__ ((weak, alias("Dummy_Handler"))) __attribute__((__used__));
void SYSCTRL_Handler         ( void ) __attribute__ ((weak, alias("Dummy_Handler"))) __attribute__((__used__));
void WDT_Handler             ( void ) __attribute__ ((weak, alias("Dummy_Handler"))) __attribute__((__used__));
void RTC_Handler             ( void ) __attribute__ ((weak, alias("Dummy_Handler"))) __attribute__((__used__));
void EIC_Handler             ( void ) __attribute__ ((weak, alias("Dummy_Handler"))) __attribute__((__used__));
void NVMCTRL_Handler         ( void ) __attribute__ ((weak, alias("Dummy_Handler"))) __attribute__((__used__));
void DMAC_Handler            ( void ) __attribute__ ((weak, alias("Dummy_Handler"))) __attribute__((__used__));
#ifdef ID_USB
void USB_Handler             ( void ) __attribute__ ((weak, alias("Dummy_Handler")));
#endif
void EVSYS_Handler           ( void ) __attribute__ ((weak, alias("Dummy_Handler"))) __attribute__((__used__));
void SERCOM0_Handler         ( void ) __attribute__ ((weak, alias("Dummy_Handler"))) __attribute__((__used__));
void SERCOM1_Handler         ( void ) __attribute__ ((weak, alias("Dummy_Handler"))) __attribute__((__used__));
#ifdef ID_SERCOM2
void SERCOM2_Handler         ( void ) __attribute__ ((weak, alias("Dummy_Handler")));
#endif
void TCC0_Handler            ( void ) __attribute__ ((weak, alias("Dummy_Handler"))) __attribute__((__used__));
void TC1_Handler             ( void ) __attribute__ ((weak, alias("Dummy_Handler"))) __attribute__((__used__));
void TC2_Handler             ( void ) __attribute__ ((weak, alias("Dummy_Handler"))) __attribute__((__used__));
void ADC_Handler             ( void ) __attribute__ ((weak, alias("Dummy_Handler"))) __attribute__((__used__));
void AC_Handler              ( void ) __attribute__ ((weak, alias("Dummy_Handler"))) __attribute__((__used__));
#ifdef ID_DAC
void DAC_Handler             ( void ) __attribute__ ((weak, alias("Dummy_Handler"))) __attribute__((__used__));
#endif
void PTC_Handler             ( void ) __attribute__ ((weak, alias("Dummy_Handler"))) __attribute__((__used__));

/* Exception Table */
__attribute__ ((section(".vectors")))
const DeviceVectors exception_table = {

        /* Configure Initial Stack Pointer, using linker-generated symbols */
        .pvStack                = (void*) (&_stack_top),

        .pfnReset_Handler       = (void*) Reset_Handler,
        .pfnNMI_Handler         = (void*) NMI_Handler,
        .pfnHardFault_Handler   = (void*) HardFault_Handler,
        .pfnReservedM12          = (void*) (0UL), /* Reserved */
        .pfnReservedM11          = (void*) (0UL), /* Reserved */
        .pfnReservedM10          = (void*) (0UL), /* Reserved */
        .pfnReservedM9           = (void*) (0UL), /* Reserved */
        .pfnReservedM8           = (void*) (0UL), /* Reserved */
        .pfnReservedM7           = (void*) (0UL), /* Reserved */
        .pfnReservedM6           = (void*) (0UL), /* Reserved */
        .pfnSVC_Handler         = (void*) SVC_Handler,
        .pfnReservedM4           = (void*) (0UL), /* Reserved */
        .pfnReservedM3           = (void*) (0UL), /* Reserved */
        .pfnPendSV_Handler      = (void*) PendSV_Handler,
        .pfnSysTick_Handler     = (void*) SysTick_Handler,

        /* Configurable interrupts */
        .pfnPM_Handler          = (void*) PM_Handler,             /*  0 Power Manager */
        .pfnSYSCTRL_Handler     = (void*) SYSCTRL_Handler,        /*  1 System Control */
        .pfnWDT_Handler         = (void*) WDT_Handler,            /*  2 Watchdog Timer */
        .pfnRTC_Handler         = (void*) RTC_Handler,            /*  3 Real-Time Counter */
        .pfnEIC_Handler         = (void*) EIC_Handler,            /*  4 External Interrupt Controller */
        .pfnNVMCTRL_Handler     = (void*) NVMCTRL_Handler,        /*  5 Non-Volatile Memory Controller */
        .pfnDMAC_Handler        = (void*) DMAC_Handler,           /*  6 Direct Memory Access Controller */
#ifdef ID_USB
        .pfnUSB_Handler         = (void*) USB_Handler,            /*  7 Universal Serial Bus */
#else
        .pfnReserved7            = (void*) (0UL),                  /*  7 Reserved */
#endif
        .pfnEVSYS_Handler       = (void*) EVSYS_Handler,          /*  8 Event System Interface */
        .pfnSERCOM0_Handler     = (void*) SERCOM0_Handler,        /*  9 Serial Communication Interface 0 */
        .pfnSERCOM1_Handler     = (void*) SERCOM1_Handler,        /* 10 Serial Communication Interface 1 */
#ifdef ID_SERCOM2
        .pfnSERCOM2_Handler     = (void*) SERCOM2_Handler,        /* 11 Serial Communication Interface 2 */
#else
        .pfnReserved11           = (void*) (0UL),                  /* 11 Reserved */
#endif
        .pfnTCC0_Handler        = (void*) TCC0_Handler,           /* 12 Timer Counter Control */
        .pfnTC1_Handler         = (void*) TC1_Handler,            /* 13 Basic Timer Counter 0 */
        .pfnTC2_Handler         = (void*) TC2_Handler,            /* 14 Basic Timer Counter 1 */
        .pfnADC_Handler         = (void*) ADC_Handler,            /* 15 Analog Digital Converter */
        .pfnAC_Handler          = (void*) AC_Handler,             /* 16 Analog Comparators */
#ifdef ID_DAC
        .pfnDAC_Handler         = (void*) DAC_Handler,            /* 17 Digital Analog Converter */
#else
        .pvReserved17           = (void*) (0UL),                  /* 17 Reserved */
#endif
        .pfnPTC_Handler         = (void*) PTC_Handler             /* 18 Peripheral Touch Controller */
};

/**
 * \brief This is the code that gets called on processor reset.
 * To initialize the device, and call the main() routine.
 */
void Reset_Handler(void)
{
        uint32_t *pSrc, *pDest;

        /* Initialize the relocate segment */
        pSrc = &_etext;
        pDest = &_data;

        if (pSrc != pDest) {
                for (; pDest < &_edata;) {
                        *pDest++ = *pSrc++;
                }
        }

        /* Clear the zero segment */
        for (pDest = &_bss; pDest < &_ebss;) {
                *pDest++ = 0;
        }

        /* Set the vector table base address */
        pSrc = (uint32_t *) & _sfixed;
        SCB->VTOR = ((uint32_t) pSrc & SCB_VTOR_TBLOFF_Msk);

        /* Change default QOS values to have the best performance and correct USB behaviour */
        SBMATRIX->SFR[SBMATRIX_SLAVE_HMCRAMC0].reg = 2;
#if defined(ID_USB)
        USB->DEVICE.QOSCTRL.bit.CQOS = 2;
        USB->DEVICE.QOSCTRL.bit.DQOS = 2;
#endif
        DMAC->QOSCTRL.bit.DQOS = 2;
        DMAC->QOSCTRL.bit.FQOS = 2;
        DMAC->QOSCTRL.bit.WRBQOS = 2;

        /* Overwriting the default value of the NVMCTRL.CTRLB.MANW bit (errata reference 13134) */
        NVMCTRL->CTRLB.bit.MANW = 1;

        // Initialize system clock before any constructors are called
        systick_init();

        __libc_init_array();

        /* Branch to main function */
        main();

        /* Infinite loop */
        while (1);
}

/**
 * \brief Default interrupt handler for unused IRQs.
 */
void Dummy_Handler(void)
{
        while (1) {
        }
}