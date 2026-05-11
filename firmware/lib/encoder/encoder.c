#include "encoder.h"

/**
 * Реакция на изменнеие состояния энкодера:
 *  - 0 - ничего не длается
 *  - 1, 2 - изменение порта
 */
void DGUS_ProcessAllEncoder(void)
{
	u16 enc_flag;
	u16 recent_enc_val;

	enc_flag = Read_Encoder();

	if(enc_flag == 0)
	{
		return;
	}
	uart_send_byte(2, 0x02);

	recent_enc_val = DGUS_Read_VP(Encoder_Value);
	uart_send_byte(2, 0x03);

	switch(enc_flag)
	{
		case 1:
			recent_enc_val += 1;
			break;
		case 2:
			recent_enc_val -= 1;
			break;
		default:
			return;
	}
	uart_send_byte(2, 0x04);

	DGUS_Write_VP(Encoder_Value, recent_enc_val);
	uart_send_byte(2, 0x05);
}

/**
 * Чтение состония энкодера
 */
u16 Read_Encoder(void)	
{
	u16 resul = 1;
	uart_send_str(2, "Hello\r\n");
	return resul;
}

/**
 * Чтение пина
 */
u8 Read_Pin(u8 port,u8 pin)
{	 
  	u8 value;
	switch(port)
	{
		// Вращение по часовой стрелке
		case 0:
			value = P0&(1<<pin);
			break;
		// Вращение против часовой стрелки
		case 1: 
			value = P1&(1<<pin);
			break;
		// Нажатие на экран
		case 2: 
			value = P2&(1<<pin);
			break;	
		default:
			value=0;
			break;				
	}
	return value;
}
