# STM32-ST95xx-EEPROM
ST95xx Series SPI EEPROM library for STM32 HAL

Usage example:
```
#include "STM32_EEPROM_SPI.h"

uint8_t RxBuffer[32];
uint8_t TxBuffer[32] = "This is the text to be written";
EEPROM_Handler eepromh;
static void EEPROM_Init(void);

int main(void)
{
  //We have to initialize HAL, SPI, etc. before using the library
  EEPROM_Init();
  EEPROM_SPI_WriteBuffer(TxBuffer, (uint16_t)0x01, (uint16_t)32);
  EEPROM_SPI_ReadBuffer(RxBuffer, (uint16_t)0x01, (uint16_t)32);
}

static void EEPROM_Init(void)
{
  eepromh.EEPROM_CS_Pin = GPIO_PIN_0; // Device selecting pin
  eepromh.EEPROM_CS_GPIO_Port = GPIOB;
  eepromh.EEPROM_HOLD_Pin = GPIO_PIN_1; // Hold pin
  eepromh.EEPROM_HOLD_GPIO_Port = GPIOB;
  eepromh.EEPROM_WP_Pin = GPIO_PIN_10; // Write protection pin
  eepromh.EEPROM_WP_GPIO_Port = GPIOB;
  eepromh.EEPROM_PAGESIZE = 128; // Page size accordingly to the documentation of an EEPROM chip
  eepromh.SPI = &hspi1;
  EEPROM_INIT(&eepromh);
}

```
