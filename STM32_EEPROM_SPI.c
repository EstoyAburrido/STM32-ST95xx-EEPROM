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

#include "STM32_EEPROM_SPI.h"

EEPROM_Handler * eeprom_h;
uint8_t EEPROM_StatusByte;

/**
 * @brief Init EEPROM SPI
 *
 * @param hndlr Pointer to EEPROM struct handler
 */
void EEPROM_INIT(EEPROM_Handler * hndlr) {
  eeprom_h = hndlr;
}

/**
  * @brief  Writes more than one byte to the EEPROM with a single WRITE cycle
  *         (Page WRITE sequence).
  *
  * @note   The number of byte can't exceed the EEPROM page size.
  * @param  pBuffer: pointer to the buffer  containing the data to be written
  *         to the EEPROM.
  * @param  WriteAddr: EEPROM's internal address to write to.
  * @param  NumByteToWrite: number of bytes to write to the EEPROM, must be equal
  *         or less than "eeprom_h->EEPROM_PAGESIZE" value.
  * @retval EepromOperations value: EEPROM_STATUS_COMPLETE or EEPROM_STATUS_ERROR
  */
EepromOperations EEPROM_SPI_WritePage(uint8_t* pBuffer, uint16_t WriteAddr, uint16_t NumByteToWrite) {
    while (eeprom_h->SPI->State != HAL_SPI_STATE_READY) {
        EEPROM_Delay(1);
    }

    HAL_StatusTypeDef spiTransmitStatus;

    sEE_WriteEnable();

    /*
        We gonna send commands in one packet of 3 bytes
     */
    uint8_t header[3];

    header[0] = EEPROM_WRITE;   // Send "Write to Memory" instruction
    header[1] = WriteAddr >> 8; // Send 16-bit address
    header[2] = WriteAddr;

    // Select the EEPROM: Chip Select low
    EEPROM_CS_LOW();

    EEPROM_SPI_SendInstruction((uint8_t*)header, 3);

    // Make 5 attemtps to write the data
    for (uint8_t i = 0; i < 5; i++) {
        spiTransmitStatus = HAL_SPI_Transmit(eeprom_h->SPI, pBuffer, NumByteToWrite, 100);

        if (spiTransmitStatus == HAL_BUSY) {
            EEPROM_Delay(5);
        } else {
            break;
        }
    }

    // Deselect the EEPROM: Chip Select high
    EEPROM_CS_HIGH();

    // Wait the end of EEPROM writing
    EEPROM_SPI_WaitStandbyState();

    // Disable the write access to the EEPROM
    sEE_WriteDisable();

    if (spiTransmitStatus == HAL_ERROR) {
        return EEPROM_STATUS_ERROR;
    } else {
        return EEPROM_STATUS_COMPLETE;
    }
}

/**
  * @brief  Writes block of data to the EEPROM. In this function, the number of
  *         WRITE cycles are reduced, using Page WRITE sequence.
  *
  * @param  pBuffer: pointer to the buffer  containing the data to be written
  *         to the EEPROM.
  * @param  WriteAddr: EEPROM's internal address to write to.
  * @param  NumByteToWrite: number of bytes to write to the EEPROM.
  * @retval EepromOperations value: EEPROM_STATUS_COMPLETE or EEPROM_STATUS_ERROR
  */
