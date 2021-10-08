/**
 * Jaqueline Maurer Machado		Turma 4422		Nº 11
 * Exercício 3 da Lista de DMA
 */

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
#include "adc.h"
#include "dma.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"
#include <stdio.h>
#include <string.h>
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
///variavel que confere o primeiro valor medido do periodo
int64_t initVal = -1;
///variavel que pega o final da medição do periodo
int64_t valI = 0;
///variavel que controla o numero de estouros do timer dentro de uma medição
int64_t valE = 0;
///volatile significa que o DMA nao consegue escrever se nao for especificado que eh volatile
///volatile → alguma coisa de fora ira mudar o valor dessa variavel (DMA, ADC...)
///usar volatil se o valor da variavel for alterado, se só for lido nao precisa
volatile uint64_t value;
///flag para o timer 2
uint8_t flag = 0;
///flag para o timer 10, que exibe a mensagem da serial
uint8_t flagT = 0;
///flag que controla quando acontece a interrupção externa pelo PC13
uint8_t flagP = 0;
///variavel para calcular o periodo
uint64_t periodo = 0;
///variavel que pega o meio do periodo → borda de subida ou descida, dependendo o caso
uint64_t periodoI = 0;
///realiza 4 medições para depois fazer a media
volatile uint16_t medidas[4];
///variáveis para calcular as médias
uint16_t media1;
uint16_t media2;
///string usada para imprimir a mensagem na serial
char msg[50];


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
  MX_ADC1_Init();
  MX_TIM8_Init();
  /* USER CODE BEGIN 2 */
  HAL_TIM_Base_Start(&htim10); ///habilita o timer 10

  HAL_TIM_Base_Start_IT(&htim11); ///habilita o timer 11

  ///foi usado o timer 2 pois ele possui um valor maior que pode ser usado no ARR para dar o valor do periodo direto em micro segundos
  ///E colocar ambas as bordas na corfiguracao para fazer o que o exercicio pede
  HAL_TIM_IC_Start_DMA(&htim2, TIM_CHANNEL_1, &value, 1); ///Habilita o Input Capture com DMA do Timer2 → PA15 → canal 1

  HAL_TIM_PWM_Start_IT(&htim3, TIM_CHANNEL_1); ///PWM usado para gerar um sinal que sera usado para medir se o programa esta funcioando → marcar a opçao para ele começar desligado
  ///sempre desliga no estouro do timer → colocando a frequencia do timer vai ser a frequencia do sinal
  ///PWM → modulação de largura de pulso

  HAL_TIM_Base_Start_IT(&htim8); ///habilita uma frequencia aleatoria de PSC = 999 e ARR = 139
  ///Trigger Output → ativar o Update Event

  ///no IOC, o ADC esta configurado para ser ativado pelo timer, para continuar a sequencia de 4 medidas, que vao ser salvas dentro de um vetor de quatro espaços
  HAL_ADC_Start_DMA(&hadc1, medidas, 4);///usando dois canais do ADC, adiciona o DMA no ADC1, mudar a prioridade dele (que vem em Low) → tem 4 medidas de ADC
  ///parametros → DMA continuous request colocar em enable e External Trigger Conversion Source → timer 8 Trigger out event (pq o 3 ja estava em uso)
  ///numero de conversoes = 2 pois esta usando duas entradas (que abre dois ranks, colocar um em 0 e outro em 1), detecção de borda de subida
 ///mudar a configuração de botao do PC13 → GPIO_EXIT13 → e habilitar em System Core → GPIO, marcar PC13 e external interrupt mode with falling tra la la em GPIO mode, pois cada vez que precionado sera considerado como uma interrupção externa
  ///GPIO → NVIC → ENABLE

  ///push botton → juntar com o de cima na proto
  ///PA6 gera o sinal do PWM → ligar com os dois de cima
  ///com os três ligados, ligar um led para o terra para ver piscando
  ///duas entradas do ADC, colocar em dois potenciomentos (no meio), um deles muda a frequencia do led e o outro quanto % do tempo o led fica ligado e desligado
  ///como ligar o potenciometro → o pino do ADC ligado no meio, uma ponta no VCC e a outra no terra
  ///se colocar o periodo no maximo, ele fica quase 1 segundo ligado, se ficar no minimo, ele fica quase o tempo todo desligado

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  { ///imprime na serial

	  if(__HAL_TIM_GET_FLAG(&htim10, TIM_FLAG_UPDATE)){ ///verifica se passou 1 segundo do Timer10 para imprimir a mensagem na tela (a mensagem é mostrada de 1 em 1 segundo para não ficar muito poluído)
		  ///eh mostrado uma das três mensagens a cada segundo
		if(flagT==0 && periodo!=0){ ///flagT começa em zero, entao será o primeiro valor a ser exibido
			///periodo total do sinal
			sprintf(msg, "Periodo do Sinal: %4luus\n\r",periodo); ///imprime o valor do periodo total na serial
			flagT=1; ///altera o valor de flagT para 1 para entrar no proximo if e exibir a proxima mensagem
			HAL_UART_Transmit_IT(&huart2, msg, strlen(msg)); ///Usando a usart2, trasmite a mensagem
		}
		else if (flagT==1){ ///se ja imprimiu a mensagem de cima, entra nessa if
			///periodo de trabalho = periodo que o sinal fica em nivel logico 1, ou seja, que ele definitivamente "trabalha" → usando a variavel periodoI, que pega o "meio do sinal"
			sprintf(msg, "Periodo de Trabalho do Sinal: %4luus\n\r",flagP==0? periodoI : periodo - periodoI); ///faz o cálculo da frequência 1 dividindo o CLOCK do ARM pela multiplicação do período medido + 1 e o valor de PSC (que nesse caso é 47)
			flagT=2; ///altera o valor de flagT para 2 para entrar no proximo if e exibir a proxima mensagem
			HAL_UART_Transmit_IT(&huart2, msg, strlen(msg)); ///Usando a usart2, trasmite a mensagem
		}
		else if(flagT==2){ ///se ja imprimiu a mensagem de cima, entra nessa if
			///calcula a porcentagem que o led fica ligado → tambem usando a variavel de periodoI
			sprintf(msg, "Representa %i%% do Sinal\n\r",flagP==0? (periodoI*100/periodo) : ((periodo - periodoI)*100/periodo));
			flagT=0; ///altera o valor de flagT de volta para zero para imprimir a primeira mensagem novamente
			periodo=0; ///zera o periodo para que ele possa ser calculado novamente
			HAL_UART_Transmit_IT(&huart2, msg, strlen(msg)); ///Usando a usart2, trasmite a mensagem
		}
		__HAL_TIM_CLEAR_FLAG(&htim10, TIM_FLAG_UPDATE);
	}

    /* USER CODE END WHILE */

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
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin){ ///pino configurado para ser uma interrupção externa → nesse caso eh o PC13
	flagP=flag; ///toda vez q o de baixo der uma borda de descida, salva o valor
	///borda de descida = terminou o periodo ativo do sinal
	///ligar OUT e IN junto na proto (conferir os pinos) e depois liga em um led
}
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim){
	uint64_t calc(long a, long b, long c, long ARR); ///Declarando a função calc

	///o periodo de trabalho eh o tempo do sinal em high
	if(htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1){ ///interrupção do canal 1 do timer 2
		///eh preciso medir a borda de subida e de descida
		if (initVal == -1) { ///se initVal estiver com seu valor inicial, que é -1
			initVal = value; ///com o estouro, salva o valor que estava no contador
		} else { ///se initVal for diferente de -1, ou seja, se já mediu alguma coisa
			if(flag == 0){ ///se ainda esta calculando o periodo "do meio" → periodoI (usado para calcular o periodo de trabalho)
				periodoI = calc(initVal, value, valE, 999999); ///calcula o periodo utilizando a função criada calc
				flag = 1; ///muda o valor de flag para ser diferente de zero e entrar no else abaixo
			}else { ///se ja esta calculando o periodo total → periodo
				///calcula de novo, tudo dessa vez, porque agora esta no final, sabendo quantos estouros do timer teve
				periodo = calc(initVal, value, valE, 999999); ///calcula o periodo utilizando a função criada calc
				initVal = value; ///initVal volta ao seu valor
				valE = 0; ///zera valE
				flag = 0; ///zera a flag
			}
		}
	}
	HAL_TIM_IC_Start_DMA(&htim2, TIM_CHANNEL_1, &value, 1); ///Habilita o Input Capture com Interrupção do Timer2 → PA0 → canal 1
}

