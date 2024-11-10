/*
 * BackupRegister.c
 *
 *  Created on: Jun 30, 2024
 *      Author: abdelrhman mattar
 */



#include"BackupRegister.h"

void BKPREG_init(void){

	RCC->APB1ENR   |=   0x18000000;		// Enable the power and backup interface clocks
	PWR->CR        |=   0x00000100;		// Access to RTC and Backup registers enabled;

}



void BKPREG_deinit(void){

	RCC->APB1ENR   &=   ~(0x18000000);		// Enable the power and backup interface clocks
	PWR->CR        &=   ~(0x00000100);		// Access to RTC and Backup registers enabled;

}


void BKPREG_write(BkpregID BackupReg, uint32_t Data)
{
	uint32_t tmp = 0U;

	tmp = (uint32_t)BKP_BASE;
	tmp += (BackupReg * 4U);

	*(volatile uint32_t *) tmp = (Data & BKP_DR1_D);
}

uint32_t BKPREG_read(BkpregID BackupReg){
	  uint32_t backupregister = 0U;
	  uint32_t pvalue = 0U;

	  backupregister = (uint32_t)BKP_BASE;
	  backupregister += (BackupReg * 4U);

	  pvalue = (*(volatile uint32_t *)(backupregister)) & BKP_DR1_D;

	  return pvalue;
}
