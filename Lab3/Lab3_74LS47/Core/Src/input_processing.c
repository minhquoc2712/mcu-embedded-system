/*
 * input_processing.c
 *
 * Lab 3 - Section 4.4.2 & Exercises 1, 5, 6, 7, 8, 9, 10:
 * Complete FSM for 3 Debounced Buttons (B1, B2, B3) and 4 Traffic Light Modes:
 *   - Mode 1: Normal Crossroad Traffic Light (Synchronized 4-phase FSM)
 *   - Mode 2: Modify Red LED duration (1..99), blink Red LEDs at 2 Hz
 *   - Mode 3: Modify Amber LED duration (1..99), blink Amber LEDs at 2 Hz
 *   - Mode 4: Modify Green LED duration (1..99), blink Green LEDs at 2 Hz
 */

#include "main.h"
#include "timer.h"
#include "input_reading.h"
#include "led_display.h"
#include "input_processing.h"

/* Button indices: B1=0, B2=1, B3=2 */
#define BTN_MODE    0
#define BTN_MODIFY  1
#define BTN_SET     2

/* Auto-repeat interval once Button 2 is held > 1s (200 ms = 5 steps/second) */
#define AUTO_REPEAT_PERIOD_MS   200

/* Per-button 3-state FSM (Section 4.4.2 Program 1.7) */
enum ButtonState {
    BUTTON_RELEASED = 0,
    BUTTON_PRESSED,
    BUTTON_PRESSED_MORE_THAN_1_SECOND
};

/* System operating modes (Specification 5.1) */
typedef enum {
    MODE_1_NORMAL       = 1,
    MODE_2_MODIFY_RED   = 2,
    MODE_3_MODIFY_AMBER = 3,
    MODE_4_MODIFY_GREEN = 4
} SystemMode;

/* Synchronized 4-phase Crossroad FSM for Mode 1 */
typedef enum {
    PHASE_NS_RED_EW_GREEN = 0,
    PHASE_NS_RED_EW_AMBER,
    PHASE_NS_GREEN_EW_RED,
    PHASE_NS_AMBER_EW_RED
} CrossroadPhase;

static enum ButtonState buttonState[3] = {
    BUTTON_RELEASED,
    BUTTON_RELEASED,
    BUTTON_RELEASED
};

/* Configured durations in seconds (range 1..99; default: Red=10s, Amber=4s, Green=6s) */
static int red_duration   = 10;
static int amber_duration = 4;
static int green_duration = 6;

static SystemMode current_mode = MODE_1_NORMAL;
static int temp_duration = 10;
static int blink_on = 1;

static CrossroadPhase current_phase = PHASE_NS_RED_EW_GREEN;
static int counter_ns = 10;
static int counter_ew = 6;

static void enter_mode_1(void)
{
    current_mode  = MODE_1_NORMAL;
    current_phase = PHASE_NS_RED_EW_GREEN;
    counter_ns    = red_duration;
    counter_ew    = green_duration;

    setConfirmLed(0);
    setRoad1Color(COLOR_RED);
    setRoad2Color(COLOR_GREEN);
    update7SEGBuffer(counter_ns, counter_ew);
    updateModeAux7SEG(1);

    clearTimer(TIMER_BLINK);
    setTimer(TIMER_TRAFFIC, 1000);
}

static void enter_modify_mode(SystemMode mode)
{
    current_mode  = mode;
    /* Start from 01 so the user can quickly set ANY value (including < current time) */
    temp_duration = 1;

    blink_on = 1;
    setConfirmLed(0);
    setModeBlinkLeds((int)current_mode, blink_on);
    /* Spec 5.1: 2 seven-seg LEDs (U3,U4) show value (01..99), other 2 (U5,U6) show mode (02..04) */
    update7SEGBuffer(temp_duration, (int)current_mode);
    updateModeAux7SEG((int)current_mode);

    clearTimer(TIMER_TRAFFIC);
    /* 2 Hz blinking -> period = 500 ms -> toggle every 250 ms */
    setTimer(TIMER_BLINK, 250);
}

