/* -----------------------------------------------------------------------------
 * Main entry point for DWIN T5L firmware using SDCC.
 * Initializes system, UART, and DGUS handling loop.
 * ----------------------------------------------------------------------------- */

#include "uart.h"
#include "sys.h"

void main(void)
{
    Sys_Init();
    Uart_Init();

    while (1)
    {
        /* Check for incoming UART data and process it */
        DGUS_ProcessAllUarts();
    }
}
