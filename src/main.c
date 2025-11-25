#include "game.h"
#include "sudoku.h"
#include "hub75.h"
#include "keypad.h"
#include "audio.h"
#include "display.h"
#include "display2.h"
#include "eeprom.h"
#include <stdio.h>

int main() {
    stdio_init_all();

    printf("\n\n========= Chroma Sudoku =========\n\n");

    printf("[>] Initializing hardware...\n");
    printf("    [>] Initializing audio:     "); audio_init(); printf("ok\n");
    printf("    [>] Initializing display:   "); display_init(); printf("ok\n");
    printf("    [>] Initializing display2:  "); display2_init(); printf("ok\n");
    printf("    [>] Initializing eeprom:    "); eeprom_init(); printf("ok\n");
    printf("    [>] Initializing keypad:    "); keypad_init(); printf("ok\n");
    printf("    [>] Initializing hub75:     "); hub75_init(); printf("ok\n");
    printf("[+] Hardware ok\n\n");

    printf("[>] Seeding gamestate...\n");
    printf("    [>] Initializing game:      "); game_init(); printf("ok\n");
    printf("[+] Gamestate ok\n\n");

    display_show_splash();
    display2_show_splash();

    printf("[+] Entering game loop\n");
    while (true) {
        game_update();
    }

    return 0;
}