EepromOperations EEPROM_SPI_WriteBuffer(uint8_t* pBuffer, uint16_t WriteAddr, uint16_t NumByteToWrite) {
    uint16_t NumOfPage = 0, NumOfSingle = 0, Addr = 0, count = 0, temp = 0;
    uint16_t sEE_DataNum = 0;

    EepromOperations pageWriteStatus = EEPROM_STATUS_PENDING;

    Addr = WriteAddr % eeprom_h->EEPROM_PAGESIZE;
    count = eeprom_h->EEPROM_PAGESIZE - Addr;
    NumOfPage =  NumByteToWrite / eeprom_h->EEPROM_PAGESIZE;
    NumOfSingle = NumByteToWrite % eeprom_h->EEPROM_PAGESIZE;

    if (Addr == 0) { /* WriteAddr is eeprom_h->EEPROM_PAGESIZE aligned  */
        if (NumOfPage == 0) { /* NumByteToWrite < eeprom_h->EEPROM_PAGESIZE */
            sEE_DataNum = NumByteToWrite;
            pageWriteStatus = EEPROM_SPI_WritePage(pBuffer, WriteAddr, sEE_DataNum);

            if (pageWriteStatus != EEPROM_STATUS_COMPLETE) {
                return pageWriteStatus;
            }

        } else { /* NumByteToWrite > eeprom_h->EEPROM_PAGESIZE */
            while (NumOfPage--) {
                sEE_DataNum = eeprom_h->EEPROM_PAGESIZE;
                pageWriteStatus = EEPROM_SPI_WritePage(pBuffer, WriteAddr, sEE_DataNum);

                if (pageWriteStatus != EEPROM_STATUS_COMPLETE) {
                    return pageWriteStatus;
                }

                WriteAddr +=  eeprom_h->EEPROM_PAGESIZE;
                pBuffer += eeprom_h->EEPROM_PAGESIZE;
            }

            sEE_DataNum = NumOfSingle;
            pageWriteStatus = EEPROM_SPI_WritePage(pBuffer, WriteAddr, sEE_DataNum);

            if (pageWriteStatus != EEPROM_STATUS_COMPLETE) {
                return pageWriteStatus;
            }
        }
    } else { /* WriteAddr is not eeprom_h->EEPROM_PAGESIZE aligned  */
        if (NumOfPage == 0) { /* NumByteToWrite < eeprom_h->EEPROM_PAGESIZE */
            if (NumOfSingle > count) { /* (NumByteToWrite + WriteAddr) > eeprom_h->EEPROM_PAGESIZE */
                temp = NumOfSingle - count;
                sEE_DataNum = count;
                pageWriteStatus = EEPROM_SPI_WritePage(pBuffer, WriteAddr, sEE_DataNum);

                if (pageWriteStatus != EEPROM_STATUS_COMPLETE) {
                    return pageWriteStatus;
                }

                WriteAddr +=  count;
                pBuffer += count;

                sEE_DataNum = temp;
                pageWriteStatus = EEPROM_SPI_WritePage(pBuffer, WriteAddr, sEE_DataNum);
            } else {
                sEE_DataNum = NumByteToWrite;
                pageWriteStatus = EEPROM_SPI_WritePage(pBuffer, WriteAddr, sEE_DataNum);
            }

            if (pageWriteStatus != EEPROM_STATUS_COMPLETE) {
                return pageWriteStatus;
            }
        } else { /* NumByteToWrite > eeprom_h->EEPROM_PAGESIZE */
            NumByteToWrite -= count;
            NumOfPage =  NumByteToWrite / eeprom_h->EEPROM_PAGESIZE;
            NumOfSingle = NumByteToWrite % eeprom_h->EEPROM_PAGESIZE;

            sEE_DataNum = count;

            pageWriteStatus = EEPROM_SPI_WritePage(pBuffer, WriteAddr, sEE_DataNum);

            if (pageWriteStatus != EEPROM_STATUS_COMPLETE) {
                return pageWriteStatus;
            }

            WriteAddr +=  count;
            pBuffer += count;

            while (NumOfPage--) {
                sEE_DataNum = eeprom_h->EEPROM_PAGESIZE;

                pageWriteStatus = EEPROM_SPI_WritePage(pBuffer, WriteAddr, sEE_DataNum);

                if (pageWriteStatus != EEPROM_STATUS_COMPLETE) {
                    return pageWriteStatus;
                }

                WriteAddr +=  eeprom_h->EEPROM_PAGESIZE;
                pBuffer += eeprom_h->EEPROM_PAGESIZE;
            }

            if (NumOfSingle != 0) {
                sEE_DataNum = NumOfSingle;

                pageWriteStatus = EEPROM_SPI_WritePage(pBuffer, WriteAddr, sEE_DataNum);

                if (pageWriteStatus != EEPROM_STATUS_COMPLETE) {
                    return pageWriteStatus;
                }
            }
        }
    }

    return EEPROM_STATUS_COMPLETE;
}

