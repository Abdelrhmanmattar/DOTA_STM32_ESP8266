/*
 * bootloader.c
 *
 *  Created on: May 8, 2024
 *      Author: abdelrhman mattar
 */
#include "stm32f1xx_hal.h"
#include "bootloader.h"
#include "rtc.h"
#include <stdarg.h>
#include "janpatch_mcu_config.h"
#include "FLASH_OPERATIONS.h"
#include "BackupRegister.h"
static uint8_t RX_LEN;
static uint8_t RxBuffer[150];

static uint8_t IS_REQUEST = False;
static uint8_t Req_len;
static uint8_t Req_ID;
static uint8_t DEV_ID;
static uint8_t *Req_Data;
static uint8_t UPDATE_TYPE;
static connect_state download_state=waiting_ProgrammingSession;

static uint8_t start = 0;

static uint8_t ack_var;
#define Response_positive() ack_var=Req_ID+0x40; BL_SendMessage("\033[0;32m %x %d %d \n\r \033[m	",Req_ID,download_state,__LINE__);HAL_UART_Transmit(&huart3,&ack_var,1, HAL_MAX_DELAY)

#define Response_negative() ack_var = 0x7f; BL_SendMessage("\033[0;31m %x %d %d \n\r \033[m",Req_ID,download_state,__LINE__);HAL_UART_Transmit(&huart3,&ack_var,1, HAL_MAX_DELAY)

#define print_line()           BL_SendMessage("LINE %d\n\r",__LINE__)

uint32_t JumpAddress;
pFunction Jump_To_Adress;
static uint32_t address_= 0x08008000,address_copy=0x08008000;

static void flash_jump_to_app(void);
static uint8_t FlashMemory_Paylaod_Write(uint16_t * pdata,uint32_t StartAddress,uint8_t Payloadlen)
{
	uint32_t Address=0;
	HAL_StatusTypeDef Hal_status=HAL_ERROR;
	uint8_t payload_status =0x00;
	HAL_FLASH_Unlock();

	for(uint8_t payload_count=0,UpdataAdress=0;payload_count<Payloadlen/2;payload_count++,UpdataAdress+=2)
	{
		Address =  StartAddress +UpdataAdress;
		Hal_status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD,Address,pdata[payload_count]);
		if(Hal_status != HAL_OK)
		{
			payload_status =0x00;
		}
		else{
			payload_status =0x01;
		}
	}
	HAL_FLASH_Lock();
	return payload_status;

}

uint32_t app_validtion(void)
{
	uint32_t ret = 0xff;
	BKPREG_init();
	ret = BKPREG_read(DR1);
	return ret;
}
uint32_t request_validation(void)
{
	uint32_t ret = 0xff;
	BKPREG_init();
	ret = BKPREG_read(DR2);
	return ret;
}
uint32_t patch_validtion(void)
{
	uint32_t ret = 0xff;
	BKPREG_init();
	ret = BKPREG_read(DR3);
	return ret;
}
uint32_t app2_validtion(void)
{
	uint32_t ret = 0xff;
	BKPREG_init();
	ret = BKPREG_read(DR4);
	return ret;
}

void app_config(uint32_t data)
{
	BKPREG_init();
	BKPREG_write(DR1, data);
}
void request_config(uint32_t data)
{
	BKPREG_init();
	BKPREG_write(DR2, data);
}

void patch_config(uint32_t data)
{
	BKPREG_init();
	BKPREG_write(DR3, data);
}
void app2_config(uint32_t data)
{
	BKPREG_init();
	BKPREG_write(DR4, data);
}


void REQ_HANDLE(void)
{
	Req_ID=SESSION_CONTROL;
	download_state=waiting_DownloadRequest;
	Response_positive();

}
uint32_t determind_path(uint8_t type__up)
{
	uint32_t address=SECOND_TARGET_ADDRESS;
	if(type__up==1)     //patch
	{
		address = PATCH_TARGET_ADDRESS;
	}
	else if(type__up==2)//full
	{
		address = SECOND_TARGET_ADDRESS;
	}
	return address;
}

