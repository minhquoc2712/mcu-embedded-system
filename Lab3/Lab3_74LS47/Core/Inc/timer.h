/*
 * timer.h
 *
 * Lab 3 - Exercise 4: Parameterized Hardware & Software Timer Module
 * Changing TIMER_CYCLE here (e.g. 1, 10, 100 ms) automatically scales both
 * TIM2's period and all software timers / button hold counters without
 * changing system timing (2 Hz blinking, 1 s countdown, etc.).
 */

#ifndef INC_TIMER_H_
#define INC_TIMER_H_

#include "main.h"

/* Single parameter for hardware timer interrupt cycle (in milliseconds) */
#ifndef TIMER_CYCLE
#define TIMER_CYCLE     10
#endif

/* Software timer channels */
#define NUM_TIMERS      4
#define TIMER_TRAFFIC   0   /* 1000 ms tick for Mode 1 traffic countdown */
#define TIMER_BLINK     1   /* 250 ms tick for 2 Hz LED blinking in Modes 2-4 */
#define TIMER_7SEG      2   /* Auxiliary 7-segment display update timer channel */
#define TIMER_REPEAT    3   /* Auto-increment timer when Button 2 held > 1s */

void setTimer(int index, int duration_ms);
int  isTimerExpired(int index);
void clearTimer(int index);
void timer_run(void);

#endif /* INC_TIMER_H_ */