/**
  * @brief  Reads a block of data from the EEPROM.
  *
  * @param  pBuffer: pointer to the buffer that receives the data read from the EEPROM.
  * @param  ReadAddr: EEPROM's internal address to read from.
  * @param  NumByteToRead: number of bytes to read from the EEPROM.
  * @retval None
  */
EepromOperations EEPROM_SPI_ReadBuffer(uint8_t* pBuffer, uint16_t ReadAddr, uint16_t NumByteToRead) {
    while (eeprom_h->SPI->State != HAL_SPI_STATE_READY) {
        EEPROM_Delay(1);
    }

    /*
        We gonna send all commands in one packet of 3 bytes
     */

    uint8_t header[3];

    header[0] = EEPROM_READ;    // Send "Read from Memory" instruction
    header[1] = ReadAddr >> 8;  // Send 16-bit address
    header[2] = ReadAddr;

    // Select the EEPROM: Chip Select low
    EEPROM_CS_LOW();

    /* Send WriteAddr address byte to read from */
    EEPROM_SPI_SendInstruction(header, 3);

    while (HAL_SPI_Receive(eeprom_h->SPI, (uint8_t*)pBuffer, NumByteToRead, 2000) == HAL_BUSY) {
        EEPROM_Delay(1);
    };

    // Deselect the EEPROM: Chip Select high
    EEPROM_CS_HIGH();

    return EEPROM_STATUS_COMPLETE;
}

/**
  * @brief  Sends a byte through the SPI interface and return the byte received
  *         from the SPI bus.
  *
  * @param  byte: byte to send.
  * @retval The value of the received byte.
  */
uint8_t EEPROM_SendByte(uint8_t byte) {
    uint8_t answerByte;

    /* Loop while DR register in not empty */
    while (eeprom_h->SPI->State == HAL_SPI_STATE_RESET) {
        EEPROM_Delay(1);
    }

    /* Send byte through the SPI peripheral */
    if (HAL_SPI_Transmit(eeprom_h->SPI, &byte, 1, 200) != HAL_OK) {
        Error_Handler();
    }

    /* Wait to receive a byte */
    while (eeprom_h->SPI->State == HAL_SPI_STATE_RESET) {
        EEPROM_Delay(1);
    }

    /* Return the byte read from the SPI bus */
    if (HAL_SPI_Receive(eeprom_h->SPI, &answerByte, 1, 200) != HAL_OK) {
        Error_Handler();
    }

    return (uint8_t)answerByte;
}
/**
  * @brief  Enables the write access to the EEPROM.
  *
  * @param  None
  * @retval None
  */
void sEE_WriteEnable(void) {
    // Select the EEPROM: Chip Select low
    EEPROM_CS_LOW();

    uint8_t command[1] = { EEPROM_WREN };
    /* Send "Write Enable" instruction */
    EEPROM_SPI_SendInstruction((uint8_t*)command, 1);

    // Deselect the EEPROM: Chip Select high
    EEPROM_CS_HIGH();
}

/**
  * @brief  Disables the write access to the EEPROM.
  *
  * @param  None
  * @retval None
  */
void sEE_WriteDisable(void) {
    // Select the EEPROM: Chip Select low
    EEPROM_CS_LOW();

    uint8_t command[1] = { EEPROM_WRDI };

    /* Send "Write Disable" instruction */
    EEPROM_SPI_SendInstruction((uint8_t*)command, 1);

    // Deselect the EEPROM: Chip Select high
    EEPROM_CS_HIGH();
}

/**
  * @brief  Write new value in EEPROM Status Register.
  *
  * @param  regval : new value of register
  * @retval None
  */
