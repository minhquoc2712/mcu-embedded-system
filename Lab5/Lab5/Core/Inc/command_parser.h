/*
 * command_parser.h
 *
 *  Created on: Nov 25, 2026
 *      Author: asus
 */

#ifndef INC_COMMAND_PARSER_H_
#define INC_COMMAND_PARSER_H_

#include <stdint.h>
#define MAX_BUFFER_SIZE 30
typedef enum {
    CMD_NONE = 0,
    CMD_RST,
    CMD_OK
} CommandType;
extern uint8_t temp;
extern volatile uint8_t buffer[MAX_BUFFER_SIZE];
extern volatile uint8_t index_buffer;
extern volatile uint8_t buffer_flag;
extern volatile uint8_t command_flag;
extern volatile CommandType command_data;
void command_parser_fsm(void);

#endif /* INC_COMMAND_PARSER_H_ */
