/* -----------------------------------------------------------------------------
 * Main entry point for DWIN T5L firmware using SDCC.
 * Initializes system, UART, and DGUS handling loop.
 * ----------------------------------------------------------------------------- */

#include "t5l1.h"
#include "uart.h"
#include "sys.h"
#include "encoder.h"

void main(void)
{
    Sys_Init();
    Uart_Init();

    u8 encoderState;
    s16 lastValue = 0;

    DGUS_Write_VP(0x5000, 123);

    while (1)
    {
        /* Check for incoming UART data and process it */
        DGUS_ProcessAllUarts();

        encoderState = Encoder_Read();
        
        switch (encoderState)
        {
        case ENCODER_RIGHT:
            lastValue += 1;
            break;
        case ENCODER_LEFT:
            lastValue -= 1;
            break;
        case ENCODER_BUTTON:
            lastValue += 50;
            break;
        default:
            continue;
        }

        DGUS_Write_VP(0x5000, (u16)lastValue);
    }
}
