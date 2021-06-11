/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "dma.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

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

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
int64_t initVal = -1;
int64_t value;
int64_t valE = 0;
uint8_t flag = 0;
uint64_t periodo = 0;
uint64_t periodoI = 0;
char msg[100];
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

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
  MX_DMA_Init();
  MX_USART2_UART_Init();
  MX_TIM2_Init();
  MX_TIM3_Init();
  MX_TIM10_Init();
  MX_TIM11_Init();
  /* USER CODE BEGIN 2 */
  HAL_TIM_Base_Start(&htim11); ///Habilita o timer 11
  HAL_TIM_Base_Start_IT(&htim10); ///Habilita o timer 10
  HAL_TIM_Base_Start_DMA(&htim2, &value, 1); ///PA0
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1); ///Habilita o PWM do Timer 3 → PA6
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */
	  if(__HAL_TIM_GET_FLAG(&htim10, TIM_FLAG_UPDATE)){ ///verifica se passou 1 segundo do Timer10 para imprimir a mensagem na tela (a mensagem é mostrada de 1 em 1 segundo para não ficar muito poluído)
		sprintf(msg, "Periodo medido: %uus\n\r",(uint64_t)(periodo)); ///Exibe o valor da variavel periodo, que eh o valor solicitado
		HAL_UART_Transmit_IT(&huart2, msg, strlen(msg)); ///Usando a usart2, trasmite a mensagem
		periodo = 0; ///iguala o periodo a zero para a próxima medição
	  }
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

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);
  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 16;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
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
}

/* USER CODE BEGIN 4 */
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim){
	uint64_t calc(long a, long b, long c, long ARR);

	if(htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1){ ///interrupção do canal 1 do timer 3
		if (initVal == -1) { ///se initVal estiver com seu valor inicial, que é -1
			value = __HAL_TIM_GET_COMPARE(htim, TIM_CHANNEL_1); ///com o estouro, salva o valor que estava no contador
		} else { ///se initVal for diferente de -1, ou seja, já mediu alguma coisa
			if(flag == 0){
				periodoI = calc(initVal, value, valE, 999999); ///calcula o periodo utilizando a função criada calc
				flag = 1;
			}else {
				periodo = calc(initVal, value, valE, 999999); ///calcula o periodo utilizando a função criada calc
				initVal = value; ///initVal volta ao seu valor
				valE = 0; ///valE volta ao seu valor inicial
				flag = 0;
			}
		}
	}
	HAL_TIM_Base_Start_DMA(&htim2, &value, 1);
}

/**
 * @brief Função que faz o cálculo do periodo desejado
 */
uint64_t calc(long a, long b, long c, long ARR){ ///entra-se com 4 valores para fazer o cálculo, que vão ser os valores de initVal, valI, valE e ARR_CL
	uint64_t period;
	if(c > 0) ///se o número de períodos medidos durante a medição for 1 ou mais
	{
		if(c > 1) ///se o número de períodos medidos durante a medição for mais que 1
		{
			c--;    ///ignorando o final do primeiro periodo
			period = (ARR - a + b) + (ARR) * c; /// se entre o valor inicial e o valor final (Internal) houverem periodos completos do timer3
			///ou seja, se tiver mais que um período, utiliza o mesmo cálculo só acrescenta a parte de multiplicar o valor do ARR pelo número de períodos a mais que teve
		}
		///se o número de períodos medidos durante a medição for 1
		else period = (ARR - a + b); /// se o valor inicial e o valor final (Internal) estiverem em periodos subsequentes do timer3
		///ou seja, se houver passado somente um período
	}
	///se o número de períodos medidos durante a medição não completar 1 inteiro
	else period = b - a; /// se ainda estiver dentro do mesmo periodo do timer3, ou seja, não tiver passado um período completo
	return period; /// a função retorna o valor do periodo calculado
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim){ ///Interrupção do final do timer 10, ou seja, estouro do timer
	if (htim->Instance == TIM11) ///se a interrupção do timer 10 estourou
			if(initVal > -1) valE++; ///conta quantas vezes o timer estourou adicionando na variável valE
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

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