void BL_SendMessage(char *format,...)
{
	char message[100]={0};
	va_list args;
	va_start(args,format);
	vsprintf(message,format,args);
	HAL_UART_Transmit(&huart1,(uint8_t*)message,sizeof(message),HAL_MAX_DELAY);
	va_end(args);

}

void RX_HANDLE(void)
{
	//BL_SendMessage("RX_HANDLE\n");
  static uint8_t index = 0;
  static uint8_t state = IDLE;
  //static uint8_t start = 0;
  // static unsigned short ad2dr_x=10;
  uint8_t x = 0;
  HAL_StatusTypeDef stat= HAL_ERROR;
  while (stat != HAL_OK)
  {
	 stat=HAL_UART_Receive(&huart3, &x, 1, HAL_MAX_DELAY);
	 //BL_SendMessage("LOOP\n\r");
  }
  if(start == 0 && x == 0x5a){start=1;return;}
  if(start == 1)
  {
	  if (state == IDLE && x<=130)
	  {
		  RX_LEN = x;
		  state = BUSY;
	  }
	  else
	  {
		  RxBuffer[index++] = x; //FIRST ID_DEVICE ID_CMD DATA
		  if (index == RX_LEN)
		  {
			  index = 0;
			  state = IDLE;
			  start = 0;
			  Req_Notification(RxBuffer, RX_LEN);
		  }
	  }

  }
}

void Req_Notification(uint8_t *req, uint8_t len)
{
  IS_REQUEST = True;
  DEV_ID = req[0];
  Req_ID = req[1];
  Req_len = len;
  Req_Data = &req[2];
  if(Req_ID == READ_FLASH_ADDRESS)
    {
  	  BL_SendMessage("RA:\n\r");
  	  for(int zz=2;zz<Req_len;zz++)
  	  {
  		  BL_SendMessage("%x ",Req_Data[zz-2]);
  	  }
  	  BL_SendMessage("\n\r");
    }
  //BL_SendMessage("get _  %d %d\n\r",DEV_ID,Req_ID);
  /*if(Req_ID == TRANSFER_DATA)
  {
	  BL_SendMessage("TD\n\r");
	  BL_SendMessage("data : %d  ",download_state);
	  for(int zz=2;zz<Req_len;zz++)
	  {
		  BL_SendMessage("%x ",Req_Data[zz-2]);
	  }
	  BL_SendMessage("\n\r");
  }*/
  //BL_SendMessage("line = %d\n\r",__LINE__);
  //BL_SendMessage("get _  %d %d %d %d\n\r",DEV_ID,Req_ID,Req_Data[0],RX_LEN);
}

