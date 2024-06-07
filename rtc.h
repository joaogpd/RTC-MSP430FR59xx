#ifndef RTC_H
#define RTC_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <msp430.h>

typedef void(*rtc_cb_t)(void);

typedef struct RTC_data {
    uint16_t year;
    uint8_t month;
    uint8_t day;
    uint8_t day_of_week;
    uint8_t hour;
    uint8_t minutes;
    uint8_t seconds;
} RTCData;

/**
 * Sets the RTC callback global variable. If 'cb' is NULL, this
 * equates to removing a callback.
 */
void set_rtc_callback(rtc_cb_t cb);

/**
 * Performs CS (clock system) configuration for using the LFXTCLK
 * with the RTC.
 *
 * IMPORTANT: this may conflict with other CS module operations performed
 * elsewhere in a program so should be used with care. It can be called
 * automatically or not by 'rtc_enable' and 'rtc_init'.
 */
void cs_config_rtc(void);

/**
 * Initializes the RTC module without altering the time registers and enables
 * the RTC module for operation.
 */
void rtc_enable(bool csinit);

/**
 * Enables the selected interrupt sources by writing the intmask to the
 * appropriate register.
 *
 * IMPORTANT: if this function is called, it will hold the RTC module for
 * writing to the registers, so a call to 'rtc_enable' will be necessary.
 *
 * @param intmask Should contain a mask for the value to be written in the
 * RTC register responsible for enabling interrupt sources.
 */
void rtc_interrupt_enable(uint8_t intmask);

/**
 * Initializes RTC module and sets the values in the time registers.
 * The RTC module is not initialized and all interrupt sources are cleared.
 *
 * @param data Holds the values that will be set to the time registers.
 */
int rtc_init(RTCData* data, bool csinit);

#endif
