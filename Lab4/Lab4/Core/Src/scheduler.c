/*
 * scheduler.c
 *
 *  Created on: Nov 9, 2026
 *      Author: Minh Quoc (2353026)
 *
 *  Cooperative Scheduler using a Sorted Differential-Delay Queue (Delta-List):
 *    - SCH_Update()         : O(1) time complexity in TIM2 10ms ISR (no loops!)
 *    - SCH_Add_Task()       : O(n) sorted delta insertion, returns unique TaskID
 *    - SCH_Delete_Task()    : O(n) lookup by TaskID, preserves delta invariants
 *    - SCH_Dispatch_Tasks() : Executes due callbacks in strict chronological + FIFO order
 */

#include "scheduler.h"

/* Task queue sorted by remaining execution time (differential Delay in ticks) */
static sTask    SCH_tasks_G[SCH_MAX_TASKS];
static uint8_t  current_num_tasks = 0U;
static uint32_t next_task_id      = 1U;

/* System uptime in 10ms ticks for get_time() */
static volatile uint32_t g_system_ticks = 0U;

/* Global error tracking variables (Section 2.3.9) */
uint8_t  Error_code_G       = ERROR_SCH_NO_ERROR;
static uint8_t  Last_error_code_G  = ERROR_SCH_NO_ERROR;
static uint32_t Error_tick_count_G = 0U;

/* Optional busy-check callback before entering CPU sleep mode */
static uint8_t (*s_sleep_guard_fn)(void) = 0;

/* ---------------- Internal helper functions ---------------- */

static void clear_slot(uint8_t index)
{
    SCH_tasks_G[index].pTask  = 0;
    SCH_tasks_G[index].Delay  = 0U;
    SCH_tasks_G[index].Period = 0U;
    SCH_tasks_G[index].RunMe  = 0U;
    SCH_tasks_G[index].TaskID = NO_TASK_ID;
}

static uint32_t allocate_task_id(void)
{
    /* Guarantee non-zero unique ID across active tasks */
    uint32_t candidate = next_task_id;
    uint8_t collision;

    do {
        if (candidate == NO_TASK_ID) {
            candidate = 1U;
        }
        collision = 0U;
        for (uint8_t i = 0U; i < current_num_tasks; i++) {
            if (SCH_tasks_G[i].TaskID == candidate) {
                collision = 1U;
                candidate++;
                break;
            }
        }
    } while (collision != 0U);

    next_task_id = candidate + 1U;
    return candidate;
}

/*
 * Insert `task` into the sorted differential-delay queue SCH_tasks_G.
 * Must be called with interrupts masked.
 * Uses `>=` during traversal so tasks expiring on the exact same tick
 * preserve strict FIFO registration order.
 */
static void insert_task_locked(sTask task)
{
    uint8_t idx = 0U;

    while ((idx < current_num_tasks) && (task.Delay >= SCH_tasks_G[idx].Delay)) {
        task.Delay -= SCH_tasks_G[idx].Delay;
        idx++;
    }

    /* Shift elements [idx .. current_num_tasks - 1] one position right */
    for (uint8_t j = current_num_tasks; j > idx; j--) {
        SCH_tasks_G[j] = SCH_tasks_G[j - 1U];
    }

    /* Subtract inserted task's relative delay from the following element */
    if (idx < current_num_tasks) {
        SCH_tasks_G[idx + 1U].Delay -= task.Delay;
    }

    task.RunMe = ((idx == 0U) && (task.Delay == 0U)) ? 1U : 0U;
    SCH_tasks_G[idx] = task;
    current_num_tasks++;
}

/* ---------------- Public Scheduler API ---------------- */

void SCH_Set_Sleep_Guard(uint8_t (*pIsBusy)(void))
{
    s_sleep_guard_fn = pIsBusy;
}

void SCH_Init(void)
{
    uint32_t primask = __get_PRIMASK();
    __disable_irq();

    for (uint8_t i = 0U; i < SCH_MAX_TASKS; i++) {
        clear_slot(i);
    }
    current_num_tasks  = 0U;
    next_task_id       = 1U;
    g_system_ticks     = 0U;
    Error_code_G       = ERROR_SCH_NO_ERROR;
    Last_error_code_G  = ERROR_SCH_NO_ERROR;
    Error_tick_count_G = 0U;

    if (!primask) {
        __enable_irq();
    }
}

/*
 * SCH_Update: Invoked every 10 ms inside HAL_TIM_PeriodElapsedCallback.
 * Strictly O(1) time complexity: only updates the head of the delta-list!
 */
void SCH_Update(void)
{
    g_system_ticks++;

    if (current_num_tasks > 0U) {
        if (SCH_tasks_G[0].Delay > 0U) {
            SCH_tasks_G[0].Delay--;
        }
        if (SCH_tasks_G[0].Delay == 0U) {
            SCH_tasks_G[0].RunMe = 1U;
        }
    }
}

/*
 * SCH_Add_Task: Adds a periodic or one-shot task to the sorted delta queue.
 *   - DELAY  : Initial delay in milliseconds (converted to 10ms ticks)
 *   - PERIOD : Repeat interval in milliseconds (0 = one-shot task)
 *   - Returns: Unique TaskID (> 0), or NO_TASK_ID (0) if queue is full.
 */
