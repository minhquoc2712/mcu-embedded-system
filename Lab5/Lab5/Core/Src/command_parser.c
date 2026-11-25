/*
 * command_parser.c
 *
 *  Created on: Nov 25, 2026
 *      Author: asus
 */


#include "command_parser.h"
#include <string.h>

#define CMD_BUF_SIZE 16

typedef enum {
    PARSER_WAIT_START = 0,
    PARSER_READ_CMD
} ParserState;

uint8_t temp = 0;
volatile uint8_t buffer[MAX_BUFFER_SIZE];
volatile uint8_t index_buffer = 0;
volatile uint8_t buffer_flag = 0;

uint8_t command_flag = 0;
CommandType command_data = CMD_NONE;

static ParserState parser_state = PARSER_WAIT_START;
static uint8_t read_index = 0;
static char cmd_buf[CMD_BUF_SIZE];
static uint8_t cmd_len = 0;

void command_parser_fsm(void) {
    while (read_index != index_buffer) {
        uint8_t ch = buffer[read_index++];
        if (read_index >= MAX_BUFFER_SIZE) {
            read_index = 0;
        }

        switch (parser_state) {
        case PARSER_WAIT_START:
            if (ch == '!') {
                cmd_len = 0;
                parser_state = PARSER_READ_CMD;
            }
            break;

        case PARSER_READ_CMD:
            if (ch == '!') {
                cmd_len = 0;  // Xu ly truong hop go nham kieu !ab!RST#
            } else if (ch == 0x08 || ch == 0x7F) {
                if (cmd_len > 0) {
                    cmd_len--;  // Ho tro phim Backspace neu go nham
                }
            } else if (ch == '\r' || ch == '\n') {
                // Bo qua phim Enter
            } else if (ch == '#') {
                cmd_buf[cmd_len] = '\0';
                if (strcmp(cmd_buf, "RST") == 0) {
                    command_data = CMD_RST;
                    command_flag = 1;
                } else if (strcmp(cmd_buf, "OK") == 0) {
                    command_data = CMD_OK;
                    command_flag = 1;
                }
                parser_state = PARSER_WAIT_START;
            } else {
                if (cmd_len < CMD_BUF_SIZE - 1) {
                    cmd_buf[cmd_len++] = (char)ch;
                } else {
                    parser_state = PARSER_WAIT_START;
                }
            }
            break;

        default:
            parser_state = PARSER_WAIT_START;
            break;
        }
    }
}
