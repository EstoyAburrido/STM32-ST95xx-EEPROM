/**
 * https://github.com/EstoyAburrido
 *
 * ST95xx Series EEPROM HAL SPI Library
 * Based on STM32 Standard Peripheral Libraries
 *
 * THIS SOFTWARE IS PROVIDED "AS IS"
 * AND ANY EXPRESS, IMPLIED OR STATUTORY WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
 * PARTICULAR PURPOSE AND NON-INFRINGEMENT OF THIRD PARTY INTELLECTUAL PROPERTY
 * RIGHTS ARE DISCLAIMED TO THE FULLEST EXTENT PERMITTED BY LAW. IN NO EVENT
 * SHALL STMICROELECTRONICS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _STM32_EEPROM_SPI_H
#define _STM32_EEPROM_SPI_H

/* C++ detection */
#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

/* M95040 SPI EEPROM defines */
#define EEPROM_WREN  0x06  /*!< Write Enable */
#define EEPROM_WRDI  0x04  /*!< Write Disable */
#define EEPROM_RDSR  0x05  /*!< Read Status Register */
#define EEPROM_WRSR  0x01  /*!< Write Status Register */
#define EEPROM_READ  0x03  /*!< Read from Memory Array */
#define EEPROM_WRITE 0x02  /*!< Write to Memory Array */

#define EEPROM_WIP_FLAG        0x01  /*!< Write In Progress (WIP) flag */



/**
 * @brief EEPROM Operations statuses
 */
typedef enum {
    EEPROM_STATUS_PENDING,
    EEPROM_STATUS_COMPLETE,
    EEPROM_STATUS_ERROR
} EepromOperations;

/**
 * @brief EEPROM Handler
 */
typedef struct{
  SPI_HandleTypeDef * SPI;
  GPIO_TypeDef * EEPROM_CS_GPIO_Port;
  uint16_t EEPROM_CS_Pin;
  GPIO_TypeDef * EEPROM_WP_GPIO_Port;
  uint16_t EEPROM_WP_Pin;
  GPIO_TypeDef * EEPROM_HOLD_GPIO_Port;
  uint16_t EEPROM_HOLD_Pin;
  uint16_t EEPROM_PAGESIZE;
} EEPROM_Handler;

void EEPROM_INIT(EEPROM_Handler * hndlr);
EepromOperations EEPROM_SPI_WriteBuffer(uint8_t* pBuffer, uint16_t WriteAddr, uint16_t NumByteToWrite);
EepromOperations EEPROM_WritePage(uint8_t* pBuffer, uint16_t WriteAddr, uint16_t NumByteToWrite);
EepromOperations EEPROM_SPI_ReadBuffer(uint8_t* pBuffer, uint16_t ReadAddr, uint16_t NumByteToRead);
uint8_t EEPROM_SPI_WaitStandbyState(void);

void EEPROM_CS_HIGH(void);
void EEPROM_CS_LOW(void);

void EEPROM_WP_HIGH(void);
void EEPROM_WP_LOW(void);

void EEPROM_HOLD_HIGH(void);
void EEPROM_HOLD_LOW(void);

void EEPROM_Delay(uint16_t msecs);

/* Low layer functions */
uint8_t EEPROM_SendByte(uint8_t byte);
void sEE_WriteEnable(void);
void sEE_WriteDisable(void);
void sEE_WriteStatusRegister(uint8_t regval);
uint8_t sEE_ReadStatusRegister(void);

void  EEPROM_SPI_SendInstruction(uint8_t *instruction, uint8_t size);
void  EEPROM_SPI_ReadStatusByte(SPI_HandleTypeDef SPIe, uint8_t *statusByte );

#ifdef __cplusplus
}
#endif

#endif // _STM32_EEPROM_SPI_H
