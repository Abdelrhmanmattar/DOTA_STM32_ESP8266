/*
 * bootloader.h
 *
 *  Created on: May 8, 2024
 *      Author: abdelrhman mattar
 */

#ifndef  _BOOTLOADER_H_
#define _BOOTLOADER_H_
#include "usart.h"
#include "string.h"
#include "stdint.h"

#define SESSION_CONTROL			(0X10)
#define PROGRAMMING_SESSION	    (0x03)
#define DOWNLOAD_REQUEST		(0X34)
#define TRANSFER_DATA			(0X36)
#define TRANSFER_EXIT			(0X37)
#define START_APP				(0X31)    //5A + LEN + CHIP_ID + OPERATION            //LEN = 2

#define READ_FLASH_ADDRESS (0x40)  //5A + LEN + CHIP_ID + OPERATION + 4BYTES READ  //LEN = 6
#define ERASE_FLASH        (0x41)       //5A + LEN + CHIP_ID + OPERATION             //LEN = 2
#define ROLL_BACK          (0x45)         //5A + LEN + CHIP_ID + OPERATION            //LEN = 2
#define GET_CHIP_ID        (0x20)         //5A + LEN + CHIP_ID + OPERATION            //LEN = 2

//frame structure
// [ length | ID_DEVICE |command | data]
// max data length is 128 bytes
// comaand 1 byte
// length 1 byte
// ID_DEVICE 1 byte
//FRAME_LENGTH  = 1+1+1+128 = 131

#define MAX_DATA_LENGTH			(128)
#define MAX_FRAME_LENGTH		(131)
#define MAX_CODE_SIZE           ((1024*33)-1)
#define MAX_patch_SIZE           ((1024*22)-1)

#define my_ID_CHIP              (0X23)

#define MAX_APP_SIZE            (1024*33)  //33KB
#define MAX_APP_PAGE            (33)  //33page

#define MAX_PATCH_SIZE          (1024*20)  //20KB
#define MAX_PATCH_PAGE          (20)  //20P

#define TOT_PAGES               (86)


#define FIRST_TARGET_ADDRESS    0x08008000
#define PATCH_TARGET_ADDRESS    0x08010400
#define SECOND_TARGET_ADDRESS   0x08015400


#define IDLE 0
#define BUSY 1
#define False 0
#define True 1


typedef enum connect_state
{
	waiting_ProgrammingSession=0,
	waiting_DownloadRequest=1,
	waiting_TransferData=2,
	waiting_TransferExit=3,
	waiting_CheckCRC=4
}connect_state;


typedef void (*pFunction)(void); /*!< Function pointer definition */

typedef void (*fnc_ptr)(void);

typedef enum{
	BL_NACK=0,
	BL_ACK
}BL_status;

typedef void (application_t)(void);
typedef struct
{
    uint32_t		stack_addr;     // Stack Pointer
    application_t*	func_p;        // Program Counter
} JumpStruct;

uint32_t app_validtion(void);
uint32_t request_validation(void);
uint32_t patch_validtion(void);
uint32_t app2_validtion(void);
void app_config(uint32_t data);
void request_config(uint32_t data);
void patch_config(uint32_t data);
void app2_config(uint32_t data);

uint8_t check_app_avaliable(uint32_t location);
BL_status BL_FeatchHostCommand();
void DO_PATCHING(void);
void RX_HANDLE(void);
void Flash_MainTask(void);
void BL_SendMessage(char *format,...);
void Req_Notification(uint8_t *req, uint8_t len);
void REQ_HANDLE(void);
#endif /* BOOTLOADER_H_ */
