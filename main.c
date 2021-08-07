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
    setup_cc_leds();
    boot();
    loop();
    return 0;
}


void boot() {
    printf("Booting...");
    reset_registers();

    for (uint32_t i = 0 ; i < 65536 ; i++) {
        mem[i] = RTS;
    }

    uint16_t start = 0x8000;
    uint8_t prog[] = {0x34,0x46,0x33,0x64,0xA6,0x42,0xAE,0x43,0xE6,0x80,0x34,0x04,0x34,0x04,0x4A,0x27,0x13,0xE6,0x80,0xE1,0xE4,0x2E,0x08,0xE1,0x61,0x2E,0x06,0xE7,0x61,0x20,0x02,0xE7,0xE4,0x4A,0x26,0xED,0xA6,0xE0,0xA7,0x45,0xA6,0xE0,0xA7,0x46,0x35,0xC6,0x32,0x7E,0x8E,0x80,0x42,0x34,0x10,0xB6,0x80,0x41,0x34,0x02,0x8D,0xC4,0xA6,0x63,0xE6,0x64,0x39,0x0A,0x01,0x02,0x03,0x04,0x00,0x06,0x07,0x09,0x08,0x0B};
    // 0x86, 0x41, 0x8E, 0x04, 0x00, 0xA7, 0x80, 0x8C, 0x06, 0x00, 0x26, 0xF9, 0x1A, 0x0F, 0x39

    for (uint16_t i = 0 ; i < 76 ; i++) mem[start + i] = prog[i];

    reg.pc = 0x802E;
}


void loop() {
    printf("Looping...");
    uint32_t loop_count = 0;
    uint32_t rh = 0;
    uint32_t rl = 0;

    while(1) {
        uint32_t cycles = process_next_instruction();

        printf("Loop count: %i", loop_count);
        dump_registers();

        // Done!
        if (cycles == 99) {
            break;
        }

        loop_count++;
        //sleep_ms(30);
        // Wait for a key press
        // inkey();
    }

    rh = mem[0xFFFE];
    rl = mem[0xFFFF];
    printf("Done");
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


void setup_cc_leds() {
    gpio_init(PIN_LED_C);
    gpio_set_dir(PIN_LED_C, GPIO_OUT);
    gpio_put(PIN_LED_C, false);

    gpio_init(PIN_LED_V);
    gpio_set_dir(PIN_LED_V, GPIO_OUT);
    gpio_put(PIN_LED_V, false);

    gpio_init(PIN_LED_Z);
    gpio_set_dir(PIN_LED_Z, GPIO_OUT);
    gpio_put(PIN_LED_Z, false);

    gpio_init(PIN_LED_N);
    gpio_set_dir(PIN_LED_N, GPIO_OUT);
    gpio_put(PIN_LED_N, false);
}


void dump_registers() {
    uint8_t gpios[] = {PIN_LED_N, PIN_LED_Z, PIN_LED_V, PIN_LED_C};

    printf("A [%02x] B [%02x] DP [%02x]\n", reg.a, reg.b, reg.dp);
    printf("X [%04x] Y [%04x] S [%04x] U [%04x] PC [%04x]\n", reg.x, reg.y, reg.u, reg.pc);
    printf("    E  F  H  I  N  Z  V  C\n");
    char *cc_oop = "CC [ ][ ][ ][ ][ ][ ][ ][ ]\n";
    char *delta = cc_oop;
    for (int32_t cc_bit = 7 ; cc_bit > -1 ; cc_bit--) {
        delta += 3;
        *delta = is_cc_bit_set(cc_bit) ? '1' : '0';

        if (cc_bit < 4) {
            gpio_put(gpios[3 - cc_bit], is_cc_bit_set(cc_bit));
        }
    }
    printf("%s\n", cc_oop);
    printf("--------------------------------------------------\n");
}
