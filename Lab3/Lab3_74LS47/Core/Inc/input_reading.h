/*
 * input_reading.h
 *
 * Lab 3 - Section 4.3.2 & Exercise 5: Button Reading and Debouncing
 */

#ifndef INC_INPUT_READING_H_
#define INC_INPUT_READING_H_

void button_reading(void);
unsigned char is_button_pressed(unsigned char index);
unsigned char is_button_pressed_1s(unsigned char index);

#endif /* INC_INPUT_READING_H_ */
