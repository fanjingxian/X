#ifndef __HEAD_H__
#define __HEAD_H__

#include <stdio.h>
#include <string.h>
#include "stm32g4xx_hal.h"
#include "stdbool.h"
#include "math.h"
#include "stdarg.h"
#include "tim.h"
#include "gpio.h"
#include "adc.h"
#include "lcd.h"
#include "led.h"
#include "usart.h"
#include "rtc.h"

#define Key1 HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_0)
#define Key2 HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_1)
#define Key3 HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_2)
#define Key4 HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0)

void LCD_Disp(uint8_t Line, char *format,...);
void LCD_Proc(void);
void Key_Proc(void);
void My_RTC_SetTime(void);

#endif // !__HEAD_H__

