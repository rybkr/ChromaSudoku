#include "joystick.h"
#include "hardware/adc.h"
#include "hardware/gpio.h"
#include "pico/stdlib.h"
#include <stdio.h>

#define JOYSTICK_SW_PIN 39
#define JOYSTICK_X_PIN 41
#define JOYSTICK_Y_PIN 40

static direction_t current_direction = DIRECTION_NONE;

static void init_input_pin(int i) {
    if (i < 32) {
        sio_hw->gpio_oe_clr = 1U << i;
        sio_hw->gpio_clr = 1U << i;
    } else {
        sio_hw->gpio_hi_oe_clr = 1U << (i - 32);
        sio_hw->gpio_hi_clr = 1U << (i - 32);
    }

    hw_write_masked(&pads_bank0_hw->io[i], PADS_BANK0_GPIO0_IE_BITS,
                    PADS_BANK0_GPIO0_IE_BITS | PADS_BANK0_GPIO0_OD_BITS);
    io_bank0_hw->io[i].ctrl = GPIO_FUNC_SIO << IO_BANK0_GPIO0_CTRL_FUNCSEL_LSB;
    hw_clear_bits(&pads_bank0_hw->io[i], PADS_BANK0_GPIO0_ISO_BITS);
    hw_clear_bits(&pads_bank0_hw->io[i], PADS_BANK0_GPIO0_PUE_BITS);
    hw_clear_bits(&pads_bank0_hw->io[i], PADS_BANK0_GPIO0_PDE_BITS);
}

void joystick_init() {
    adc_init();
    adc_gpio_init(JOYSTICK_X_PIN);
    adc_gpio_init(JOYSTICK_Y_PIN);
    gpio_disable_pulls(JOYSTICK_X_PIN);
    gpio_disable_pulls(JOYSTICK_Y_PIN);

    adc_set_round_robin(0x03);

    adc_select_input(0);
    adc_read();
}

uint32_t joystick_get_event() {
    uint32_t x = adc_read();
    uint32_t y = adc_read();
    return ((uint32_t)y << 16) | (uint32_t)x;
}

direction_t joystick_is_pressed(uint32_t event) {
    uint16_t x = (uint16_t)(event & 0xFFFF);
    uint16_t y = (uint16_t)(event >> 16);
    unsigned direction = 0;

    if (x > 0xF00) {
        direction |= (unsigned)DIRECTION_E;
    } else if (x < 0xFF) {
        direction |= (unsigned)DIRECTION_W;
    }

    if (y > 0xF00) {
        direction |= (unsigned)DIRECTION_N;
    } else if (y < 0xFF) {
        direction |= (unsigned)DIRECTION_S;
    }

    if (current_direction != DIRECTION_NONE) {
        if (direction == DIRECTION_NONE) {
            current_direction = DIRECTION_NONE;
        } else {
            direction = DIRECTION_NONE;
        }
    } else {
        current_direction = direction;
    }

    return (direction_t)direction;
}