static void increment_temp_duration(void)
{
    temp_duration++;
    if (temp_duration > 99)
    {
        temp_duration = 1;
    }
    /* Value has been modified again and is not saved yet -> turn OFF D13 */
    setConfirmLed(0);
    update7SEGBuffer(temp_duration, (int)current_mode);
}

/* Save chosen value and maintain Red == Green + Amber crossroad safety */
static void commit_temp_duration(void)
{
    if (current_mode == MODE_2_MODIFY_RED)
    {
        red_duration = temp_duration;
        if (red_duration < 2)
        {
            red_duration = 2;
        }
        if (amber_duration >= red_duration)
        {
            amber_duration = 1;
        }
        green_duration = red_duration - amber_duration;
    }
    else if (current_mode == MODE_3_MODIFY_AMBER)
    {
        amber_duration = temp_duration;
        if (amber_duration + green_duration > 99)
        {
            green_duration = (99 - amber_duration >= 1) ? (99 - amber_duration) : 1;
        }
        red_duration = green_duration + amber_duration;
    }
    else if (current_mode == MODE_4_MODIFY_GREEN)
    {
        green_duration = temp_duration;
        if (green_duration + amber_duration > 99)
        {
            amber_duration = (99 - green_duration >= 1) ? (99 - green_duration) : 1;
        }
        red_duration = green_duration + amber_duration;
    }
    setConfirmLed(1);
}

static void run_normal_traffic_fsm(void)
{
    if (isTimerExpired(TIMER_TRAFFIC))
    {
        setTimer(TIMER_TRAFFIC, 1000);

        counter_ns--;
        counter_ew--;

        switch (current_phase)
        {
            case PHASE_NS_RED_EW_GREEN:
                if (counter_ew <= 0)
                {
                    current_phase = PHASE_NS_RED_EW_AMBER;
                    counter_ew    = amber_duration;
                    setRoad1Color(COLOR_RED);
                    setRoad2Color(COLOR_AMBER);
                }
                break;

            case PHASE_NS_RED_EW_AMBER:
                if (counter_ew <= 0 || counter_ns <= 0)
                {
                    current_phase = PHASE_NS_GREEN_EW_RED;
                    counter_ns    = green_duration;
                    counter_ew    = red_duration;
                    setRoad1Color(COLOR_GREEN);
                    setRoad2Color(COLOR_RED);
                }
                break;

            case PHASE_NS_GREEN_EW_RED:
                if (counter_ns <= 0)
                {
                    current_phase = PHASE_NS_AMBER_EW_RED;
                    counter_ns    = amber_duration;
                    setRoad1Color(COLOR_AMBER);
                    setRoad2Color(COLOR_RED);
                }
                break;

            case PHASE_NS_AMBER_EW_RED:
            default:
                if (counter_ns <= 0 || counter_ew <= 0)
                {
                    current_phase = PHASE_NS_RED_EW_GREEN;
                    counter_ns    = red_duration;
                    counter_ew    = green_duration;
                    setRoad1Color(COLOR_RED);
                    setRoad2Color(COLOR_GREEN);
                }
                break;
        }

        update7SEGBuffer(counter_ns, counter_ew);
    }
}

static void run_modify_blink_fsm(void)
{
    if (isTimerExpired(TIMER_BLINK))
    {
        setTimer(TIMER_BLINK, 250);
        blink_on = !blink_on;
        setModeBlinkLeds((int)current_mode, blink_on);
    }
}

