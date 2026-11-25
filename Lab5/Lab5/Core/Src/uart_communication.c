/*
 * uart_communication.c
 *
 *  Created on: Nov 25, 2026
 *      Author: asus
 */


#include "uart_communication.h"
#include "command_parser.h"
#include "software_timer.h"
#include <stdio.h>

typedef enum {
    COMM_IDLE = 0,
    COMM_SEND_ADC,
    COMM_WAIT_OK
} CommState;

static CommState comm_state = COMM_IDLE;
static uint32_t adc_value = 0;
static char tx_buf[32];

void uart_communication_fsm(ADC_HandleTypeDef *hadc, UART_HandleTypeDef *huart) {
    switch (comm_state) {
    case COMM_IDLE:
        if (command_flag == 1) {
            command_flag = 0;
            if (command_data == CMD_RST) {
                HAL_ADC_Start(hadc);
                if (HAL_ADC_PollForConversion(hadc, 100) == HAL_OK) {
                    adc_value = HAL_ADC_GetValue(hadc);
                }
                HAL_ADC_Stop(hadc);
                comm_state = COMM_SEND_ADC;
            }
        }
        break;

    case COMM_SEND_ADC: {
        int len = sprintf(tx_buf, "\r\n!ADC=%lu#\r\n", (unsigned long)adc_value);
        HAL_UART_Transmit(huart, (uint8_t *)tx_buf, (uint16_t)len, 1000);
        setTimer(0, 3000);  // Timeout 3 giay cho !OK#
        comm_state = COMM_WAIT_OK;
        break;
    }

    case COMM_WAIT_OK:
        if (command_flag == 1) {
            command_flag = 0;
            if (command_data == CMD_OK) {
                comm_state = COMM_IDLE;
            }
        } else if (isTimerExpired(0)) {
            // Het 3s chua co !OK# -> gui lai DUNG gia tri adc_value cu
            comm_state = COMM_SEND_ADC;
        }
        break;

    default:
        comm_state = COMM_IDLE;
        break;
    }
}
