/* -----------------------------------------------------------------------------
 * System timing library for DWIN T5L.
 * Provides 1ms base tick, delay function, and UART timeout handling.
 * ----------------------------------------------------------------------------- */
#include "timer.h"

volatile u32 t0_count = 0;
volatile u32 t1_count = 0;
volatile u16 sys_tick_ms = 0;
volatile u16 sys_tick_rtc = 0;
volatile u16 monitor_ms = 0;

void delay_ms(u16 ms)
{
    u16 start = sys_tick_ms;
    while ((u16)(sys_tick_ms - start) < ms)
        ;
}

void Timer2_Init(void)
{
    TH2 = 0x00;
    TL2 = 0x00;
    T2CON = 0x70;
    TRL2H = 0xBC; // 1ms timer
    TRL2L = 0xCD;
    IEN0 |= 0x20; // Start timer 2
    TR2 = 0x01;
    ET2 = 1; // T2 timer interrupt enable control bit
    EA = 1;
}

// Timer2 interrupt service routine (1ms interval)
void Timer2_ISR(void) __interrupt(5)
{
    TF2 = 0;
    sys_tick_ms++;
    monitor_ms++;
    sys_tick_rtc++;

    if (T_O2 > 0)
        T_O2--;
}