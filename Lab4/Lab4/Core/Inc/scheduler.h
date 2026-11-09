/*
 * scheduler.h
 *
 *  Created on: Nov 9, 2026
 *      Author: Minh Quoc (2353026)
 *
 *  O(1) Sorted Delta-List Cooperative Scheduler for STM32F103C6
 */

#ifndef INC_SCHEDULER_H_
#define INC_SCHEDULER_H_

#include "main.h"
#include <stdint.h>

/* Scheduler configuration */
#define SCH_MAX_TASKS                       40U
#define NO_TASK_ID                          0U
#define SCH_TICK_MS                         10U

/* Return codes for SCH_Delete_Task */
#define RETURN_NORMAL                       0U
#define RETURN_ERROR                        1U

/* Scheduler error codes (Section 2.3.9) */
#define ERROR_SCH_NO_ERROR                  0U
#define ERROR_SCH_TOO_MANY_TASKS            1U
#define ERROR_SCH_CANNOT_DELETE_TASK        2U

/* Task control structure (Program 1.7) */
typedef struct {
    void (*pTask)(void);    /* Pointer to the task (void (void) function) */
    uint32_t Delay;         /* Differential delay (in ticks) relative to previous task */
    uint32_t Period;        /* Interval (in ticks) between subsequent runs (0 = one-shot) */
    uint8_t  RunMe;         /* Set when head task is due to execute */
    uint32_t TaskID;        /* Unique non-zero identifier for this task */
} sTask;

/* Required Cooperative Scheduler API (Section 4 - Problem) */
void     SCH_Init(void);
void     SCH_Update(void);
void     SCH_Dispatch_Tasks(void);
uint32_t SCH_Add_Task(void (*pFunction)(), uint32_t DELAY, uint32_t PERIOD);
uint8_t  SCH_Delete_Task(uint32_t taskID);

/* Power saving, diagnostics & timestamp API (Sections 2.3.8, 2.3.9 & 5) */
void     SCH_Go_To_Sleep(void);
void     SCH_Report_Status(void);
uint32_t get_time(void);

/* Optional hook allowing background UART TX buffer to finish before __WFI() */
void     SCH_Set_Sleep_Guard(uint8_t (*pIsBusy)(void));

#endif /* INC_SCHEDULER_H_ */