/**
 * @brief Função que faz o cálculo do periodo desejado
 * @brief Eh exatamente a mesma função usada no exercicio anterior, que era de IC e tambem era necessario medir o periodo do sinal
 */
uint64_t calc(long a, long b, long c, long ARR){ ///entra-se com 4 valores para fazer o cálculo, que vão ser os valores de initVal, valI, valE e ARR_CL
	uint64_t period; ///variavel que retorna o periodo calculado na função
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
	return period + 1; /// a função retorna o valor do periodo calculado
	///ele poderia entregar 0us, mas na real eh 1, entao soma +1
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim){ ///Interrupção do final do timer 11, ou seja, estouro do timer
	if (htim->Instance == TIM11) ///se a interrupção do timer 11 estourou
		///se initVal for maior que -1, ou seja, se ja passou no minimo um estouro do timer
			if(initVal > -1) valE++; ///conta quantas vezes o timer estourou adicionando na variável valE
}
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc){
	uint16_t a=__HAL_TIM_GET_AUTORELOAD(&htim3); ///pega o valor total de ARR
	uint16_t p=__HAL_TIM_GET_COMPARE(&htim3,TIM_CHANNEL_1); ///valor atual do COMPARE do mesmo timer
	uint16_t pul;
	uint16_t arr;
	media2=medidas[1];
	media2+=medidas[3];
	media2/=2;
	///faz a media tambem
	///o PWM esta em 1Hz → PSC = 1999 e ARR = 41999 → gera o periodo do sinal → MAX
	///uma outra frequencia → 7Hz → PSC = 1999 e ARR = 5999 → MIN
	///Valores de constantes no IOC
	///O led pisca em uma frequencia que pode variar de 1Hz ate 7Hz → esse valor foi pre-determinado, mas pode ser alterado de acordo com o desejado
	///foi escolhido uma frequencia mais baixa para que seja possivel visualizar melhor a mudança da piscada do led
	arr= media2*(ARRT3_MAX-ARRT3_MIN)/4095+ARRT3_MIN; ///faz uma porcentagem → o maximo da medida eh 4095
	///media2 x variação → divide por 4095 e vai dar um valor entre 0 e 36000 e depois soma 6000 de volta na conta para desse jeito ter o novo valor de ARR
	if(arr<a*0.9 || arr>a*1.1)__HAL_TIM_SET_AUTORELOAD(&htim3, arr); ///se o ARR variar +10% ou -10%, seta ele para o valor novo
	media1=medidas[0];
	media1+=medidas[2];
	media1/=2;
	///faz duas medidas e depois faz a media das duas
	pul=media1*arr/4095; ///mesma coisa que o anterior para esse aqui, mas como eh o minimo nao precisa fazer a soma
	if(pul<p*0.9 || pul>p*1.1)__HAL_TIM_SET_COMPARE(&htim3,TIM_CHANNEL_1, pul); ///mesma coisa do anterior → confere se esta +10% ou -10% e seta ele para o novo valor

	HAL_ADC_Start_DMA(&hadc1, medidas, 4); ///manda rodar novamente
	///quando for usar DMA, tem que ativar de novo, todas as vezes
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
