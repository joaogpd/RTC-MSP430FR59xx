#include "rtc.h"

volatile rtc_cb_t rtc_callback = NULL;

void set_rtc_callback(rtc_cb_t cb) {
    rtc_callback = cb;
}

static void rtc_cs_gpio_init(void) {
    PJSEL0 |= BIT4 | BIT5;
}

void cs_config_rtc(void) {
    rtc_cs_gpio_init();

    UNLOCK_CS_REGISTERS();

    CSCTL4 &= ~LFXTOFF; // Enable LFXTCLK
    do {
        CSCTL5 &= ~LFXTOFFG;
        SFRIFG1 &= ~OFIFG;
    } while (SFRIFG1 & OFIFG); // Wait for oscillator fault flag to settle

    LOCK_CS_REGISTERS(); // Lock CS
}

void rtc_enable(bool csinit) {
    if (csinit) {
        cs_config_rtc();
    }

    UNLOCK_RTC_REGISTERS();

    RTCCTL1 = RTCHOLD; // Stop RTC module
    RTCCTL1 |= RTCBCD | RTCMODE; // BCD Mode, Calendar Mode
    RTCCTL1 &= ~RTCHOLD; // Enables RTC module

    LOCK_RTC_REGISTERS();
}

void rtc_interrupt_enable(uint8_t intmask) {
    UNLOCK_RTC_REGISTERS();

    RTCCTL1 = RTCHOLD;
    RTCCTL0 = intmask;

    LOCK_RTC_REGISTERS();
}

int rtc_init(RTCData* data, bool csinit) {
    if (data == NULL) {
        return -1;
    }

    if (csinit) {
        cs_config_rtc();
    }

    UNLOCK_RTC_REGISTERS();

    RTCCTL1 = RTCHOLD;           // Stop RTC Module
    RTCCTL1 |= RTCBCD | RTCMODE; // BCD Mode, Calendar Mode

    // IMPORTANT: do the next two steps before writing to RTC time registers

    // Clear alarm interrupt flag and interrupt enable bits
    RTCCTL0_L &= ~(RTCAIFG | RTCAIE);

    // Clear all alarm enable bits and alarm values
    RTCAMIN = 0x00;
    RTCAHOUR = 0x00;
    RTCADOW = 0x00;
    RTCADAY = 0x00;

    // Write to RTC time registers
    RTCYEAR = data->year;
    RTCMON = data->month;
    RTCDAY = data->day;
    RTCDOW = data->day_of_week;
    RTCHOUR = data->hour;
    RTCMIN = data->minutes;
    RTCSEC = data->seconds;

    LOCK_RTC_REGISTERS();

    return 0;
}

#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=RTC_VECTOR
__interrupt void RTC_ISR(void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(RTC_VECTOR))) RTC_ISR (void)
#else
#error Compiler not supported!
#endif
{
    if (rtc_callback != NULL) {
        rtc_callback();
    }
}
