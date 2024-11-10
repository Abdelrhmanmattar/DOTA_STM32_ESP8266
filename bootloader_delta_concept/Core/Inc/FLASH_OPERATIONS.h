/*
 * FLASH_OPERATIONS.h
 *
 *  Created on: Jun 30, 2024
 *      Author: abdelrhman mattar
 */

#ifndef INC_FLASH_OPERATIONS_H_
#define INC_FLASH_OPERATIONS_H_
#include "stm32f103xb.h"
#include "stdio.h"
#include "stdint.h"



#define STM32F103_SRAM_SIZE         (20 * 1024)
#define STM32F103_FLASH_SIZE         (128 * 1024)
#define STM32F103_SRAM_END          (SRAM_BASE + STM32F103_SRAM_SIZE)
#define STM32F103_FLASH_END          (FLASH_BASE + STM32F103_FLASH_SIZE)


uint8_t BL_Address_Varification(uint32_t Addresss);
uint32_t Flash_Read(uint32_t address, uint8_t *data, uint32_t length);
uint32_t Flash_Write(uint32_t address, uint8_t *data, uint32_t length);
uint32_t Flash_Erase(uint32_t address);
uint8_t swap(void);
#endif /* INC_FLASH_OPERATIONS_H_ */
