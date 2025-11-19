#ifndef KEYPAD_H
#define KEYPAD_H

#include <stdint.h>
#include <stdbool.h>

// Keypad functions
void keypad_init(void);
uint16_t keypad_get_event(void);
char keypad_get_char(uint16_t event);
bool keypad_is_pressed(uint16_t event);

#endif // KEYPAD_H