void Flash_MainTask(void)
{
  static uint32_t code_size = 0, recevied_code = 0;
  uint8_t req_valid = False;
  if (IS_REQUEST == True && DEV_ID == my_ID_CHIP)
  {
	  //BL_SendMessage("line = %d\n\r",__LINE__);
    switch (Req_ID)
    {


    case START_APP:
    {
    	if(Req_len==2)
    	{
    		download_state=waiting_ProgrammingSession;
    		uint32_t app1;
    		app1=app_validtion();
    		if(app1!=0)
    		{
    			Response_positive();
    			flash_jump_to_app();
    		}
    		else
    		{
    			Response_negative();
    		}
    	}
    	else{Response_negative();}
    }break;
    case READ_FLASH_ADDRESS:
    {
    	download_state=waiting_ProgrammingSession;
    	if(Req_len == 6)
    	{
            uint32_t add_read = (uint32_t)Req_Data[0] | ((uint32_t)Req_Data[1]<<8) | ((uint32_t)Req_Data[2]<<16) | ((uint32_t)Req_Data[3]<<24);
            uint32_t ret_data=0xffffffff;
            if(BL_Address_Varification(add_read)==1)
            {
            	Flash_Read(add_read,&ret_data, 4);
            	Response_positive();
            	HAL_UART_Transmit(&huart3, &ret_data,4,HAL_MAX_DELAY);
            }
            else
            {
            	Response_negative();
            }
    	}
    	else
    	{
    		Response_negative();
    	}
    }break;
    case ERASE_FLASH:
    {
    	download_state=waiting_ProgrammingSession;
    	if(Req_len==2)
    	{
    		for(uint8_t i=0;i<TOT_PAGES;i++)
    		{
    			Flash_Erase(FIRST_TARGET_ADDRESS+(i*1024));
    		}
    		app_config(0);
    		patch_config(0);
    		app2_config(0);
    		request_config(0);
    		Response_positive();
    	}
    	else
    	{
    		Response_negative();
    	}

    }break;
    case ROLL_BACK:
    {
    	download_state=waiting_ProgrammingSession;
    	BL_SendMessage("app1 = %x , app2 = %x\n\r",app_validtion(),app2_validtion());
    	if(app_validtion()!=0 && app2_validtion()!=0)
    	{
    		swap();
    		uint32_t v1 = app_validtion()&0xffff;
    		uint32_t v2 = app2_validtion()&0xffff;
    		app_config(v2);
    		app2_config(v1);
    		Response_positive();
    		flash_jump_to_app();

    	}
    	else
    	{
    		Response_negative();
    	}

    }break;
    case GET_CHIP_ID:
    {
    	if(Req_len == 2)
    	{
    		download_state=waiting_ProgrammingSession;
    		uint16_t id__ = (uint16_t)(DBGMCU->IDCODE & 0x00000FFF);
    		BL_SendMessage("ID %d \n\r",id__);
    		Response_positive();
    		HAL_UART_Transmit(&huart3,(uint8_t*)&id__,2,HAL_MAX_DELAY);
    	}
    	else
    	{
    		download_state=waiting_ProgrammingSession;
    		Response_negative();
    	}
    }break;
    case SESSION_CONTROL:
    {
    	/*BL_SendMessage("SC_:");
    	for(int i =2 ;i<Req_len;i++)
    	{
    		BL_SendMessage("%x ",Req_Data[i]);
    	}
    	BL_SendMessage("\r\n");*/
      if (Req_Data[0] == PROGRAMMING_SESSION && Req_len == 3)
      {

        download_state=waiting_DownloadRequest;
        Response_positive();
      }
      else
      {

        download_state = waiting_ProgrammingSession;
        Response_negative();

      }
    }
    break;

    case DOWNLOAD_REQUEST:
    {
      if (download_state == waiting_DownloadRequest && Req_len == 7)
      {
        code_size = (uint32_t)Req_Data[0] | ((uint32_t)Req_Data[1]<<8) | ((uint32_t)Req_Data[2]<<16) | ((uint32_t)Req_Data[3]<<24);
        recevied_code=0;
        UPDATE_TYPE = Req_Data[4];
        BL_SendMessage("DR_ %d %d:\n\r",code_size,UPDATE_TYPE);
        if ( (code_size < MAX_CODE_SIZE && UPDATE_TYPE == 2 ) || (code_size < MAX_patch_SIZE && UPDATE_TYPE == 1) )
        {
            download_state = waiting_TransferData;
            req_valid = True;
            address_=determind_path(UPDATE_TYPE);
            address_copy=address_;
            BL_SendMessage("adde %x \n\r",address_copy);
            for(uint8_t pi=0 ; pi<33;pi++){Flash_Erase(SECOND_TARGET_ADDRESS+(pi*1024));}
            for(uint8_t pi=0 ; pi<20;pi++){Flash_Erase(PATCH_TARGET_ADDRESS+(pi*1024));}
            Response_positive();


        }
      }
      if (req_valid == False)
      {
    	  Response_negative();
        download_state = waiting_ProgrammingSession;

      }
    }
    break;
    case TRANSFER_DATA:
    {
      if(download_state == waiting_TransferData && Req_len==130)
      {
          //FlashMemory_Paylaod_Write( (uint16_t*)Req_Data,address_, 128);
    	  FlashMemory_Paylaod_Write( (uint16_t*)Req_Data,address_, 128);
          address_+=128;
          recevied_code+=128;
          if(recevied_code >= code_size)download_state = waiting_TransferExit;
          BL_SendMessage("TD %x %x\n\r",recevied_code,code_size);
          Response_positive();

      }
      else
      {
    	  recevied_code=0;code_size=0;
          download_state = waiting_ProgrammingSession;
          address_=FIRST_TARGET_ADDRESS;
    	  Response_negative();

      }
      
    }break;
	case TRANSFER_EXIT:
	{
		if (download_state==waiting_TransferExit && Req_len==2)
		{
			Response_positive();
			download_state=waiting_ProgrammingSession;
			if(UPDATE_TYPE==1){patch_config(code_size);BL_SendMessage("PATCHING\n\r");}
			else if(UPDATE_TYPE==2){app2_config(code_size);BL_SendMessage("APPING\n\r");}
		    DO_PATCHING();
			flash_jump_to_app();
		}
		else
		{
			download_state=waiting_ProgrammingSession;
			Response_negative();


		}
	}break;
	default:
	{
		download_state = waiting_ProgrammingSession;
		address_=SECOND_TARGET_ADDRESS;
		Response_negative();

	}break;




  }
  IS_REQUEST = False;
  }
}


