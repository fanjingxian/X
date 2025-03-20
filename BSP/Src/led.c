#include "lcd.h"

void LED_Show(uint8_t led, uint8_t value)
{
    HAL_GPIO_WritePin(GPIOD,GPIO_PIN_2,GPIO_PIN_SET);
    if(value)
        HAL_GPIO_WritePin(GPIOC,GPIO_PIN_8<<(led-1),GPIO_PIN_RESET);
    else
        HAL_GPIO_WritePin(GPIOC,GPIO_PIN_8<<(led-1),GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOD,GPIO_PIN_2,GPIO_PIN_RESET);
}

void Led_Init(void)
{
	for(uint8_t i= 1;i<9;i++){
			LED_Show(i,0);
	}
}

