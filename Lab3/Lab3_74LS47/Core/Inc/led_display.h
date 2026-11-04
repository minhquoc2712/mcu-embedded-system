/*
 * led_display.h
 *
 * Lab 3 - Exercise 6: Hardware Output Driver for 12 Traffic LEDs & 74LS47 7-Segment LEDs
 */

#ifndef INC_LED_DISPLAY_H_
#define INC_LED_DISPLAY_H_

#include "main.h"

typedef enum {
    COLOR_OFF = 0,
    COLOR_RED,
    COLOR_AMBER,
    COLOR_GREEN
} TrafficColor;

/* Traffic LED control (Active-LOW on PB8..PB13: RESET = ON, SET = OFF) */
void clearAllTrafficLeds(void);
void setRoad1Color(TrafficColor color);
void setRoad2Color(TrafficColor color);
void setModeBlinkLeds(int mode, int led_on);
void setConfirmLed(int led_on);

/* 74LS47 BCD 7-Segment Display Driver (U3, U4 = Top Pair; U5, U6 = Left Pair; U2 = Mode) */
void update7SEGBuffer(int top_two_digits, int left_two_digits);
void updateModeAux7SEG(int mode);

#endif /* INC_LED_DISPLAY_H_ */
