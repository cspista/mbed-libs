#include "WakeUp.h"

#if defined(TARGET_KL25Z)
Callback<void()> WakeUp::callback;
float WakeUp::cycles_per_ms = 1.0;

void WakeUp::irq_handler(void)
{
        WakeUp::callback.call();
}

extern "C" void LPTimer_IRQHandler(void)
{
    // 1. Megszakítási flag törlése (Write 1 to clear)
    LPTMR0->CSR |= LPTMR_CSR_TCF_MASK;

    // 2. Timer és megszakítás engedélyezés leállítása
    LPTMR0->CSR = 0;

    // 3. NVIC függő megszakítás törlése, hogy a deepsleep ne ébredjen fel azonnal
    NVIC_ClearPendingIRQ(LPTimer_IRQn);
    // WakeUp::irq_handler(); 
}

void WakeUp::set_ms(uint32_t ms)
{
    // Kikapcsolás
    LPTMR0->CSR = 0;

    if (ms == 0)
        return;

    // 16 bites compare regiszter
    if (ms > 0xFFFF)
        ms = 0xFFFF;

    // LPTMR órajel engedélyezése
    SIM->SCGC5 |= SIM_SCGC5_LPTMR_MASK;

    // 1 kHz LPO kiválasztása
    SIM->SOPT1 =
        (SIM->SOPT1 & ~SIM_SOPT1_OSC32KSEL_MASK) |
        SIM_SOPT1_OSC32KSEL(1);

    NVIC_ClearPendingIRQ(LPTimer_IRQn);
    NVIC_EnableIRQ(LPTimer_IRQn);

    // 1 kHz, prescaler bypass
    LPTMR0->PSR =
        LPTMR_PSR_PCS(1) |
        LPTMR_PSR_PBYP_MASK;

    // Compare
    LPTMR0->CMR = (uint16_t)ms;

    // Flag biztos törlése indulás előtt
    LPTMR0->CSR = LPTMR_CSR_TCF_MASK;

    // Interrupt + timer start
    LPTMR0->CSR =
        LPTMR_CSR_TIE_MASK |
        LPTMR_CSR_TEN_MASK;
}

void WakeUp::calibrate(void)
{
    cycles_per_ms = 1.0;
}

#endif