uint32_t SCH_Add_Task(void (*pFunction)(), uint32_t DELAY, uint32_t PERIOD)
{
    if (pFunction == 0) {
        return NO_TASK_ID;
    }

    uint32_t delay_ticks  = DELAY / SCH_TICK_MS;
    uint32_t period_ticks = PERIOD / SCH_TICK_MS;

    if ((DELAY > 0U) && (delay_ticks == 0U)) {
        delay_ticks = 1U;
    }
    if ((PERIOD > 0U) && (period_ticks == 0U)) {
        period_ticks = 1U;
    }

    uint32_t primask = __get_PRIMASK();
    __disable_irq();

    if (current_num_tasks >= SCH_MAX_TASKS) {
        Error_code_G = ERROR_SCH_TOO_MANY_TASKS;
        if (!primask) {
            __enable_irq();
        }
        return NO_TASK_ID;
    }

    sTask new_task;
    new_task.pTask  = (void (*)(void))pFunction;
    new_task.Delay  = delay_ticks;
    new_task.Period = period_ticks;
    new_task.RunMe  = 0U;
    new_task.TaskID = allocate_task_id();

    insert_task_locked(new_task);

    if (!primask) {
        __enable_irq();
    }

    return new_task.TaskID;
}

/*
 * SCH_Delete_Task: Deletes an active task matching `taskID`.
 * Transfers its remaining differential Delay to the next task in queue
 * so all subsequent task timings remain 100% unaffected.
 */
uint8_t SCH_Delete_Task(uint32_t taskID)
{
    if (taskID == NO_TASK_ID) {
        Error_code_G = ERROR_SCH_CANNOT_DELETE_TASK;
        return RETURN_ERROR;
    }

    uint32_t primask = __get_PRIMASK();
    __disable_irq();

    uint8_t found_idx = SCH_MAX_TASKS;
    for (uint8_t i = 0U; i < current_num_tasks; i++) {
        if (SCH_tasks_G[i].TaskID == taskID) {
            found_idx = i;
            break;
        }
    }

    if (found_idx == SCH_MAX_TASKS) {
        Error_code_G = ERROR_SCH_CANNOT_DELETE_TASK;
        if (!primask) {
            __enable_irq();
        }
        return RETURN_ERROR;
    }

    /* Preserve total delay of tasks following the deleted slot */
    if ((found_idx + 1U) < current_num_tasks) {
        SCH_tasks_G[found_idx + 1U].Delay += SCH_tasks_G[found_idx].Delay;
    }

    /* Compact the queue */
    for (uint8_t i = found_idx; i < (current_num_tasks - 1U); i++) {
        SCH_tasks_G[i] = SCH_tasks_G[i + 1U];
    }
    clear_slot(current_num_tasks - 1U);
    current_num_tasks--;

    if (current_num_tasks > 0U) {
        SCH_tasks_G[0].RunMe = (SCH_tasks_G[0].Delay == 0U) ? 1U : 0U;
    }

    if (!primask) {
        __enable_irq();
    }

    return RETURN_NORMAL;
}

/*
 * SCH_Dispatch_Tasks: Runs in the main super-loop `while(1)`.
 * Pops and executes all due tasks (Delay == 0), re-schedules periodic tasks
 * without cumulative drift, reports status, and puts MCU into low-power sleep.
 */
void SCH_Dispatch_Tasks(void)
{
    while (1) {
        sTask   due_task;
        uint8_t has_due = 0U;

        uint32_t primask = __get_PRIMASK();
        __disable_irq();

        if ((current_num_tasks > 0U) && (SCH_tasks_G[0].RunMe > 0U)) {
            due_task = SCH_tasks_G[0];
            has_due  = 1U;

            /* Pop head [0] */
            for (uint8_t i = 0U; i < (current_num_tasks - 1U); i++) {
                SCH_tasks_G[i] = SCH_tasks_G[i + 1U];
            }
            clear_slot(current_num_tasks - 1U);
            current_num_tasks--;

            /* Re-schedule periodic task (Period > 0); one-shot (Period == 0) is auto-removed */
            if (due_task.Period > 0U) {
                due_task.Delay = due_task.Period;
                due_task.RunMe = 0U;
                insert_task_locked(due_task);
            }

            /* Promote next co-expiring task (relative Delay == 0) immediately */
            if ((current_num_tasks > 0U) && (SCH_tasks_G[0].Delay == 0U)) {
                SCH_tasks_G[0].RunMe = 1U;
            }
        }

        if (!primask) {
            __enable_irq();
        }

        if (has_due == 0U) {
            break;
        }

        /* Execute task callback with interrupts enabled */
        if (due_task.pTask != 0) {
            (*due_task.pTask)();
        }
    }

    SCH_Report_Status();
    SCH_Go_To_Sleep();
}

void SCH_Report_Status(void)
{
    if (Error_code_G != Last_error_code_G) {
        Last_error_code_G = Error_code_G;
        if (Error_code_G != ERROR_SCH_NO_ERROR) {
            Error_tick_count_G = 6000U; /* Keep error latched for 60s (at 10ms tick) */
        } else {
            Error_tick_count_G = 0U;
        }
    } else {
        if (Error_tick_count_G != 0U) {
            if (--Error_tick_count_G == 0U) {
                Error_code_G = ERROR_SCH_NO_ERROR;
            }
        }
    }
}

void SCH_Go_To_Sleep(void)
{
    /* Only enter WFI sleep when background non-blocking peripherals (e.g. UART TX) are idle */
    if ((s_sleep_guard_fn == 0) || (s_sleep_guard_fn() == 0U)) {
        HAL_PWR_EnterSLEEPMode(PWR_MAINREGULATOR_ON, PWR_SLEEPENTRY_WFI);
    }
}

/*
 * get_time: Returns elapsed time in milliseconds, accurate to 10 ms (Section 5).
 */
uint32_t get_time(void)
{
    return g_system_ticks * SCH_TICK_MS;
}
