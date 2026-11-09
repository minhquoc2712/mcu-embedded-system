/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body (Lab 4 - O(1) Cooperative Scheduler)
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2026 STMicroelectronics.
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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "scheduler.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define UART_TX_BUF_SIZE  1024U
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
TIM_HandleTypeDef htim2;

UART_HandleTypeDef huart1;

/* USER CODE BEGIN PV */
static uint8_t           s_uart_tx_buf[UART_TX_BUF_SIZE];
static volatile uint16_t s_uart_tx_head = 0U;
static volatile uint16_t s_uart_tx_tail = 0U;

static uint32_t g_d6_blink_task_id = NO_TASK_ID;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM2_Init(void);
static void MX_USART1_UART_Init(void);
/* USER CODE BEGIN PFP */
static void    UART_Log_PutChar(char c);
static void    UART_Log_Str(const char *s);
static void    UART_Log_UInt(uint32_t val, uint8_t min_width);
static void    UART_Log_Event(const char *msg);
static void    UART_Log_Pump(void);
static uint8_t UART_Log_IsBusy(void);

void Task_Blink_0_5s(void);
void Task_Blink_1_0s(void);
void Task_Blink_1_5s(void);
void Task_Blink_2_0s(void);
void Task_Blink_2_5s(void);
void Task_D6_FastBlink(void);
void Task_OneShot_700ms(void);
void Task_OneShot_1700ms(void);
void Task_OneShot_3100ms_DeleteD6(void);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
static void UART_Log_PutChar(char c)
{
  uint16_t next_head = (uint16_t)((s_uart_tx_head + 1U) % UART_TX_BUF_SIZE);
  if (next_head != s_uart_tx_tail)
  {
    s_uart_tx_buf[s_uart_tx_head] = (uint8_t)c;
    s_uart_tx_head = next_head;
  }
}

static void UART_Log_Str(const char *s)
{
  while ((s != 0) && (*s != '\0'))
  {
    UART_Log_PutChar(*s++);
  }
}

static void UART_Log_UInt(uint32_t val, uint8_t min_width)
{
  char digits[10];
  uint8_t count = 0U;

  do
  {
    digits[count++] = (char)('0' + (val % 10U));
    val /= 10U;
  } while (val > 0U);

  while (count < min_width)
  {
    UART_Log_PutChar(' ');
    min_width--;
  }

  while (count > 0U)
  {
    UART_Log_PutChar(digits[--count]);
  }
}

static void UART_Log_Event(const char *msg)
{
  UART_Log_PutChar('[');
  UART_Log_UInt(get_time(), 5U);
  UART_Log_Str(" ms] ");
  UART_Log_Str(msg);
  UART_Log_Str("\r\n");
}

static void UART_Log_Pump(void)
{
  while ((s_uart_tx_tail != s_uart_tx_head) &&
         (__HAL_UART_GET_FLAG(&huart1, UART_FLAG_TXE) != RESET))
  {
    huart1.Instance->DR = s_uart_tx_buf[s_uart_tx_tail];
    s_uart_tx_tail = (uint16_t)((s_uart_tx_tail + 1U) % UART_TX_BUF_SIZE);
  }
}

static uint8_t UART_Log_IsBusy(void)
{
  if (s_uart_tx_tail != s_uart_tx_head)
  {
    return 1U;
  }
  if (__HAL_UART_GET_FLAG(&huart1, UART_FLAG_TC) == RESET)
  {
    return 1U;
  }
  return 0U;
}

/* -------- 5 Periodic Tasks: 0.5s, 1.0s, 1.5s, 2.0s, 2.5s (Section 4) -------- */

void Task_Blink_0_5s(void)
{
  HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_1);
  UART_Log_Event("Periodic 0.5s -> Toggle D1 (PA1)");
}

void Task_Blink_1_0s(void)
{
  HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_2);
  UART_Log_Event("Periodic 1.0s -> Toggle D2 (PA2)");
}

void Task_Blink_1_5s(void)
{
  HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_3);
  UART_Log_Event("Periodic 1.5s -> Toggle D3 (PA3)");
}

void Task_Blink_2_0s(void)
{
  HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_4);
  UART_Log_Event("Periodic 2.0s -> Toggle D4 (PA4)");
}

void Task_Blink_2_5s(void)
{
  HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
  UART_Log_Event("Periodic 2.5s -> Toggle D5 (PA5)");
}

/* -------- One-Shot & SCH_Delete_Task Demonstration (Section 5) -------- */

void Task_D6_FastBlink(void)
{
  HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_6);
}

void Task_OneShot_700ms(void)
{
  UART_Log_Event("[ONE-SHOT #1] 700ms delay expired!");
}

void Task_OneShot_1700ms(void)
{
  UART_Log_Event("[ONE-SHOT #2] 1700ms delay expired!");
}

void Task_OneShot_3100ms_DeleteD6(void)
{
  if (SCH_Delete_Task(g_d6_blink_task_id) == RETURN_NORMAL)
  {
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_SET); /* Turn D6 OFF (Active-LOW) */
    UART_Log_Event("[ONE-SHOT #3] 3100ms -> SCH_Delete_Task(D6) OK!");
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
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */
  /* Turn OFF all Active-LOW LEDs D1..D6 (PA1..PA6 = HIGH) at startup */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4
                          |GPIO_PIN_5|GPIO_PIN_6, GPIO_PIN_SET);

  /* Initialize O(1) Delta-List Cooperative Scheduler */
  SCH_Init();
  SCH_Set_Sleep_Guard(UART_Log_IsBusy);

  UART_Log_Str("=== HCMUT Lab 4: O(1) Cooperative Scheduler ===\r\n");
  UART_Log_Event("Scheduler initialized (Tick = 10ms)");

  /* 5 Required Periodic Tasks: 0.5s, 1.0s, 1.5s, 2.0s, 2.5s */
  SCH_Add_Task(Task_Blink_0_5s,  500U,  500U);
  SCH_Add_Task(Task_Blink_1_0s, 1000U, 1000U);
  SCH_Add_Task(Task_Blink_1_5s, 1500U, 1500U);
  SCH_Add_Task(Task_Blink_2_0s, 2000U, 2000U);
  SCH_Add_Task(Task_Blink_2_5s, 2500U, 2500U);

  /* Fast-blink D6 (200ms) to visually demonstrate SCH_Delete_Task(taskID) */
  g_d6_blink_task_id = SCH_Add_Task(Task_D6_FastBlink, 200U, 200U);

  /*
   * Out-of-order One-Shot tasks (PERIOD = 0) registered before main loop:
   * Added as 3100ms -> 700ms -> 1700ms, delivered in order 700ms -> 1700ms -> 3100ms!
   */
  SCH_Add_Task(Task_OneShot_3100ms_DeleteD6, 3100U, 0U);
  SCH_Add_Task(Task_OneShot_700ms,            700U, 0U);
  SCH_Add_Task(Task_OneShot_1700ms,          1700U, 0U);

  /* Start 10ms hardware timer interrupt */
  HAL_TIM_Base_Start_IT(&htim2);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    SCH_Dispatch_Tasks();
    UART_Log_Pump();
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

/**
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 7999;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 9;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 9600;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOA_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4
                          |GPIO_PIN_5|GPIO_PIN_6, GPIO_PIN_RESET);

  /*Configure GPIO pins : PA1 PA2 PA3 PA4
                           PA5 PA6 */
  GPIO_InitStruct.Pin = GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4
                          |GPIO_PIN_5|GPIO_PIN_6;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  if (htim->Instance == TIM2)
  {
    SCH_Update();
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
