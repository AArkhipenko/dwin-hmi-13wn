#ifndef __ENCODER_H__
#define __ENCODER_H__

#include "t5l1.h"

#define ENCODER_NO_EVENT 0
#define ENCODER_RIGHT 1
#define ENCODER_LEFT 2
#define ENCODER_BUTTON 3

/**
 * @brief Чтение значения энкодера
 * @return 0 - нет события, 1 - вправо, 2 - влево, 3 - кнопка
 */
u8 Encoder_Read(void);

#endif