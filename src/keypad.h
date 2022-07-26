#ifndef KEYPAD_H_
#define KEYPAD_H_

#include <stdint.h>

void initialize_keypad();
int is_quitting();
int is_keydown(uint8_t keycode);
int get_key();

#endif  // KEYPAD_H_
