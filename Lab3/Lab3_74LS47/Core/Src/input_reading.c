/*
 * input_reading.c
 *
 * Lab 3 - Section 4.3.1 & Exercise 5: Read Port Pins and Debouncing (3 Buttons)
 *   - Index 0 (BUTTON_1 / PB1): Mode selection button
 *   - Index 1 (BUTTON_2 / PB2): Modify / increase duration value (1..99)
 *   - Index 2 (BUTTON_3 / PB3): Set / confirm duration value
 */

#include "main.h"
#include "timer.h"
#include "input_reading.h"

#define N0_OF_BUTTONS                   3
#define DURATION_FOR_AUTO_INCREASING    (1000 / TIMER_CYCLE)
#define BUTTON_IS_PRESSED               GPIO_PIN_RESET
#define BUTTON_IS_RELEASED              GPIO_PIN_SET

static GPIO_TypeDef * const buttonPort[N0_OF_BUTTONS] = {
    BUTTON_1_GPIO_Port,
    BUTTON_2_GPIO_Port,
    BUTTON_3_GPIO_Port
};

static const uint16_t buttonPin[N0_OF_BUTTONS] = {
    BUTTON_1_Pin,
    BUTTON_2_Pin,
    BUTTON_3_Pin
};

/* Initialize all buffers to RELEASED so startup never triggers a false press */
static GPIO_PinState buttonBuffer[N0_OF_BUTTONS] = {
    BUTTON_IS_RELEASED, BUTTON_IS_RELEASED, BUTTON_IS_RELEASED
};
static GPIO_PinState debounceButtonBuffer1[N0_OF_BUTTONS] = {
    BUTTON_IS_RELEASED, BUTTON_IS_RELEASED, BUTTON_IS_RELEASED
};
static GPIO_PinState debounceButtonBuffer2[N0_OF_BUTTONS] = {
    BUTTON_IS_RELEASED, BUTTON_IS_RELEASED, BUTTON_IS_RELEASED
};

static uint8_t  flagForButtonPress1s[N0_OF_BUTTONS]    = {0, 0, 0};
static uint16_t counterForButtonPress1s[N0_OF_BUTTONS] = {0, 0, 0};

void button_reading(void)
{
    for (unsigned char i = 0; i < N0_OF_BUTTONS; i++)
    {
        debounceButtonBuffer2[i] = debounceButtonBuffer1[i];
        debounceButtonBuffer1[i] = HAL_GPIO_ReadPin(buttonPort[i], buttonPin[i]);

        if (debounceButtonBuffer1[i] == debounceButtonBuffer2[i])
        {
            buttonBuffer[i] = debounceButtonBuffer1[i];
        }

        if (buttonBuffer[i] == BUTTON_IS_PRESSED)
        {
            if (counterForButtonPress1s[i] < DURATION_FOR_AUTO_INCREASING)
            {
                counterForButtonPress1s[i]++;
                if (counterForButtonPress1s[i] >= DURATION_FOR_AUTO_INCREASING)
                {
                    flagForButtonPress1s[i] = 1;
                }
            }
            else
            {
                flagForButtonPress1s[i] = 1;
            }
        }
        else
        {
            counterForButtonPress1s[i] = 0;
            flagForButtonPress1s[i] = 0;
        }
    }
}

unsigned char is_button_pressed(unsigned char index)
{
    if (index >= N0_OF_BUTTONS) return 0;
    return (buttonBuffer[index] == BUTTON_IS_PRESSED);
}

unsigned char is_button_pressed_1s(unsigned char index)
{
    if (index >= N0_OF_BUTTONS) return 0xff;
    return (flagForButtonPress1s[index] == 1);
}
