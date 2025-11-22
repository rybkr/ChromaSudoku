#include "keypad.h"
#include "hardware/gpio.h"
#include "hardware/irq.h"
#include "hardware/timer.h"
#include "pico/stdlib.h"

#define ROW1_PIN 37
#define ROW2_PIN 36
#define ROW3_PIN 35
#define ROW4_PIN 34

#define COL1_PIN 33
#define COL2_PIN 32
#define COL3_PIN 31
#define COL4_PIN 30

const char keymap[17] = "DCBA#9630852*741";

typedef struct {
    uint16_t q[32];
    uint16_t head;
    uint16_t tail;
} KeyEvents;

static KeyEvents kev = {.head = 0, .tail = 0};
static volatile int col = -1;
static bool state[16];

static const uint8_t row_pins[4] = {ROW1_PIN, ROW2_PIN, ROW3_PIN, ROW4_PIN};
static const uint8_t col_pins[4] = {COL1_PIN, COL2_PIN, COL3_PIN, COL4_PIN};

static void init_output_pin(int i) {
    if (i < 32) {
        sio_hw->gpio_oe_clr = 1U << i;
        sio_hw->gpio_clr = 1U << i;
    } else {
        sio_hw->gpio_hi_oe_clr = 1U << (i - 32);
        sio_hw->gpio_oe_clr = 1U << (i - 32);
    }

    hw_write_masked(&pads_bank0_hw->io[i], PADS_BANK0_GPIO0_IE_BITS,
                    PADS_BANK0_GPIO0_IE_BITS | PADS_BANK0_GPIO0_OD_BITS);
    io_bank0_hw->io[i].ctrl = GPIO_FUNC_SIO << IO_BANK0_GPIO0_CTRL_FUNCSEL_LSB;
    hw_clear_bits(&pads_bank0_hw->io[i], PADS_BANK0_GPIO0_ISO_BITS);

    if (i < 32) {
        sio_hw->gpio_oe_set = 1U << i;
    } else {
        sio_hw->gpio_hi_oe_set = 1U << (i - 32);
    }
}

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
    // No pull-ups: rows should be LOW normally, go HIGH when key pressed
    hw_clear_bits(&pads_bank0_hw->io[i], PADS_BANK0_GPIO0_PUE_BITS);
    hw_clear_bits(&pads_bank0_hw->io[i], PADS_BANK0_GPIO0_PDE_BITS);
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
        if (sio_hw->gpio_hi_in & (1U << (row_pins[i] - 32))) {
            result |= (1U << i);
        }
    }
    return result;
}

void keypad_drive_column(void) {
    // Clear interrupt flag
    timer_hw->intr = 1U << 0;

    // Check for key releases on previous column before switching
    // Read row states to detect keys that were pressed but are now released
    const uint8_t hi_rows = keypad_read_rows();
    for (int row = 0; row < 4; row++) {
        const int key_idx = (col << 2) + row;

        // Key released: row reads LOW (disconnected from HIGH column) when
        // previously pressed
        if (!((1U << row) & hi_rows) && state[key_idx]) {
            state[key_idx] = 0;
            const char key_char = keymap[key_idx];
            const uint16_t event = (uint8_t)key_char;
            key_push(event);
        }
    }

    // Clear all column pins (set LOW)
    for (int i = 0; i < 4; i++) {
        if (col_pins[i] < 32) {
            sio_hw->gpio_clr = 1U << col_pins[i];
        } else {
            sio_hw->gpio_hi_clr = 1U << (col_pins[i] - 32);
        }
    }

    // Advance to next column
    col = (col + 1) & 3U;

    // Set the active column pin HIGH
    // When a key is pressed, it connects HIGH column to LOW row,
    // causing the row to read HIGH (rising edge)
    if (col_pins[col] < 32) {
        sio_hw->gpio_set = 1U << col_pins[col];
    } else {
        sio_hw->gpio_hi_set = 1U << (col_pins[col] - 32);
    }

    timer_hw->alarm[0] = timer_hw->timerawl + 25000;
}

void keypad_isr(void) {
    // GPIO interrupt on row pins - determine which row had the interrupt
    // Match working example pattern: check all row pins for rising edge
    int row = -1;

    // Check which row pin triggered the interrupt (rising edge)
    for (int i = 0; i < 4; i++) {
        uint32_t event_mask = gpio_get_irq_event_mask(row_pins[i]);
        if (event_mask & GPIO_IRQ_EDGE_RISE) {
            row = i;
            break;
        }
    }

    // If we found a row that triggered, process the key press
    if (row >= 0 && row <= 3) {
        const int key_idx = (col << 2) + row;
        const char key_char = keymap[key_idx];

        // Only process if this key wasn't already pressed (debounce)
        if (!state[key_idx]) {
            state[key_idx] = 1;
            const uint16_t event = (1U << 8) | (uint8_t)key_char;
            key_push(event);
        }

        // Acknowledge the interrupt for this row pin
        gpio_acknowledge_irq(row_pins[row], GPIO_IRQ_EDGE_RISE);
    }

    // Acknowledge any other row pin interrupts that might have fired
    for (int i = 0; i < 4; i++) {
        uint32_t event_mask = gpio_get_irq_event_mask(row_pins[i]);
        if (event_mask & (GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL)) {
            gpio_acknowledge_irq(
                row_pins[i],
                (event_mask & GPIO_IRQ_EDGE_RISE ? GPIO_IRQ_EDGE_RISE : 0) |
                    (event_mask & GPIO_IRQ_EDGE_FALL ? GPIO_IRQ_EDGE_FALL : 0));
        }
    }
}

void keypad_init(void) {
    // Initialize column pins as outputs (drive keypad columns)
    for (int i = 0; i < 4; i++) {
        init_output_pin(col_pins[i]);
    }

    // Initialize row pins as inputs (read keypad rows)
    for (int i = 0; i < 4; i++) {
        init_input_pin(row_pins[i]);
    }

    // Set up GPIO interrupts on row pins for rising edge (key press detection)
    gpio_add_raw_irq_handler_masked64((1ULL << row_pins[0]) |
                                      (1ULL << row_pins[1]) |
                                      (1ULL << row_pins[2]) |
                                      (1ULL << row_pins[3]),
                                      keypad_isr);

    // Enable rising edge interrupts on all row pins (like working example)
    // Rising edge = key pressed (row goes HIGH when key connects HIGH column)
    for (int i = 0; i < 4; i++) {
        gpio_set_irq_enabled(row_pins[i], GPIO_IRQ_EDGE_RISE, 1);
    }

    // Enable GPIO bank 0 interrupts
    irq_set_enabled(IO_IRQ_BANK0, 1);

    // Enable alarm 0 interrupt (drives columns) - use timer to scan columns
    hw_set_bits(&timer_hw->inte, 1U << 0);
    irq_set_exclusive_handler(TIMER0_IRQ_0, keypad_drive_column);
    irq_set_enabled(TIMER0_IRQ_0, 1);

    // Start scanning: drive first column after 1ms
    timer_hw->alarm[0] = timer_hw->timerawl + 1000000; // 1ms delay to start
}

uint16_t keypad_get_event(void) { return key_pop(); }

char keypad_get_char(uint16_t event) { return (char)(event & 0xFF); }

bool keypad_is_pressed(uint16_t event) { return (event & 0x100) != 0; }

bool keypad_is_key_held(char key) {
    for (int i = 0; i < 16; i++) {
        if (keymap[i] == key && state[i]) {
            return true;
        }
    }
    return false;
}
