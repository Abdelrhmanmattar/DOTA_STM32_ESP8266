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
#include "crc.h"
#include "rtc.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "bootloader.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

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
/* USER CODE BEGIN PFP */

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
	  SCB->VTOR=0X8000000;
	  HAL_Init();
	  SystemClock_Config();
	  MX_CRC_Init();
	  MX_RTC_Init();
	  MX_USART3_UART_Init();
	  MX_USART1_UART_Init();

	  /* USER CODE BEGIN 2 */
	  /* USER CODE END 2 */

	  BL_SendMessage("START\n\r");
	  /* USER CODE BEGIN 2 */
	  /* USER CODE END 2 */
	  uint16_t valid_app,valid_req;
      /*app_config(0);
      app2_config(0);
      patch_config(0);
      request_config(0);*/
	  valid_app = app_validtion();
	  valid_req = request_validation();
	  BL_SendMessage("START %d %d\n\r",valid_app , valid_req);

		if ( (valid_app==0) || (valid_req!=0) )
		{
			if(valid_req!=0)
			{

				request_config(0);
				BL_SendMessage("VALID_REQ\n\r");
				REQ_HANDLE();
			}
			while(1)
			{
				RX_HANDLE();
				Flash_MainTask();
			}
		}
		else {BL_SendMessage("else\n\r");flash_jump_to_app();}
		//BL_SendMessage("5555555\n\r");
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_LSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler(__FILE__,__LINE__);
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
    Error_Handler(__FILE__,__LINE__);
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_RTC;
  PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_LSI;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler(__FILE__,__LINE__);
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(const char *file, int line)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  BL_SendMessage("Error: file %s, line %d\r\n", file, line);
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
