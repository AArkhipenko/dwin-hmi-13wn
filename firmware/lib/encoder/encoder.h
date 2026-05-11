#ifndef __ENCODER_H__
#define __ENCODER_H__

#include "t5l1.h"
#include "sys.h"
#include "uart.h"
#include "addresses.h"

void DGUS_ProcessAllEncoder(void);
u16 Read_Encoder(void);
u8 Read_Pin(u8 port,u8 pin);

#endif