/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file - Lab 3 (74LS47 Schematic Edition)
  *                   MCU: STM32F103C6 (Matches A0..A15, B1..B3, B4..B7, B8..B13)
  ******************************************************************************
  */
/* USER CODE END Header */

#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f1xx_hal.h"

void Error_Handler(void);

/* BCD Outputs on GPIOA: U3 (A0..A3), U4 (A4..A7), U5 (A8..A11), U6 (A12..A15) */
#define U3_A_Pin        GPIO_PIN_0
#define U3_B_Pin        GPIO_PIN_1
#define U3_C_Pin        GPIO_PIN_2
#define U3_D_Pin        GPIO_PIN_3

#define U4_A_Pin        GPIO_PIN_4
#define U4_B_Pin        GPIO_PIN_5
#define U4_C_Pin        GPIO_PIN_6
#define U4_D_Pin        GPIO_PIN_7

#define U5_A_Pin        GPIO_PIN_8
#define U5_B_Pin        GPIO_PIN_9
#define U5_C_Pin        GPIO_PIN_10
#define U5_D_Pin        GPIO_PIN_11

#define U6_A_Pin        GPIO_PIN_12
#define U6_B_Pin        GPIO_PIN_13
#define U6_C_Pin        GPIO_PIN_14
#define U6_D_Pin        GPIO_PIN_15

/* Optional U2 Mode 7-Seg BCD Outputs on GPIOB (B4..B7) & Set Indicator D13 (PB0) */
#define SET_LED_Pin     GPIO_PIN_0
#define SET_LED_GPIO_Port GPIOB

#define U2_A_Pin        GPIO_PIN_4
#define U2_B_Pin        GPIO_PIN_5
#define U2_C_Pin        GPIO_PIN_6
#define U2_D_Pin        GPIO_PIN_7

/* 3 Debounced Buttons on GPIOB (B1, B2, B3) */
#define BUTTON_1_Pin       GPIO_PIN_1
#define BUTTON_1_GPIO_Port GPIOB
#define BUTTON_2_Pin       GPIO_PIN_2
#define BUTTON_2_GPIO_Port GPIOB
#define BUTTON_3_Pin       GPIO_PIN_3
#define BUTTON_3_GPIO_Port GPIOB

/* 12 Crossroad Traffic LEDs on GPIOB (Active-LOW: RESET = ON, SET = OFF)
 * Vertical Road (D1..D6): B8 = Red, B9 = Yellow, B10 = Green
 * Horizontal Road (D7..D12): B11 = Red, B12 = Yellow, B13 = Green
 */
#define RED_NS_Pin         GPIO_PIN_8
#define RED_NS_GPIO_Port   GPIOB
#define YELLOW_NS_Pin      GPIO_PIN_9
#define YELLOW_NS_GPIO_Port GPIOB
#define GREEN_NS_Pin       GPIO_PIN_10
#define GREEN_NS_GPIO_Port GPIOB

#define RED_EW_Pin         GPIO_PIN_11
#define RED_EW_GPIO_Port   GPIOB
#define YELLOW_EW_Pin      GPIO_PIN_12
#define YELLOW_EW_GPIO_Port GPIOB
#define GREEN_EW_Pin       GPIO_PIN_13
#define GREEN_EW_GPIO_Port GPIOB

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
