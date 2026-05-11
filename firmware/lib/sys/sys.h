/* -----------------------------------------------------------------------------
 * Header for system-level DGUS RAM and NOR Flash access routines.
 * Defines prototypes for VP read/write and graph/page control.
 * ----------------------------------------------------------------------------- */
#ifndef __SYS_H__
#define __SYS_H__

#include "t5l1.h"
#include "addresses.h"
#include "timer.h"

void Sys_Init(void);

/**
 * @brief Read a 16-bit value from DGUS RAM (addr = BYTE address).
 *        Even addr -> DATA3:DATA2, Odd addr -> DATA1:DATA0
 */
u16 DGUS_Read_VP(u16 addr);

/**
 * @brief Write a 16-bit value to DGUS RAM (addr = BYTE address).
 *        Even addr -> write DATA3:DATA2 (0x8C), Odd addr -> DATA1:DATA0 (0x83)
 */
void DGUS_Write_VP(u16 addr, u16 val);
void DGUS_WriteBytes(u16 addr, const u8 *buf, u16 len);
void DGUS_ReadBytes(u16 addr, u8 *buf, u16 words);
void DGUS_WriteText(u16 addr, const char *text);
u8 DGUS_GetPageID(void);
void DGUS_SetPageID(u8 page_id);
u8 DGUS_NOR_Write(u32 nor_addr, u16 vp_addr, u16 len_bytes);
u8 DGUS_NOR_Read(u32 nor_addr, u16 vp_addr, u16 len_bytes);
void DGUS_ResetHmi(void);
#endif