#ifndef KEYPAD_H_CF5C5FBB219A82A0
#define KEYPAD_H_CF5C5FBB219A82A0

#include <stdint.h>
#include <stdbool.h>

void keypad_init();
uint16_t keypad_get_event();

char keypad_get_char(uint16_t event);
bool keypad_is_pressed(uint16_t event);
bool keypad_is_key_held(char key);

#endif // KEYPAD_H_CF5C5FBB219A82A0
