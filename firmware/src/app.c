/* -----------------------------------------------------------------------------
 * Файл с исходным кодом пользовательского приложения
 * ----------------------------------------------------------------------------- */
#include "app.h"
#include "encoder.h"
#include "uart.h"

#define PORT_1 0x5000

void App_Init(void)
{
	DGUS_Write_VP(PORT_1, 0);
}

void App_Process(void)
{
    u16 newValue = DGUS_Read_VP(PORT_1);
	u8 encoderState = Encoder_Read();
	
	switch (encoderState)
	{
	case ENCODER_RIGHT:
		newValue += 1;
		break;
	case ENCODER_LEFT:
		newValue -= 1;
		break;
	case ENCODER_BUTTON:
		newValue += 50;
		break;
	default:
		return;
	}

	DGUS_Write_VP(PORT_1, newValue);
}