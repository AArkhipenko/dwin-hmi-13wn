#include "encoder.h"
#include "timer.h"

/**
 * Чтение пина
 */
u8 Read_Pin(u8 port, u8 pin)
{
	switch (port)
	{
	case 0:
		return (P0 & (1 << pin)) ? 1 : 0;
	case 1:
		return (P1 & (1 << pin)) ? 1 : 0;
	case 2:
		return (P2 & (1 << pin)) ? 1 : 0;
	default:
		return 0;
	}
}

/**
 * Функция возвращает событие (0-3) - без изменений
 */
u8 Encoder_Read(void)
{
	delay_ms(1);

	/** == Обработка нажатия кнопки == */
	static u8 prev_btn_state = 1;
	u8 button_current = Read_Pin(1, 2);

	if (button_current == 0 && prev_btn_state == 1)
	{
		prev_btn_state = 0;
		return ENCODER_BUTTON;
	}
	else
	{
		prev_btn_state = button_current;
	}

	/** == Обработка поворота энкодера == */
	/**
	 * Цикл изменения энкодера
	 * 1;1 -> 0;1 || 1;0 -> 0;0 -> 1;1
	 */
	static u8 prev_pin_state = 0b11;
	u8 pinA, pinB, pin_state;

	// ========== Чтение энкодера ==========
	pinA = Read_Pin(1, 0);
	pinB = Read_Pin(1, 1);
	pin_state = (pinA << 1 | pinB);

	// Состояние "без изменений" или переход в состояние "без изменений"
	if(pin_state == 0b11)
	{
		prev_pin_state = 0b11;
		return ENCODER_NO_EVENT;
	}

	// Начало изменения энкодера
	if(pin_state > 0)
	{
		prev_pin_state = pin_state;
	}

	// Завершение изменения энкодера
	if(prev_pin_state > 0 &&
		pin_state == 0)
	{
		u8 event = ENCODER_NO_EVENT;

		if(prev_pin_state == 0b01)
		{
			event = ENCODER_RIGHT;
		}
		else if(prev_pin_state == 0b10)
		{
			event = ENCODER_LEFT;
		}

		prev_pin_state = pin_state;
		return event;
	}

	return ENCODER_NO_EVENT;
}
