/*

    Original idea & program
    https://os.mbed.com/users/Sissors/code/WakeUp/
        by Erik

    modified version
    https://os.mbed.com/users/kenjiArai/code/WakeUp/
 */

/*
 *  Modified only for STM CPU
 *      by Kenji Arai / JH1PJL
 *
 *  http://www7b.biglobe.ne.jp/~kenjia/
 *  http://mbed.org/users/kenjiArai/
 *      Created:    September  21st, 2017
 *      Revised:    January    17th, 2021
 */

#if \
    defined(TARGET_NUCLEO_F334R8)
#   error Not support yet
#elif \
    defined(TARGET_NUCLEO_F401RE)\
 || defined(TARGET_NUCLEO_F411RE)\
 || defined(TARGET_NUCLEO_F446RE)\
 || defined(TARGET_NUCLEO_L053R8)\
 || defined(TARGET_NUCLEO_L073RZ)\
 || defined(TARGET_NUCLEO_L152RE)\
 || defined(TARGET_NUCLEO_L476RG)\
 || defined(TARGET_DISCO_L475VG_IOT01A)

#include "WakeUp.h"
#include "rtc_api.h"

#define BYTE2BCD(byte)  ((byte % 10) | ((byte / 10) << 4))

//Most things are pretty similar between the different STM targets.
//Only the IRQ number the alarm is connected to differs. Any errors
//with RTC_IRQn/RTC_Alarm_IRQn in them are related to this
#if defined(TARGET_M4) || defined(TARGET_M3)
#define RTC_IRQ     RTC_Alarm_IRQn
#else
#define RTC_IRQ     RTC_IRQn
#endif

// Some things to handle Disco L476VG (and similar ones)
#if defined(TARGET_STM32L4)
#define IMR     IMR1
#define EMR     EMR1
#define RTSR    RTSR1
#define FTSR    FTSR2
#define PR      PR1
#endif

void WakeUp::set_ms(uint32_t ms)
{
    if (ms == 0) {              //Just disable alarm
        return;
    }

    if (!rtc_isenabled()) {     //Make sure RTC is running
        rtc_init();
        wait_us(250);           //The f401 seems to want a delay after init
    }

#if defined(TARGET_NUCLEO_L476RG) || defined(TARGET_DISCO_L475VG_IOT01A)
    PWR->CR1 |= PWR_CR1_DBP;    //Enable backup domain
#else
    PWR->CR |= PWR_CR_DBP;      //Enable backup domain
#endif
    RTC->WPR = 0xCA;            //Disable RTC write protection
    RTC->WPR = 0x53;

    //Alarm must be disabled to change anything
    RTC->CR &= ~RTC_CR_ALRAE;
    RTC->CR &= 0x00ff00ff;
    while(!(RTC->ISR & RTC_ISR_ALRAWF));

    //RTC prescaler + calculate how many sub-seconds should be added
    uint32_t prescaler = (RTC->PRER & 0x7FFF) + 1;
    uint32_t subsecsadd = ((ms % 1000) * prescaler) / 1000;

    if ((ms < 1000) && (subsecsadd < 2))
        subsecsadd = 2;//At least 5 subsecs delay to be sure interrupt is called

    __disable_irq();   //At this point we don't want IRQs anymore

    //Get current time
    uint32_t subsecs = RTC->SSR;
    time_t secs = rtc_read();

    //Calculate alarm values
    //Subseconds is countdown,
    //    so substract the 'added' sub-seconds and prevent underflow
    if (subsecs < subsecsadd) {
        subsecs += prescaler;
        secs++;
    }
    subsecs -= subsecsadd;

    //Set seconds correctly
    secs += ms / 1000;
    struct tm *timeinfo = localtime(&secs);

    //Enable rising edge EXTI interrupt of the RTC
    EXTI->IMR |= RTC_EXTI_LINE_ALARM_EVENT;     // enable it
    EXTI->EMR &= ~RTC_EXTI_LINE_ALARM_EVENT;    // disable event
    EXTI->RTSR |= RTC_EXTI_LINE_ALARM_EVENT;    // enable rising edge
    EXTI->FTSR &= ~RTC_EXTI_LINE_ALARM_EVENT;   // disable falling edge

    //Calculate alarm register values
    uint32_t alarmreg = 0;
    alarmreg |= BYTE2BCD(timeinfo->tm_sec)  << 0;
    alarmreg |= BYTE2BCD(timeinfo->tm_min)  << 8;
    alarmreg |= BYTE2BCD(timeinfo->tm_hour) << 16;
    alarmreg |= BYTE2BCD(timeinfo->tm_mday) << 24;
    alarmreg &= 0x3f3f7f7f; // All MSKx & WDSEL = 0

    //Enable RTC interrupt (use Alarm-A)
    RTC->ALRMAR = alarmreg;
    RTC->ALRMASSR = subsecs | RTC_ALRMASSR_MASKSS;      //Mask no subseconds
    RTC->CR |= RTC_CR_ALRAE | RTC_CR_ALRAIE;            //Enable Alarm-A

    RTC->WPR = 0xFF;        //Enable RTC write protection
#if defined(TARGET_NUCLEO_L476RG) || defined(TARGET_DISCO_L475VG_IOT01A)
    PWR->CR1 &= ~PWR_CR1_DBP;   //Disable backup domain
#else
    PWR->CR &= ~PWR_CR_DBP;     //Disable backup domain
#endif

    __enable_irq();         //Alarm is set, so irqs can be enabled again

    //Enable everything else
    NVIC_SetVector(RTC_IRQ, (uint32_t)WakeUp::irq_handler);
    NVIC_EnableIRQ(RTC_IRQ);
}

void WakeUp::standby_then_reset(uint32_t ms)
{
    if (ms == 0) {  // just go to Reset
        system_reset();
    }
    set_ms(ms);
#if defined(TARGET_NUCLEO_L476RG) || defined(TARGET_DISCO_L475VG_IOT01A)
    __HAL_RCC_PWR_CLK_ENABLE();
    HAL_PWREx_EnterSHUTDOWNMode();
#else
    PWR->CR |= PWR_CR_CWUF;
    HAL_PWR_EnterSTANDBYMode();
#endif
}

void WakeUp::irq_handler(void)
{
    //Clear RTC + EXTI interrupt flags
#if defined(TARGET_NUCLEO_L476RG) || defined(TARGET_DISCO_L475VG_IOT01A)
    PWR->CR1 |= PWR_CR1_DBP;    // Enable power domain
#else
    PWR->CR |= PWR_CR_DBP;      // Enable power domain
#endif
    RTC->ISR &= ~RTC_ISR_ALRAF;
#if defined(TARGET_NUCLEO_L476RG) || defined(TARGET_DISCO_L475VG_IOT01A)
    //RTC->CR &= 0x00ff00ff;      // just in case
#else
    RTC->CR &= 0x00ff00ff;      // just in case
#endif
    RTC->WPR = 0xCA;            // Disable RTC write protection
    RTC->WPR = 0x53;
    RTC->CR &= ~(RTC_CR_ALRAE | RTC_CR_ALRAIE); //Disable Alarm-A
    RTC->WPR = 0xFF;            // Enable RTC write protection
    EXTI->PR = RTC_EXTI_LINE_ALARM_EVENT;
#if defined(TARGET_NUCLEO_L476RG) || defined(TARGET_DISCO_L475VG_IOT01A)
    PWR->CR1 &= ~PWR_CR1_DBP;   // Disable power domain
#else
    PWR->CR &= ~PWR_CR_DBP;     // Disable power domain
#endif
    system_reset();
}

#endif