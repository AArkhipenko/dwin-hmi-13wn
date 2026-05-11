/* -----------------------------------------------------------------------------
 * System timing library for DWIN T5L.
 * Provides 1ms base tick, delay function, and UART timeout handling.
 * ----------------------------------------------------------------------------- */
#ifndef __TIMER_H__
#define __TIMER_H__

#include "t5l1.h"
#include "config.h"
#include "uart.h"

extern volatile u16 sys_tick_ms;
extern volatile u16 monitor_ms;

/**
 * @brief Инициализация второго таймера
 */
void Timer2_Init(void);

/**
 * @brief Аппаратное прерывание таймера
 */
void Timer2_ISR(void) __interrupt(5);

/**
 * @brief Задержка в мс
 * 
 * @param ms - количество мс
 */
void delay_ms(u16 ms);

#endif