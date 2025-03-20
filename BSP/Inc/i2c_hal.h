#ifndef __I2C_HAL_H
#define __I2C_HAL_H

#include "stm32g4xx_hal.h"

void I2CStart(void);
void I2CStop(void);
unsigned char I2CWaitAck(void);
void I2CSendAck(void);
void I2CSendNotAck(void);
void I2CSendByte(unsigned char cSendByte);
unsigned char I2CReceiveByte(void);
void I2CInit(void);
void Write24C02(uint8_t *pucBuf, uint8_t ucAddr, uint8_t ucNum);
void Read24C02(uint8_t *pucBuf, uint8_t ucAddr, uint8_t ucNum);
void WriteMCP4017(uint8_t ucValue);
uint8_t ReadMCP4017(void);
#endif
