#include "game.h"
#include "sudoku.h"
#include "joystick.h"
#include "hub75.h"
#include "keypad.h"
#include "audio.h"
#include "oled.h"
#include "eeprom.h"
#include "pico/multicore.h"
#include <stdio.h>

int main() {
    stdio_init_all();

    printf("\n\n========= Chroma Sudoku =========\n\n");
    printf("[>] Initializing hardware...\n");

    printf("    [>] Initializing audio:     "); audio_init(); printf("ok\n");
    printf("    [>] Initializing oled:      "); oled_init(); printf("ok\n");
    printf("    [>] Initializing eeprom:    "); eeprom_init(); printf("ok\n");
    printf("    [>] Initializing keypad:    "); keypad_init(); printf("ok\n");
    printf("    [>] Initializing hub75:     "); hub75_init(); printf("ok\n");
    printf("    [>] Initializing joystick   "); joystick_init(); printf("ok\n");
    printf("[+] Hardware ok\n\n");

    printf("[>] Seeding gamestate...\n");
    printf("    [>] Initializing game:      "); game_init(); printf("ok\n");
    printf("[+] Gamestate ok\n\n");

    multicore_launch_core1(hub75_spin);
    audio_stop();
    oled_splash();

    printf("[+] Entering game loop\n");
    while (1) {
        game_update();
    }

    return 0;
}
