#include <msp430.h> 
#include <stdio.h>
#include "rtc.h"
#include "adc.h"

#define ADC_CHANNEL 0
#define TMP35_TURN_ON_TIME 2000

volatile uint8_t hour = 0;
volatile uint8_t minute = 0;
volatile uint8_t second = 0;

void get_time(void) {
    UNLOCK_RTC_REGISTERS();
    volatile uint16_t temp_rtciv = RTCIV;
    if (temp_rtciv == RTCIV__RTCTEVIFG) {
        RTCCTL0_L |= RTCRDYIE;
    } else if (temp_rtciv == RTCIV__RTCRDYIFG) {
        RTCCTL0_L &= ~(RTCRDYIE);

        hour = RTCHOUR;
        minute = RTCMIN;
        second = RTCSEC;

        P5OUT |= BIT0;
        __delay_cycles(TMP35_TURN_ON_TIME);
        request_adc_values();
    }
    LOCK_RTC_REGISTERS();
}

void read_sensor(void) {
    volatile uint16_t temp_adc12iv = ADC12IV;
    if (temp_adc12iv == ADC12IV__ADC12IFG0) {
        P5OUT &= ~BIT0;

        volatile uint16_t temp = ((ADC12MEM0 / 40.96) * 12) / 10;

        printf("Temperature: %d. At time %02d:%02d:%02d\n", temp, hour, minute, second);
    }
}

void config_cs(void) {
    CSCTL0_H = CSKEY >> 8;                    // Unlock CS registers
    CSCTL1 = DCOFSEL_0;                       // Set DCO to 1MHz
    CSCTL2 = SELS__DCOCLK | SELM__DCOCLK;     // SMCLK and MCLK source DCO
    CSCTL3 = DIVS__1 | DIVM__1;               // Set SMCLK and MCLK dividers
    CSCTL0_H = 0;                             // Lock CS registers
}

/**
 * main.c
 */
int main(void)
{
	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer
	
	config_cs();

	P5DIR |= BIT0;

    P1SEL1 |= BIT2;
    P1SEL0 |= BIT2;

	PM5CTL0 &= ~LOCKLPM5;

	set_rtc_callback(get_time);
	set_adc_callback(read_sensor);

	set_vref(VREF12);

	configure_adc(RES12BIT, ADC12IE0, 0, 0,
	              (uint16_t)(ADC12MCTLx(ADC_CHANNEL)));
	enable_channel_interrupt(ADC_CHANNEL); // This should be abstracted away

	enable_channel(ADC_CHANNEL, P12, ADC12VRSEL_1);

	RTCData data;
	data.year = 0x2024;
	data.month = 0x06;
	data.day = 0x07;
	data.day_of_week = 0x05;
	data.hour = 0x17;
	data.minutes = 0x30;
	data.seconds = 0x00;

	rtc_init(&data, true);

    rtc_event_select(RTCTEV__MIN);
    rtc_interrupt_enable(RTCTEVIE | RTCRDYIE, true);

	P5OUT |= BIT0;
    __delay_cycles(TMP35_TURN_ON_TIME);
	request_adc_values();

	rtc_enable(false);

	printf("got here\n");

	while (1) {
	    __bis_SR_register(LPM3_bits + GIE);
	}

	return 0;
}