static void flash_jump_to_app(void)
{
	SCB->VTOR=FIRST_TARGET_ADDRESS;
	uint32_t MSP_Value = *((volatile uint32_t*)FIRST_TARGET_ADDRESS);
	__set_MSP(MSP_Value);
	/* Reset Handler defination function of our main application */
	uint32_t MainAppAddr = *((volatile uint32_t*)(FIRST_TARGET_ADDRESS+4));

	/* Declare pointer to function contain the beginning address of reset function in user application */
	pFunction ResetHandler_Address = (pFunction)MainAppAddr;

	/* Deinitionalization of modules that used in bootloader and work
	   the configurations of new application */
/*	HAL_UART_DeInit(&huart1);
	HAL_UART_DeInit(&huart3);
*/
	/*HAL_RCC_DeInit();
	HAL_DeInit();
	SysTick->CTRL=0;
	SysTick->LOAD=0;
	SysTick->VAL=0;
	SCB->VTOR=FIRST_TARGET_ADDRESS;*/
	/* Reset main stack pointer */
	//__set_MSP(MSP_Value);

	/* Jump to Apllication Reset Handler */
	ResetHandler_Address();
}


uint8_t check_app_avaliable(uint32_t location)
{
	//Check if the application is there
	uint32_t emptyCellCount = 0,data;
	for(uint8_t i=0; i<10; i++)
	{
		data = 0xffffffff;
		Flash_Read(location+(i*4) , &data ,4);
		if(data == 0xffffffff)
			emptyCellCount++;
	}

	if(emptyCellCount != 10)
		return 1;
	else
		return 0;
}






void DO_PATCHING(void)
{
	BL_SendMessage("start_patch\n\r");
	if(UPDATE_TYPE==1)
	{
		print_line();
		if(app_validtion()!=0 && patch_validtion()!=0)
		{
			print_line();
			update_source(FIRST_TARGET_ADDRESS,app_validtion());
			update_patch(PATCH_TARGET_ADDRESS, patch_validtion());
			update_target(SECOND_TARGET_ADDRESS, MAX_APP_SIZE);
			BL_SendMessage("s --> %x \t p --> %x \t t--> %x\n\r",app_validtion(),patch_validtion(),app2_validtion());
			int res = start_janpatch();
			if(res==0)
			{
				swap();
				app_config(udpate_size());
				app2_config(source_size());
				BL_SendMessage("s --> %x \t p --> %x \t t--> %x\n\r",app_validtion(),patch_validtion(),app2_validtion());
				BL_SendMessage("start_swap %d\n\r",1);
			}

		}
		else
		{
			print_line();

		}
	}
	else if(UPDATE_TYPE==2)
	{
		print_line();
		BL_SendMessage("swap %x\n\r",swap());
		uint32_t app1=app_validtion();
		uint32_t app2=app2_validtion();
		BL_SendMessage("APP %x app2 %x\n\r",app1,app2);
		app_config(app2);
		app2_config(app1);
		BL_SendMessage("APP %x app2 %x\n\r",app_validtion(),app2_validtion());
		BL_SendMessage("end_swap %d\n\r",2);
	}
}
















