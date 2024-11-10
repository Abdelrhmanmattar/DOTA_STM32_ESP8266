/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stdio.h"
#include "stdarg.h"
#include "usart.h"
#include "gpio.h"
#include "BackupRegister.h"
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef void (*pFunction)(void);
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void BL_SendMessage(char *format,...);
volatile uint8_t x,RX_LEN;
volatile uint8_t program_request=0;
volatile uint8_t RX_ARRAY[255];
#define FIRST_TARGET_ADDRESS 0x8000000
/* USER CODE BEGIN PFP */
void request_config(uint32_t data)
{
	BKPREG_init();
	BKPREG_write(DR2, data);
}
void app_config(uint32_t data)
{
	BKPREG_init();
	BKPREG_write(DR1, data);
}
static void flash_jump_to_app(void)
{
	uint32_t MSP_Value = *((volatile uint32_t*)FIRST_TARGET_ADDRESS);
	/* Reset Handler defination function of our main application */
	uint32_t MainAppAddr = *((volatile uint32_t*)(FIRST_TARGET_ADDRESS+4));

	/* Declare pointer to function contain the beginning address of reset function in user application */
	pFunction ResetHandler_Address = (pFunction)MainAppAddr;

	/* Deinitionalization of modules that used in bootloader and work
	   the configurations of new application */
	HAL_UART_DeInit(&huart1);
	HAL_UART_DeInit(&huart3);

	HAL_RCC_DeInit(); /* Resets the RCC clock configuration to the default reset state. */
	HAL_DeInit();
	SysTick->CTRL=0;
	SysTick->LOAD=0;
	SysTick->VAL=0;
	SCB->VTOR=FIRST_TARGET_ADDRESS;
	/* Reset main stack pointer */
	__set_MSP(MSP_Value);

	/* Jump to Apllication Reset Handler */
	ResetHandler_Address();
}
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
	SCB->VTOR=0X08008000;
  HAL_Init();
  SystemClock_Config();
  MX_GPIO_Init();
  MX_USART1_UART_Init();
  MX_USART3_UART_Init();

  HAL_UART_Receive_IT(&huart3, &x, 1);
  uint32_t i=0;
  BL_SendMessage("THIS'S PATCH\n\r");
  while (1)
  {
		if (program_request)
		{
			program_request=0;
			BL_SendMessage("REQ_S\n\r");
			request_config(1);
			HAL_NVIC_SystemReset();
		}
    /* USER CODE END WHILE */
	  BL_SendMessage("THIS'S PATCH %d\n\r",i++);
	  HAL_Delay(1000);
    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
void BL_SendMessage(char *format,...)
{
	char message[100]={0};
	va_list args;
	va_start(args,format);
	vsprintf(message,format,args);
	HAL_UART_Transmit(&huart1,(uint8_t*)message,sizeof(message),HAL_MAX_DELAY);
	va_end(args);

}


void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	BL_SendMessage("ISR\n\r");
	if (huart->Instance == USART3)
	{
		x=(uint8_t)(huart->Instance->DR & 0x00FF);
		  static uint8_t index = 0;
		  static uint8_t state = 0;
		  static uint8_t start = 0;
		  if(start == 0 && x == 0x5a){start=1;HAL_UART_Receive_IT(&huart3, &x, 1);return;}
		  if(start==1)
		  {
			  if (state == 0)
			  {
				  RX_LEN = x;
				  state = 1;
			  }
			  else
			  {
				  RX_ARRAY[index++]=x;
					if (index==RX_LEN)
					{
						index=0;
						state=0;
						if(RX_LEN==3 && RX_ARRAY[0]==0X23 && RX_ARRAY[1]==0x10 && RX_ARRAY[2]==0x03)
						{
							program_request=1;
						}
					}

			  }
		  }
		  HAL_UART_Receive_IT(&huart3, &x, 1);
	}
}


/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  BL_SendMessage("Error: file %s, line %d\r\n", __FILE__,__LINE__);
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
