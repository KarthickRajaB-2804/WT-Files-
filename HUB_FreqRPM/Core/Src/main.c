/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
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
#include "spi.h"
#include "tim.h"
#include "gpio.h"
#include "DRV8323.h"
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
#define POLES 30

#define HALL_A HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0)
#define HALL_B HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_1)
#define HALL_C HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_2)

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
volatile uint32_t count2;
float rpm,freq = 0.0;
uint32_t last_time,motor_status = 0;
volatile int dir = 0;
volatile uint8_t current_State,previous_State = 0;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
int Motor_Direction(void)
{
	const int direction_sequence[8][8] =
	{
			{0,0,0,0,0,0,0,0},   //0
			{0,0,0,-1,0,+1,0,0}, //1 previous
			{0,0,0,+1,0,0,-1,0}, //2 previous
			{0,+1,-1,0,0,0,0,0}, //3 previous
			{0,0,0,0,0,-1,+1,0}, //4 previous
			{0,-1,0,0,+1,0,0,0}, //5 previous
			{0,0,+1,0,-1,0,0,0}, //6 previous
			{0,0,0,0,0,0,0,0}    //7 previous
	};
	current_State = (HALL_A<<0)|(HALL_B<<1)|(HALL_C<<2);
	if(current_State == 0 || previous_State == 7)
	{
		dir = 0;
		return 0;
	}
	dir = direction_sequence[previous_State][current_State];
    previous_State = current_State;   // update previous state
    return dir;
}

void All_Disable(void)
{
	TIM1->CCER &= ~(
	        (1<<0)  | (1<<2)  |   // U HS + LS
	        (1<<4)  | (1<<6)  |   // V HS + LS
	        (1<<8)  | (1<<10)     // W HS + LS
	    );
}

void BLDC_Commutate(uint8_t hall)
{
	// U Phase - Channel 1
	// V Phase - Channel 2
    // W Phase - Channel 3

    All_Disable();

	switch(hall)
	{
	  // DIR = 0

	  case 1: // 110    V+ W-  U-floating
          TIM1->CCER |= (1<<4);
          TIM1->CCER |= (1<<10);
		  break;

      case 2: // 100    U+ W-  V-floating
    	  TIM1->CCER |= (1<<0);
          TIM1->CCER |= (1<<10);
    	  break;

      case 3: // 101    U+ V-  W-floating
    	  TIM1->CCER |= (1<<0);
    	  TIM1->CCER |= (1<<6);
    	  break;

      case 4: // 001    W+ V-  U-floating
    	  TIM1->CCER |= (1<<8);
    	  TIM1->CCER |= (1<<6);
    	  break;

      case 5: // 011    W+ U-  V-floating
    	  TIM1->CCER |= (1<<8);
    	  TIM1->CCER |= (1<<2);
    	  break;

      case 6: // 010    V+ U-  W-floating
    	  TIM1->CCER |= (1<<4);
          TIM1->CCER |= (1<<2);
          break;

      default:
    	  break;
	}

}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_TIM2_Init();
  MX_SPI3_Init();
  MX_TIM1_Init();
  /* USER CODE BEGIN 2 */

  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_8, SET);   // DRV_ENABLE
  HAL_TIMEx_HallSensor_Start_IT(&htim2);
  HAL_TIM_Base_Start_IT(&htim2);
  DRV_INIT();
  //TIM1->BDTR = (1<<15);
  // PWM Generation
  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
  HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_1);
  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);
  HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_2);
  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_3);
  HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_3);

  BLDC_Commutate(2);
  TIM1->EGR |= (1<<5);  //COMG
  __HAL_TIM_MOE_ENABLE (&htim1);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
   while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

  /* USER CODE END 3 */
  }
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
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_TIM1;
  PeriphClkInit.Tim1ClockSelection = RCC_TIM1CLK_HCLK;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
// Calculate RPM of the Hub motor
// Counter resets to 0 after detecting edge
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
	if(htim->Instance == TIM2 && htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1)
	{
		HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_4);      //TP1
		count2 = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1);
		if(count2>0)
	    {
		    freq = (1000000/count2);
		    Motor_Direction();   // Read the direction of motor
			rpm=(dir*(120*freq)/(6*POLES)); // 6 Hall Transitions
			motor_status = 1;
		}
		else
		{
			freq = 0.0;
			rpm = 0.0;
		}
		//BLDC_Commutate(current_State);
	}

}
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	if(htim->Instance == TIM2)
	{
		//motor_status = 0;
		if(motor_status == 0)
		{
			rpm = 0.0;
			dir = 0;
		}
		motor_status = 0;
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
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
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
