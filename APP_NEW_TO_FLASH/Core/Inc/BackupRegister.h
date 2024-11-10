/*
 * BackupRegister.h
 *
 *  Created on: Jun 30, 2024
 *      Author: abdelrhman mattar
 */

#ifndef INC_BACKUPREGISTER_H_
#define INC_BACKUPREGISTER_H_

	#define RTC_BKP_NUMBER           10

	#include <stdint.h>
#include "main.h"

	typedef enum{
		DR1 = 1, DR2, DR3, DR4, DR5, DR6, DR7, DR8, DR9, DR10
	}BkpregID;

	// Habilita escrita nos Backup registers
	void BKPREG_init(void);

	// Desabilita escrita nos Backup registers
	void BKPREG_deinit(void);

	// Escreve dado "Data" no endereço especificado em "BackupReg"
	void BKPREG_write(BkpregID BackupReg, uint32_t Data);

	// Retorna valor armazenado no endereço especificado em "BackupReg"
	uint32_t BKPREG_read(BkpregID BackupReg);

#endif /* INC_BACKUPREGISTER_H_ */
