/*
 * e6809 for Raspberry Pi Pico
 *
 * @version     1.0.0
 * @author      smittytone
 * @copyright   2021
 * @licence     MIT
 *
 */
#include "main.h"


int main() {

    // Enable STDIO
    stdio_init_all();

    boot();
    loop();
    return 0;
}


void boot() {
    reset_registers();

    for (uint16_t i = 0 ; i < 65536 ; i++) mem[i] = 0x00;

    uint16_t start = 32767;
    uint8_t prog[] = {0x82, 0x04, 0x00, 0x86, 0x41, 0xA7, 0x80, 0x8C, 0x06, 0x00, 0x26, 0xF9, 0x39};

    for (uint16_t i = 0 ; i < 14 ; i++) mem[start + i] = prog[i];

    reg.pc = start;
}


void loop() {

    uint32_t loop_count = 0;

    while(1) {
        uint32_t cycles = process_next_instruction();

        printf("Loop count: %i", loop_count);
        dump_registers();

        // Done!
        if (cycles == 99) break;

        loop_count++;

        // Wait for a key press
        inkey();
    }
}


void inkey() {
    // Wait for any button to be pushed, using debounce
    bool is_pressed = false;
    bool was_pressed = false;
    uint32_t debounce_count_press = 0;
    uint32_t debounce_count_release = 0;

    while (true) {
        uint32_t now = time_us_32();

        if (gpio_get(PIN_STEP_BUTTON)) {
            is_pressed = true;
        } else {
            is_pressed = false;
        }

        if (is_pressed) {
            if (debounce_count_press == 0) {
                debounce_count_press = now;
            } else if (now - debounce_count_press > DEBOUNCE_TIME_US) {
                debounce_count_press == 0;
                was_pressed = true;
            }
        } else if (was_pressed) {
            if (debounce_count_press == 0) {
                debounce_count_press = now;
            } else if (now - debounce_count_press > DEBOUNCE_TIME_US) {
                break;
            }
        }
    }
}


void dump_registers() {
    printf("A [%02x] B [%02x] DP [%02x]\n", reg.a, reg.b, reg.dp);
    printf("X [%04x] Y [%04x] S [%04x] U [%04x] PC [%04x]\n", reg.x, reg.y, reg.u, reg.pc);
    printf("    E  F  H  I  N  Z  V  C\n");
    char *cc_oop = "CC [ ][ ][ ][ ][ ][ ][ ][ ]\n";
    char *delta = cc_oop;
    for (int32_t i = 7 ; i > -1 ; i--) {
        delta += 3;
        *delta = is_cc_bit_set(i) ? '1' : '0';
    }
    printf("%s\n", cc_oop);
    printf("--------------------------------------------------\n");
}
