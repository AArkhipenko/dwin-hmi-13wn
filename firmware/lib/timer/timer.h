/* -----------------------------------------------------------------------------
 * System timing library for DWIN T5L.
 * Provides 1ms base tick, delay function, and UART timeout handling.
 * ----------------------------------------------------------------------------- */
#ifndef __TIMER_H__
#define __TIMER_H__

#include "t5l1.h"
#include "config.h"
#include "uart.h"

extern volatile u32 t0_count;
extern volatile u32 t1_count;
extern volatile u16 sys_tick_ms;
extern volatile u16 sys_tick_rtc;
extern volatile u16 monitor_ms;

void delay_ms(u16 ms);
void Timer2_Init(void);
void Timer2_ISR(void) __interrupt(5);

#endif