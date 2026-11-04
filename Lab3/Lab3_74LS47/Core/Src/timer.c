/*
 * timer.c
 *
 * Lab 3 - Section 4.3.3 & Exercise 4: Software Timer & TIM2 ISR Callback
 */

#include "main.h"
#include "timer.h"
#include "input_reading.h"

static int timer_counter[NUM_TIMERS] = {0};
static int timer_flag[NUM_TIMERS] = {0};

void setTimer(int index, int duration_ms)
{
    if (index < 0 || index >= NUM_TIMERS) return;
    int ticks = duration_ms / TIMER_CYCLE;
    if (duration_ms > 0 && ticks <= 0)
    {
        ticks = 1;
    }
    timer_counter[index] = ticks;
    timer_flag[index] = 0;
}

int isTimerExpired(int index)
{
    if (index < 0 || index >= NUM_TIMERS) return 0;
    if (timer_flag[index] == 1)
    {
        timer_flag[index] = 0;
        return 1;
    }
    return 0;
}

void clearTimer(int index)
{
    if (index < 0 || index >= NUM_TIMERS) return;
    timer_counter[index] = 0;
    timer_flag[index] = 0;
}

void timer_run(void)
{
    for (int i = 0; i < NUM_TIMERS; i++)
    {
        if (timer_counter[i] > 0)
        {
            timer_counter[i]--;
            if (timer_counter[i] <= 0)
            {
                timer_flag[i] = 1;
            }
        }
    }
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM2)
    {
        timer_run();
        button_reading();
    }
}
