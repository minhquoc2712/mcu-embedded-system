/* USER CODE BEGIN Header */
/**
  * @file           : main.c
  * @brief          : Exercise 3 - update7SEG
  */
/* USER CODE END Header */
#include "main.h"

/* USER CODE BEGIN Includes */
/* USER CODE END Includes */

/* USER CODE BEGIN PTD */
/* USER CODE END PTD */

/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* USER CODE BEGIN PM */
/* USER CODE END PM */

TIM_HandleTypeDef htim2;

/* USER CODE BEGIN PV */
const int MAX_LED = 4;
int index_led = 0;
int led_buffer[4] = {1, 2, 3, 4};

int counter = 0;        // dem so lan ngat (10ms) cho 7SEG
int led_state = 0;      // trang thai DOT
int led_counter = 0;    // dem so lan ngat cho DOT
/* USER CODE END PV */

void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM2_Init(void);
/* USER CODE BEGIN PFP */
void display7SEG(int num);
void update7SEG(int index);
/* USER CODE END PFP */

/* USER CODE BEGIN 0 */
/* USER CODE END 0 */

int main(void)
{
  /* USER CODE BEGIN 1 */
  /* USER CODE END 1 */

  HAL_Init();

  /* USER CODE BEGIN Init */
  /* USER CODE END Init */

  SystemClock_Config();

  /* USER CODE BEGIN SysInit */
  /* USER CODE END SysInit */

  MX_GPIO_Init();
  MX_TIM2_Init();
  /* USER CODE BEGIN 2 */
  // dam bao trang thai ban dau: tat het (PNP va common anode: High = tat)
  HAL_GPIO_WritePin(GPIOA, DOT_Pin|LED_RED_Pin|EN0_Pin|EN1_Pin|EN2_Pin|EN3_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(GPIOB, SEG_A_Pin|SEG_B_Pin|SEG_C_Pin|SEG_D_Pin
                          |SEG_E_Pin|SEG_F_Pin|SEG_G_Pin, GPIO_PIN_SET);
  HAL_TIM_Base_Start_IT(&htim2);
  /* USER CODE END 2 */

  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    /* USER CODE END 3 */
  }
}

void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
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

static void MX_TIM2_Init(void)
{
  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

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
}