/* Button 1 (PB1): Mode selection (1 -> 2 -> 3 -> 4 -> 1) */
static void process_button_mode(void)
{
    switch (buttonState[BTN_MODE])
    {
        case BUTTON_RELEASED:
            if (is_button_pressed(BTN_MODE))
            {
                buttonState[BTN_MODE] = BUTTON_PRESSED;
                if (current_mode == MODE_1_NORMAL)
                {
                    enter_modify_mode(MODE_2_MODIFY_RED);
                }
                else if (current_mode == MODE_2_MODIFY_RED)
                {
                    enter_modify_mode(MODE_3_MODIFY_AMBER);
                }
                else if (current_mode == MODE_3_MODIFY_AMBER)
                {
                    enter_modify_mode(MODE_4_MODIFY_GREEN);
                }
                else
                {
                    enter_mode_1();
                }
            }
            break;

        case BUTTON_PRESSED:
            if (!is_button_pressed(BTN_MODE))
            {
                buttonState[BTN_MODE] = BUTTON_RELEASED;
            }
            else if (is_button_pressed_1s(BTN_MODE))
            {
                buttonState[BTN_MODE] = BUTTON_PRESSED_MORE_THAN_1_SECOND;
            }
            break;

        case BUTTON_PRESSED_MORE_THAN_1_SECOND:
            if (!is_button_pressed(BTN_MODE))
            {
                buttonState[BTN_MODE] = BUTTON_RELEASED;
            }
            break;
    }
}

/* Button 2 (PB2): Increase duration (1..99), auto-repeat every 200ms if held > 1s */
static void process_button_modify(void)
{
    switch (buttonState[BTN_MODIFY])
    {
        case BUTTON_RELEASED:
            if (is_button_pressed(BTN_MODIFY))
            {
                buttonState[BTN_MODIFY] = BUTTON_PRESSED;
                if (current_mode != MODE_1_NORMAL)
                {
                    increment_temp_duration();
                }
            }
            break;

        case BUTTON_PRESSED:
            if (!is_button_pressed(BTN_MODIFY))
            {
                buttonState[BTN_MODIFY] = BUTTON_RELEASED;
            }
            else if (is_button_pressed_1s(BTN_MODIFY))
            {
                buttonState[BTN_MODIFY] = BUTTON_PRESSED_MORE_THAN_1_SECOND;
                if (current_mode != MODE_1_NORMAL)
                {
                    increment_temp_duration();
                    setTimer(TIMER_REPEAT, AUTO_REPEAT_PERIOD_MS);
                }
            }
            break;

        case BUTTON_PRESSED_MORE_THAN_1_SECOND:
            if (!is_button_pressed(BTN_MODIFY))
            {
                buttonState[BTN_MODIFY] = BUTTON_RELEASED;
                clearTimer(TIMER_REPEAT);
            }
            else if (current_mode != MODE_1_NORMAL && isTimerExpired(TIMER_REPEAT))
            {
                setTimer(TIMER_REPEAT, AUTO_REPEAT_PERIOD_MS);
                increment_temp_duration();
            }
            break;
    }
}

/* Button 3 (PB3): Set / confirm duration value */
static void process_button_set(void)
{
    switch (buttonState[BTN_SET])
    {
        case BUTTON_RELEASED:
            if (is_button_pressed(BTN_SET))
            {
                buttonState[BTN_SET] = BUTTON_PRESSED;
                if (current_mode != MODE_1_NORMAL)
                {
                    commit_temp_duration();
                }
            }
            break;

        case BUTTON_PRESSED:
            if (!is_button_pressed(BTN_SET))
            {
                buttonState[BTN_SET] = BUTTON_RELEASED;
            }
            else if (is_button_pressed_1s(BTN_SET))
            {
                buttonState[BTN_SET] = BUTTON_PRESSED_MORE_THAN_1_SECOND;
            }
            break;

        case BUTTON_PRESSED_MORE_THAN_1_SECOND:
            if (!is_button_pressed(BTN_SET))
            {
                buttonState[BTN_SET] = BUTTON_RELEASED;
            }
            break;
    }
}

void fsm_init(void)
{
    enter_mode_1();
}

void fsm_for_input_processing(void)
{
    process_button_mode();
    process_button_modify();
    process_button_set();

    if (current_mode == MODE_1_NORMAL)
    {
        run_normal_traffic_fsm();
    }
    else
    {
        run_modify_blink_fsm();
    }
}
