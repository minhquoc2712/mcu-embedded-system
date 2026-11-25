/*
 * software_timer.c
 *
 *  Created on: Nov 25, 2026
 *      Author: asus
 */

#include "software_timer.h"

static volatile int timer_counter[MAX_TIMERS] = {0};
static volatile int timer_flag[MAX_TIMERS] = {0};

void setTimer(int index, int duration_ms) {
    if (index < 0 || index >= MAX_TIMERS) return;
    timer_counter[index] = duration_ms / TIMER_CYCLE;
    timer_flag[index] = (timer_counter[index] <= 0) ? 1 : 0;
}

int isTimerExpired(int index) {
    if (index < 0 || index >= MAX_TIMERS) return 0;
    if (timer_flag[index] == 1) {
        timer_flag[index] = 0;
        return 1;
    }
    return 0;
}

void timerRun(void) {
    for (int i = 0; i < MAX_TIMERS; i++) {
        if (timer_counter[i] > 0) {
            timer_counter[i]--;
            if (timer_counter[i] == 0) {
                timer_flag[i] = 1;
            }
        }
    }
}