void sEE_WriteStatusRegister(uint8_t regval) {
    uint8_t command[2];

    command[0] = EEPROM_WRSR;
    command[1] = regval;

    // Enable the write access to the EEPROM
    sEE_WriteEnable();

    // Select the EEPROM: Chip Select low
    EEPROM_CS_LOW();

    // Send "Write Status Register" instruction
    // and Regval in one packet
    EEPROM_SPI_SendInstruction((uint8_t*)command, 2);

    // Deselect the EEPROM: Chip Select high
    EEPROM_CS_HIGH();

    sEE_WriteDisable();
}


/**
  * @brief  Polls the status of the Write In Progress (WIP) flag in the EEPROM's
  *         status register and loop until write operation has completed.
  *
  * @param  None
  * @retval None
  */
uint8_t EEPROM_SPI_WaitStandbyState(void) {
    uint8_t sEEstatus[1] = { 0x00 };
    uint8_t command[1] = { EEPROM_RDSR };

    // Select the EEPROM: Chip Select low
    EEPROM_CS_LOW();

    // Send "Read Status Register" instruction
    EEPROM_SPI_SendInstruction((uint8_t*)command, 1);

    // Loop as long as the memory is busy with a write cycle
    do {

        while (HAL_SPI_Receive(eeprom_h->SPI, (uint8_t*)sEEstatus, 1, 200) == HAL_BUSY) {
            EEPROM_Delay(1);
        };

        EEPROM_Delay(1);

    } while ((sEEstatus[0] & EEPROM_WIP_FLAG) == SET); // Write in progress

    // Deselect the EEPROM: Chip Select high
    EEPROM_CS_HIGH();

    return 0;
}

/**
 * @brief Low level function to send header data to EEPROM
 *
 * @param instruction array of bytes to send
 * @param size        data size in bytes
 */
void EEPROM_SPI_SendInstruction(uint8_t *instruction, uint8_t size) {
    while (eeprom_h->SPI->State == HAL_SPI_STATE_RESET) {
        EEPROM_Delay(1);
    }

    if (HAL_SPI_Transmit(eeprom_h->SPI, (uint8_t*)instruction, (uint16_t)size, 200) != HAL_OK) {
        Error_Handler();
    }
}
/**
 *
 */
void EEPROM_CS_HIGH(void){
  HAL_GPIO_WritePin(eeprom_h->EEPROM_CS_GPIO_Port, eeprom_h->EEPROM_CS_Pin, GPIO_PIN_SET);
}
/**
 *
 */
void EEPROM_CS_LOW(void){
  HAL_GPIO_WritePin(eeprom_h->EEPROM_CS_GPIO_Port, eeprom_h->EEPROM_CS_Pin, GPIO_PIN_RESET);
}

/**
 *
 */
void EEPROM_WP_HIGH(void){
  HAL_GPIO_WritePin(eeprom_h->EEPROM_WP_GPIO_Port, eeprom_h->EEPROM_WP_Pin, GPIO_PIN_SET);
}
/**
 *
 */
void EEPROM_WP_LOW(void){
  HAL_GPIO_WritePin(eeprom_h->EEPROM_WP_GPIO_Port, eeprom_h->EEPROM_WP_Pin, GPIO_PIN_RESET);
}

/**
 *
 */
void EEPROM_HOLD_HIGH(void){
  HAL_GPIO_WritePin(eeprom_h->EEPROM_HOLD_GPIO_Port, eeprom_h->EEPROM_HOLD_Pin, GPIO_PIN_SET);
}
/**
 *
 */
void EEPROM_HOLD_LOW(void){
  HAL_GPIO_WritePin(eeprom_h->EEPROM_HOLD_GPIO_Port, eeprom_h->EEPROM_HOLD_Pin, GPIO_PIN_RESET);
}

/**
 *
 */
void EEPROM_Delay(uint16_t msecs){
  //osDelay(msecs);
  HAL_Delay(msecs);
}
