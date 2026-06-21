/* -----------------------------------------------------------------------------
 * Main entry point for DWIN T5L firmware using SDCC.
 * Initializes system, UART, and DGUS handling loop.
 * ----------------------------------------------------------------------------- */

#include "t5l1.h"
#include "uart.h"
#include "sys.h"
#include "app.h"

void main(void)
{
    Sys_Init();
    Uart_Init();
    App_Init();

    while (1)
    {
        // Мониторинг UART
        DGUS_ProcessAllUarts();

        // Выполнение пользовательского приложения
        App_Process();
    }
}
