/*
 * FLASH_OPERATIONS.c
 *
 *  Created on: Jun 30, 2024
 *      Author: abdelrhman mattar
 */


#include <FLASH_OPERATIONS.h>
#include "bootloader.h"
#include <string.h>


uint32_t Flash_Read(uint32_t address, uint8_t *data, uint32_t length)
{
    while(length--)
    {
        *(data++)=*((uint8_t*)address++);
    }
		return 0;
}

uint32_t Flash_Erase(uint32_t address)
{
    FLASH_EraseInitTypeDef FlashSet;
    FlashSet.TypeErase = FLASH_TYPEERASE_PAGES;
    FlashSet.PageAddress = address;
    FlashSet.NbPages = 1;
    /*设置PageError，调用擦除函数*/
    uint32_t PageError = 0;
    HAL_FLASH_Unlock();
    HAL_FLASHEx_Erase(&FlashSet, &PageError);
    HAL_FLASH_Lock();
		return 0;
}

uint32_t Flash_Write(uint32_t address, uint8_t *data, uint32_t length)
{
    uint8_t buff[4] = {0xFF, 0xFF, 0xFF, 0xFF};
    uint32_t len = length / 4;
    // 解锁Flash
    HAL_FLASH_Unlock();

    // 写入数据到指定地址
    while(len--)
    {
      HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, address, *((uint32_t *)data));
      address += 4;
      data += 4;
    }

    len = length % 4;

    for(int i = 0; i < len; i++)
      buff[i] = data[i];

    HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, address, *((uint32_t *)buff));

    // 重新锁定Flash
    HAL_FLASH_Lock();
		return 0;
}

static volatile uint8_t buffer[1024],buffer2[1024] ;

uint8_t swap(void)
{
  uint32_t app_1=FIRST_TARGET_ADDRESS;//points to the 1st address of application1
  uint32_t app_2=SECOND_TARGET_ADDRESS;//points to the 1st address of application2

    int mem1 = 33*1024;
    int mem2 = 33*1024;
    int limit;
    if(mem1>mem2)
        limit= mem1;
    else
        limit= mem2;

   int lm = limit/1024;

    for(int i=1; i<=lm; i++,app_1+=1024,app_2+=1024)
    {
    	BL_SendMessage("LOOP %d\n\r",i);
    	memcpy(buffer,(uint8_t*) app_1, 1024);
    	memcpy(buffer2,(uint8_t*) app_2, 1024);
    	Flash_Erase((app_1));
    	Flash_Erase((app_2));
    	Flash_Write(app_1,buffer2,  1024);
    	Flash_Write(app_2,buffer, 1024);
    }
    return 1;
}


uint8_t BL_Address_Varification(uint32_t Addresss)
{
	uint8_t Adress_varfiy=1;
	if(Addresss>=FLASH_BASE &&Addresss<STM32F103_FLASH_END)
	{
		Adress_varfiy=1;
	}
	else if(Addresss>=SRAM_BASE &&Addresss<STM32F103_SRAM_END)
	{
		Adress_varfiy=1;
	}
	else{
		Adress_varfiy=0;
	}
	return Adress_varfiy;
}
