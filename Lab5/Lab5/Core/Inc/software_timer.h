/*
 * software_timer.h
 *
 *  Created on: Nov 25, 2026
 *      Author: asus
 */
#ifndef INC_SOFTWARE_TIMER_H_
#define INC_SOFTWARE_TIMER_H_
#include <stdint.h>
#define MAX_TIMERS   2
#define TIMER_CYCLE  10  // 10 ms moi ngat TIM2
void setTimer(int index, int duration_ms);
void clearTimer(int index);
int isTimerExpired(int index);
void timerRun(void);


#endif /* INC_SOFTWARE_TIMER_H_ */
