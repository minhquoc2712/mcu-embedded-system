/* USER CODE BEGIN Header */
/**
  * @file    main.c
  * @brief   Exercise 10 - 8x8 LED Matrix animation (character "A" moves left/right)
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
// ---- Clock ----
int hour = 15, minute = 8, second = 50;

// ---- 4 seven-segment LEDs ----
const int MAX_LED = 4;
int index_led = 0;
int led_buffer[4] = {1, 2, 3, 4};

// ---- LED matrix: each byte = one ROW, bit n = column n ----
const int MAX_LED_MATRIX = 8;
int index_led_matrix = 0;

// Original character "A" (never modified, used as the source of the animation)
const uint8_t A_pattern[8] = {0x18, 0x24, 0x24, 0x24, 0x3C, 0x24, 0x24, 0x24};

// Buffer actually displayed on the matrix
uint8_t matrix_buffer[8] = {0x18, 0x24, 0x24, 0x24, 0x3C, 0x24, 0x24, 0x24};

// ---- Animation ----
#define ANIMATION_PERIOD 300        // ms between two shifts
#define MAX_SHIFT        2          // "A" is 4 columns wide -> can move 2 columns each side
#define START_DIR       -1          // -1: start moving toward column 0, +1: the opposite
int shift_pos = 0;                  // 0 = center, <0 toward column 0, >0 toward column 7
int shift_dir = START_DIR;

GPIO_TypeDef* enm_port[8] = {ENM0_GPIO_Port, ENM1_GPIO_Port, ENM2_GPIO_Port, ENM3_GPIO_Port,
                             ENM4_GPIO_Port, ENM5_GPIO_Port, ENM6_GPIO_Port, ENM7_GPIO_Port};
uint16_t enm_pin[8] = {ENM0_Pin, ENM1_Pin, ENM2_Pin, ENM3_Pin,
                       ENM4_Pin, ENM5_Pin, ENM6_Pin, ENM7_Pin};

// ---- Software timers (TIM2: Prescaler 7999, Period 9 -> 10ms) ----
int TIMER_CYCLE = 10;
volatile int timer0_counter = 0, timer0_flag = 0;   // 1s (clock + DOT)
volatile int timer1_counter = 0, timer1_flag = 0;   // 7SEG scan
volatile int timer2_counter = 0, timer2_flag = 0;   // matrix scan
volatile int timer3_counter = 0, timer3_flag = 0;   // animation
/* USER CODE END PV */

void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM2_Init(void);
/* USER CODE BEGIN PFP */
void display7SEG(int num);
void update7SEG(int index);
void updateClockBuffer(void);
void updateLEDMatrix(int index);
void updateAnimation(void);
void setTimer0(int duration);
void setTimer1(int duration);
void setTimer2(int duration);
void setTimer3(int duration);
void timer_run(void);
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
  // Matrix idle: all rows High (off), all columns High (ENM High -> dark)
  GPIOB->ODR |= 0xFF00;
  HAL_GPIO_WritePin(GPIOA, ENM0_Pin|ENM1_Pin|ENM2_Pin|ENM3_Pin
                          |ENM4_Pin|ENM5_Pin|ENM6_Pin|ENM7_Pin, GPIO_PIN_SET);

  HAL_TIM_Base_Start_IT(&htim2);
  updateClockBuffer();
  setTimer0(1000);
  setTimer1(500);
  setTimer2(10);
  setTimer3(ANIMATION_PERIOD);
  /* USER CODE END 2 */

  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    if (timer0_flag == 1) {
        setTimer0(1000);
        second++;
        if (second >= 60) { second = 0; minute++; }
        if (minute >= 60) { minute = 0; hour++; }
        if (hour >= 24)   { hour = 0; }
        updateClockBuffer();
        HAL_GPIO_TogglePin(DOT_GPIO_Port, DOT_Pin);
    }

    if (timer1_flag == 1) {
        setTimer1(500);
        update7SEG(index_led++);
        if (index_led >= MAX_LED) index_led = 0;
    }

    if (timer2_flag == 1) {
        setTimer2(10);
        updateLEDMatrix(index_led_matrix++);
        if (index_led_matrix >= MAX_LED_MATRIX) index_led_matrix = 0;
    }

    if (timer3_flag == 1) {
        setTimer3(ANIMATION_PERIOD);
        updateAnimation();
    }
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

  HAL_GPIO_WritePin(GPIOA, ENM0_Pin|ENM1_Pin|ENM2_Pin|ENM3_Pin
                          |ENM4_Pin|ENM5_Pin|ENM6_Pin|ENM7_Pin, GPIO_PIN_RESET);

  HAL_GPIO_WritePin(GPIOA, DOT_Pin|LED_RED_Pin|EN0_Pin|EN1_Pin
                          |EN2_Pin|EN3_Pin, GPIO_PIN_SET);

  HAL_GPIO_WritePin(GPIOB, SEG_A_Pin|SEG_B_Pin|SEG_C_Pin|SEG_D_Pin
                          |SEG_E_Pin|SEG_F_Pin|SEG_G_Pin, GPIO_PIN_SET);

  HAL_GPIO_WritePin(GPIOB, ROW2_Pin|ROW3_Pin|ROW4_Pin|ROW5_Pin
                          |ROW6_Pin|ROW7_Pin|ROW0_Pin|ROW1_Pin, GPIO_PIN_RESET);

  GPIO_InitStruct.Pin = ENM0_Pin|ENM1_Pin|DOT_Pin|LED_RED_Pin
                          |EN0_Pin|EN1_Pin|EN2_Pin|EN3_Pin
                          |ENM2_Pin|ENM3_Pin|ENM4_Pin|ENM5_Pin
                          |ENM6_Pin|ENM7_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = SEG_A_Pin|SEG_B_Pin|SEG_C_Pin|ROW2_Pin
                          |ROW3_Pin|ROW4_Pin|ROW5_Pin|ROW6_Pin
                          |ROW7_Pin|SEG_D_Pin|SEG_E_Pin|SEG_F_Pin
                          |SEG_G_Pin|ROW0_Pin|ROW1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}