static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  HAL_GPIO_WritePin(GPIOA, DOT_Pin|LED_RED_Pin|EN0_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(GPIOA, EN1_Pin|EN2_Pin|EN3_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(GPIOB, SEG_A_Pin|SEG_B_Pin|SEG_C_Pin|SEG_D_Pin
                          |SEG_E_Pin|SEG_F_Pin|SEG_G_Pin, GPIO_PIN_RESET);

  GPIO_InitStruct.Pin = DOT_Pin|LED_RED_Pin|EN0_Pin|EN1_Pin
                          |EN2_Pin|EN3_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = SEG_A_Pin|SEG_B_Pin|SEG_C_Pin|SEG_D_Pin
                          |SEG_E_Pin|SEG_F_Pin|SEG_G_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}

/* USER CODE BEGIN 4 */

// display7SEG tu Lab 1 (common anode: RESET = sang)
void display7SEG(int num)
{
    if (num == 0) {
        HAL_GPIO_WritePin(GPIOB, SEG_A_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOB, SEG_B_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOB, SEG_C_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOB, SEG_D_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOB, SEG_E_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOB, SEG_F_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOB, SEG_G_Pin, GPIO_PIN_SET);
    }
    else if (num == 1) {
        HAL_GPIO_WritePin(GPIOB, SEG_A_Pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(GPIOB, SEG_B_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOB, SEG_C_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOB, SEG_D_Pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(GPIOB, SEG_E_Pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(GPIOB, SEG_F_Pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(GPIOB, SEG_G_Pin, GPIO_PIN_SET);
    }
    else if (num == 2) {
        HAL_GPIO_WritePin(GPIOB, SEG_A_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOB, SEG_B_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOB, SEG_C_Pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(GPIOB, SEG_D_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOB, SEG_E_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOB, SEG_F_Pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(GPIOB, SEG_G_Pin, GPIO_PIN_RESET);
    }
    else if (num == 3) {
        HAL_GPIO_WritePin(GPIOB, SEG_A_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOB, SEG_B_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOB, SEG_C_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOB, SEG_D_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOB, SEG_E_Pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(GPIOB, SEG_F_Pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(GPIOB, SEG_G_Pin, GPIO_PIN_RESET);
    }
    else if (num == 4) {
        HAL_GPIO_WritePin(GPIOB, SEG_A_Pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(GPIOB, SEG_B_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOB, SEG_C_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOB, SEG_D_Pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(GPIOB, SEG_E_Pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(GPIOB, SEG_F_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOB, SEG_G_Pin, GPIO_PIN_RESET);
    }
    else if (num == 5) {
        HAL_GPIO_WritePin(GPIOB, SEG_A_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOB, SEG_B_Pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(GPIOB, SEG_C_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOB, SEG_D_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOB, SEG_E_Pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(GPIOB, SEG_F_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOB, SEG_G_Pin, GPIO_PIN_RESET);
    }
    else if (num == 6) {
        HAL_GPIO_WritePin(GPIOB, SEG_A_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOB, SEG_B_Pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(GPIOB, SEG_C_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOB, SEG_D_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOB, SEG_E_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOB, SEG_F_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOB, SEG_G_Pin, GPIO_PIN_RESET);
    }
    else if (num == 7) {
        HAL_GPIO_WritePin(GPIOB, SEG_A_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOB, SEG_B_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOB, SEG_C_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOB, SEG_D_Pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(GPIOB, SEG_E_Pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(GPIOB, SEG_F_Pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(GPIOB, SEG_G_Pin, GPIO_PIN_SET);
    }
    else if (num == 8) {
        HAL_GPIO_WritePin(GPIOB, SEG_A_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOB, SEG_B_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOB, SEG_C_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOB, SEG_D_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOB, SEG_E_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOB, SEG_F_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOB, SEG_G_Pin, GPIO_PIN_RESET);
    }
    else if (num == 9) {
        HAL_GPIO_WritePin(GPIOB, SEG_A_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOB, SEG_B_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOB, SEG_C_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOB, SEG_D_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOB, SEG_E_Pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(GPIOB, SEG_F_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOB, SEG_G_Pin, GPIO_PIN_RESET);
    }
    else {
        HAL_GPIO_WritePin(GPIOB, SEG_A_Pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(GPIOB, SEG_B_Pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(GPIOB, SEG_C_Pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(GPIOB, SEG_D_Pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(GPIOB, SEG_E_Pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(GPIOB, SEG_F_Pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(GPIOB, SEG_G_Pin, GPIO_PIN_SET);
    }
}

// Report 1: update7SEG
void update7SEG(int index)
{
    // tat het 4 LED roi bat LED can hien
    HAL_GPIO_WritePin(GPIOA, EN0_Pin|EN1_Pin|EN2_Pin|EN3_Pin, GPIO_PIN_SET);
    switch (index) {
        case 0:   // 7SEG thu nhat hien led_buffer[0]
            display7SEG(led_buffer[0]);
            HAL_GPIO_WritePin(EN0_GPIO_Port, EN0_Pin, GPIO_PIN_RESET);
            break;
        case 1:   // 7SEG thu hai hien led_buffer[1]
            display7SEG(led_buffer[1]);
            HAL_GPIO_WritePin(EN1_GPIO_Port, EN1_Pin, GPIO_PIN_RESET);
            break;
        case 2:   // 7SEG thu ba hien led_buffer[2]
            display7SEG(led_buffer[2]);
            HAL_GPIO_WritePin(EN2_GPIO_Port, EN2_Pin, GPIO_PIN_RESET);
            break;
        case 3:   // 7SEG thu tu hien led_buffer[3]
            display7SEG(led_buffer[3]);
            HAL_GPIO_WritePin(EN3_GPIO_Port, EN3_Pin, GPIO_PIN_RESET);
            break;
        default:
            break;
    }
}

// Report 2: HAL_TIM_PeriodElapsedCallback
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance != TIM2) return;

    // ---- 4 LED 7 doan: moi LED 500ms (50 x 10ms) ----
    counter++;
    if (counter >= 50) {
        counter = 0;
        update7SEG(index_led++);
        if (index_led >= MAX_LED) index_led = 0;
    }

    // ---- DOT (PA4): doi trang thai moi 1s (100 x 10ms) ----
    led_counter++;
    if (led_counter >= 100) {
        led_counter = 0;
        HAL_GPIO_TogglePin(DOT_GPIO_Port, DOT_Pin);
    }
}
/* USER CODE END 4 */

void Error_Handler(void)
{
  __disable_irq();
  while (1)
  {
  }
}

#ifdef  USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line)
{
}
#endif
