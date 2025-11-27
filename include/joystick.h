#ifndef JOYSTICK_H_CB9C1A5B6910FB2F
#define JOYSTICK_H_CB9C1A5B6910FB2F

#include <stdint.h>
#include <stdbool.h>

typedef enum {
    DIRECTION_NONE = 0,

    DIRECTION_N = 1U << 0,
    DIRECTION_E = 1U << 1,
    DIRECTION_S = 1U << 2,
    DIRECTION_W = 1U << 3,

    DIRECTION_NE = DIRECTION_N | DIRECTION_E,
    DIRECTION_SE = DIRECTION_S | DIRECTION_E,
    DIRECTION_NW = DIRECTION_N | DIRECTION_W,
    DIRECTION_SW = DIRECTION_S | DIRECTION_W,
} direction_t;

void joystick_init();
uint32_t joystick_get_event();

direction_t joystick_is_pressed(uint32_t event);

#endif // JOYSTICK_H_CB9C1A5B6910FB2F