/* USER CODE BEGIN 4 */

// Common anode: 0 = segment ON. Bit order: a b c d e f g
void display7SEG(int num)
{
    const uint8_t seg[10][7] = {
        {0,0,0,0,0,0,1}, {1,0,0,1,1,1,1}, {0,0,1,0,0,1,0}, {0,0,0,0,1,1,0},
        {1,0,0,1,1,0,0}, {0,1,0,0,1,0,0}, {0,1,0,0,0,0,0}, {0,0,0,1,1,1,1},
        {0,0,0,0,0,0,0}, {0,0,0,0,1,0,0}
    };
    const uint16_t pins[7] = {SEG_A_Pin, SEG_B_Pin, SEG_C_Pin, SEG_D_Pin,
                              SEG_E_Pin, SEG_F_Pin, SEG_G_Pin};
    for (int i = 0; i < 7; i++) {
        GPIO_PinState s = GPIO_PIN_SET;
        if (num >= 0 && num <= 9) s = seg[num][i] ? GPIO_PIN_SET : GPIO_PIN_RESET;
        HAL_GPIO_WritePin(GPIOB, pins[i], s);
    }
}

void update7SEG(int index)
{
    HAL_GPIO_WritePin(GPIOA, EN0_Pin|EN1_Pin|EN2_Pin|EN3_Pin, GPIO_PIN_SET);
    switch (index) {
        case 0: display7SEG(led_buffer[0]); HAL_GPIO_WritePin(EN0_GPIO_Port, EN0_Pin, GPIO_PIN_RESET); break;
        case 1: display7SEG(led_buffer[1]); HAL_GPIO_WritePin(EN1_GPIO_Port, EN1_Pin, GPIO_PIN_RESET); break;
        case 2: display7SEG(led_buffer[2]); HAL_GPIO_WritePin(EN2_GPIO_Port, EN2_Pin, GPIO_PIN_RESET); break;
        case 3: display7SEG(led_buffer[3]); HAL_GPIO_WritePin(EN3_GPIO_Port, EN3_Pin, GPIO_PIN_RESET); break;
        default: break;
    }
}

void updateClockBuffer(void)
{
    led_buffer[0] = hour / 10;
    led_buffer[1] = hour % 10;
    led_buffer[2] = minute / 10;
    led_buffer[3] = minute % 10;
}

// Scan by ROW: selected row = Low (cathode), column data = !bit (ENM Low -> lit)
void updateLEDMatrix(int index)
{
    if (index < 0 || index >= MAX_LED_MATRIX) return;
    uint8_t value = matrix_buffer[index];

    // 1. turn off all rows
    GPIOB->ODR |= 0xFF00;

    // 2. column data
    for (int i = 0; i < 8; i++) {
        HAL_GPIO_WritePin(enm_port[i], enm_pin[i],
                          (GPIO_PinState)(!((value >> i) & 1)));
    }

    // 3. turn on the selected row (PB8 + index)
    GPIOB->ODR &= ~((uint16_t)1 << (8 + index));
}

// Exercise 10: move the character "A" left and right.
// The character bounces between the two edges of the matrix:
//   center -> toward column 0 -> back -> toward column 7 -> back -> ...
void updateAnimation(void)
{
    // 1. next position
    shift_pos += shift_dir;
    if (shift_pos <= -MAX_SHIFT) shift_dir = 1;     // reached one edge: turn around
    if (shift_pos >=  MAX_SHIFT) shift_dir = -1;    // reached the other edge: turn around

    // 2. rebuild the buffer from the original pattern
    for (int i = 0; i < 8; i++) {
        if (shift_pos < 0)
            matrix_buffer[i] = (uint8_t)(A_pattern[i] >> (-shift_pos));
        else
            matrix_buffer[i] = (uint8_t)(A_pattern[i] << shift_pos);
    }
}

void setTimer0(int duration) { timer0_counter = duration / TIMER_CYCLE; timer0_flag = 0; }
void setTimer1(int duration) { timer1_counter = duration / TIMER_CYCLE; timer1_flag = 0; }
void setTimer2(int duration) { timer2_counter = duration / TIMER_CYCLE; timer2_flag = 0; }
void setTimer3(int duration) { timer3_counter = duration / TIMER_CYCLE; timer3_flag = 0; }

void timer_run(void)
{
    if (timer0_counter > 0) { timer0_counter--; if (timer0_counter == 0) timer0_flag = 1; }
    if (timer1_counter > 0) { timer1_counter--; if (timer1_counter == 0) timer1_flag = 1; }
    if (timer2_counter > 0) { timer2_counter--; if (timer2_counter == 0) timer2_flag = 1; }
    if (timer3_counter > 0) { timer3_counter--; if (timer3_counter == 0) timer3_flag = 1; }
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM2) {
        timer_run();
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
