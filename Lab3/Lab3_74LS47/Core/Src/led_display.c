/*
 * led_display.c
 *
 * Lab 3 - Exercise 6: Hardware Output Driver for 12 Traffic LEDs & 74LS47 7-Segment LEDs
 */

#include "main.h"
#include "led_display.h"

static void writeBCD74LS47(GPIO_TypeDef *port,
                           uint16_t pinA, uint16_t pinB,
                           uint16_t pinC, uint16_t pinD,
                           int digit)
{
    if (digit < 0 || digit > 9)
    {
        /* 0x0F (1111) blanks all segments on 74LS47 */
        HAL_GPIO_WritePin(port, pinA | pinB | pinC | pinD, GPIO_PIN_SET);
        return;
    }

    HAL_GPIO_WritePin(port, pinA, (digit & 0x01) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(port, pinB, (digit & 0x02) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(port, pinC, (digit & 0x04) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(port, pinD, (digit & 0x08) ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

void clearAllTrafficLeds(void)
{
    HAL_GPIO_WritePin(GPIOB,
                      RED_NS_Pin | YELLOW_NS_Pin | GREEN_NS_Pin |
                      RED_EW_Pin | YELLOW_EW_Pin | GREEN_EW_Pin,
                      GPIO_PIN_SET);
}

void setRoad1Color(TrafficColor color)
{
    HAL_GPIO_WritePin(GPIOB, RED_NS_Pin | YELLOW_NS_Pin | GREEN_NS_Pin, GPIO_PIN_SET);
    switch (color)
    {
        case COLOR_RED:
            HAL_GPIO_WritePin(RED_NS_GPIO_Port, RED_NS_Pin, GPIO_PIN_RESET);
            break;
        case COLOR_AMBER:
            HAL_GPIO_WritePin(YELLOW_NS_GPIO_Port, YELLOW_NS_Pin, GPIO_PIN_RESET);
            break;
        case COLOR_GREEN:
            HAL_GPIO_WritePin(GREEN_NS_GPIO_Port, GREEN_NS_Pin, GPIO_PIN_RESET);
            break;
        default:
            break;
    }
}

void setRoad2Color(TrafficColor color)
{
    HAL_GPIO_WritePin(GPIOB, RED_EW_Pin | YELLOW_EW_Pin | GREEN_EW_Pin, GPIO_PIN_SET);
    switch (color)
    {
        case COLOR_RED:
            HAL_GPIO_WritePin(RED_EW_GPIO_Port, RED_EW_Pin, GPIO_PIN_RESET);
            break;
        case COLOR_AMBER:
            HAL_GPIO_WritePin(YELLOW_EW_GPIO_Port, YELLOW_EW_Pin, GPIO_PIN_RESET);
            break;
        case COLOR_GREEN:
            HAL_GPIO_WritePin(GREEN_EW_GPIO_Port, GREEN_EW_Pin, GPIO_PIN_RESET);
            break;
        default:
            break;
    }
}

void setModeBlinkLeds(int mode, int led_on)
{
    clearAllTrafficLeds();
    if (!led_on) return;

    switch (mode)
    {
        case 2: /* Mode 2: Blink all 4 Red LEDs (B8 & B11) at 2 Hz */
            HAL_GPIO_WritePin(GPIOB, RED_NS_Pin | RED_EW_Pin, GPIO_PIN_RESET);
            break;
        case 3: /* Mode 3: Blink all 4 Amber/Yellow LEDs (B9 & B12) at 2 Hz */
            HAL_GPIO_WritePin(GPIOB, YELLOW_NS_Pin | YELLOW_EW_Pin, GPIO_PIN_RESET);
            break;
        case 4: /* Mode 4: Blink all 4 Green LEDs (B10 & B13) at 2 Hz */
            HAL_GPIO_WritePin(GPIOB, GREEN_NS_Pin | GREEN_EW_Pin, GPIO_PIN_RESET);
            break;
        default:
            break;
    }
}

void setConfirmLed(int led_on)
{
    HAL_GPIO_WritePin(SET_LED_GPIO_Port, SET_LED_Pin,
                      led_on ? GPIO_PIN_RESET : GPIO_PIN_SET);
}

void update7SEGBuffer(int top_two_digits, int left_two_digits)
{
    if (top_two_digits < 0)   top_two_digits = 0;
    if (top_two_digits > 99)  top_two_digits = 99;
    if (left_two_digits < 0)  left_two_digits = 0;
    if (left_two_digits > 99) left_two_digits = 99;

    /* U3 (tens) & U4 (ones) on PA0..PA7 */
    writeBCD74LS47(GPIOA, U3_A_Pin, U3_B_Pin, U3_C_Pin, U3_D_Pin, top_two_digits / 10);
    writeBCD74LS47(GPIOA, U4_A_Pin, U4_B_Pin, U4_C_Pin, U4_D_Pin, top_two_digits % 10);

    /* U5 (tens) & U6 (ones) on PA8..PA15 */
    writeBCD74LS47(GPIOA, U5_A_Pin, U5_B_Pin, U5_C_Pin, U5_D_Pin, left_two_digits / 10);
    writeBCD74LS47(GPIOA, U6_A_Pin, U6_B_Pin, U6_C_Pin, U6_D_Pin, left_two_digits % 10);
}

void updateModeAux7SEG(int mode)
{
    /* Also output current mode (1..4) on PB4..PB7 in case U2 is kept on schematic */
    writeBCD74LS47(GPIOB, U2_A_Pin, U2_B_Pin, U2_C_Pin, U2_D_Pin, mode);
}
