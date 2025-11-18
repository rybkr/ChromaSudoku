#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/timer.h"
#include "hardware/irq.h"
#include "keypad.h"

#define ROW1_PIN 34
#define ROW2_PIN 35
#define ROW3_PIN 36
#define ROW4_PIN 37

#define COL1_PIN 30
#define COL2_PIN 31
#define COL3_PIN 32
#define COL4_PIN 33

const char keymap[17] = "DCBA#9630852*741";

typedef struct {
    uint16_t q[32];
    uint16_t head;
    uint16_t tail;
} KeyEvents;

static KeyEvents kev = {.head = 0, .tail = 0};
static int col = -1;
static bool state[16];

static const uint8_t row_pins[4] = {ROW1_PIN, ROW2_PIN, ROW3_PIN, ROW4_PIN};
static const uint8_t col_pins[4] = {COL1_PIN, COL2_PIN, COL3_PIN, COL4_PIN};

static void init_output_pin(int i) {
    sio_hw->gpio_oe_clr = 1U << i;
    sio_hw->gpio_clr = 1U << i;
    hw_write_masked(&pads_bank0_hw->io[i], PADS_BANK0_GPIO0_IE_BITS, 
                    PADS_BANK0_GPIO0_IE_BITS | PADS_BANK0_GPIO0_OD_BITS);
    io_bank0_hw->io[i].ctrl = GPIO_FUNC_SIO << IO_BANK0_GPIO0_CTRL_FUNCSEL_LSB;
    hw_clear_bits(&pads_bank0_hw->io[i], PADS_BANK0_GPIO0_ISO_BITS);
    sio_hw->gpio_oe_set = 1U << i;
}

static void init_input_pin(int i) {
    sio_hw->gpio_oe_clr = 1U << i;
    sio_hw->gpio_clr = 1U << i;
    hw_write_masked(&pads_bank0_hw->io[i], PADS_BANK0_GPIO0_IE_BITS, 
                    PADS_BANK0_GPIO0_IE_BITS | PADS_BANK0_GPIO0_OD_BITS);
    io_bank0_hw->io[i].ctrl = GPIO_FUNC_SIO << IO_BANK0_GPIO0_CTRL_FUNCSEL_LSB;
    hw_clear_bits(&pads_bank0_hw->io[i], PADS_BANK0_GPIO0_ISO_BITS);
}

static void key_push(uint16_t value) {
    if ((kev.head + 1) % 32 == kev.tail) {
        return;
    }
    kev.q[kev.head] = value;
    kev.head = (kev.head + 1) % 32;
}

static uint16_t key_pop(void) {
    if (kev.head == kev.tail) {
        return 0;
    }
    uint16_t value = kev.q[kev.tail];
    kev.tail = (kev.tail + 1) % 32;
    return value;
}

static uint8_t keypad_read_rows(void) {
    uint8_t result = 0;
    for (int i = 0; i < 4; i++) {
        if (sio_hw->gpio_in & (1U << row_pins[i])) {
            result |= (1U << i);
        }
    }
    return result;
}

void keypad_drive_column(void) {
    timer_hw->intr = 1U << 0;
    col = (col + 1) & 3U;
    
    // Clear all column pins
    for (int i = 0; i < 4; i++) {
        sio_hw->gpio_clr = 1U << col_pins[i];
    }
    
    // Set the active column pin
    sio_hw->gpio_set = 1U << col_pins[col];
    
    timer_hw->alarm[0] = timer_hw->timerawl + 25000;
}

void keypad_isr(void) {
    timer_hw->intr = 1U << 1;
    timer_hw->alarm[1] = timer_hw->timerawl + 25000;
    
    const uint8_t hi_rows = keypad_read_rows();
    
    for (int row = 0; row <= 3; ++row) {
        const int key_idx = (col << 2) + row;
        const char key_char = keymap[key_idx];
        
        if (((1U << row) & hi_rows) && !state[key_idx]) {
            state[key_idx] = 1;
            const uint16_t event = (1U << 8) | (uint8_t)key_char;
            key_push(event);
        } else if (!((1U << row) & hi_rows) && state[key_idx]) {
            state[key_idx] = 0;
            const uint16_t event = (uint8_t)key_char;
            key_push(event);
        }
    }
}

void keypad_init(void) {
    // Initialize column pins as outputs
    for (int i = 0; i < 4; i++) {
        init_output_pin(col_pins[i]);
    }
    
    // Initialize row pins as inputs
    for (int i = 0; i < 4; i++) {
        init_input_pin(row_pins[i]);
    }
    
    hw_set_bits(&timer_hw->inte, 1U << 0);
    irq_set_exclusive_handler(TIMER0_IRQ_0, keypad_drive_column);
    irq_set_enabled(TIMER0_IRQ_0, 1);
    timer_hw->alarm[0] = timer_hw->timerawl + 1000000;
    
    hw_set_bits(&timer_hw->inte, 1U << 1);
    irq_set_exclusive_handler(TIMER0_IRQ_1, keypad_isr);
    irq_set_enabled(TIMER0_IRQ_1, 1);
    timer_hw->alarm[1] = timer_hw->timerawl + 1100000;
}

uint16_t keypad_get_event(void) {
    return key_pop();
}

char keypad_get_char(uint16_t event) {
    return (char)(event & 0xFF);
}

bool keypad_is_pressed(uint16_t event) {
    return (event & 0x100) != 0;
}
