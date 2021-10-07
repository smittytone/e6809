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


uint16_t    lit_buttons = 0;
uint8_t     mode = 0;
uint8_t     old_mode = 0;
bool        mode_changed = false;
uint8_t     input_count = 0;
uint16_t    input_value = 0;
uint16_t    input_valid = 0;

uint16_t    current_address = 0x0000;


int main() {

    // Enable STDIO
    stdio_init_all();
    setup_cc_leds();

    // Boot the CPU
    boot();

    // Run tests
    //test_main();

    // Enter UI
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

    bool key_pressed = false;
    bool key_can_release = false;
    uint32_t debounce_count_press = 0;
    uint32_t debounce_count_release = 0;
    uint16_t the_key = 0;

    uint32_t loop_count = 0;
    uint32_t rh = 0;
    uint32_t rl = 0;

    keypad_init();
    keypad_set_brightness(0.2f);
    set_keys();

    while (true) {

        uint32_t now = time_us_32();
        uint16_t any_key = keypad_get_button_states();
        key_pressed = (any_key != 0);

        if (key_pressed) {
            if (debounce_count_press == 0) {
                debounce_count_press = now;
            } else if (now - debounce_count_press > DEBOUNCE_TIME_US) {
                debounce_count_press == 0;
                key_can_release = true;
                if (the_key == 0) the_key = any_key;
            }
        } else if (key_can_release) {
            if (debounce_count_press == 0) {
                debounce_count_press = now;
            } else if (now - debounce_count_press > DEBOUNCE_TIME_US) {
                key_can_release = false;
                process_key(the_key);
                the_key = 0;
            }
        }
    }
}


void process_key(uint16_t input) {
    input &= input_valid;
    if (input != 0) {
        switch (mode) {
            case 1:
            case 2:
                input_value = (input_value << 4) | get_val(input);
                input_count -= 1;
                if (input_count == 0) {
                    old_mode = mode;
                    mode = 3;
                    mode_changed = true;
                }
                break;
            case 3:
                if (input == 0x01) {
                    // Pass value
                    if (old_mode == 2) {
                        current_address = input_value;
                    }

                    if (old_mode == 1) {
                        mem[current_address] = input_value;
                        current_address++;
                    }
                }

                mode = 0;
                mode_changed = true;
                break;
            default:
                // Main menu
                // 0 -- Input 8-bit value to current memory location
                // 1 -- Input 16-bit value to set current memory location
                // 2 -- Run code at current memory
                // 3 -- Run code at current memory, single stepping
                if (input == 0x01) {
                    mode = 1;
                    mode_changed = true;
                }

                if (input == 0x02) {
                    mode = 2;
                    mode_changed = true;
                }

                if (input == 0x03) {
                    mode = 2;
                    mode_changed = true;
                }

                if (input == 0x04) {
                    mode = 3;
                    mode_changed = true;
                }
        }
    }

    if (mode_changed) set_keys();
}


void set_keys() {
    input_value = 0;

    // Clear the keyboard
    keypad_clear();
    keypad_update_leds();

    switch (mode) {
        case 1:
            // Enter 16-bit address: all keys yellow
            keypad_set_all(0x10, 0x10, 0x00);
            input_count = 4;
            input_valid = 0xFFFF;
            break;
        case 2:
            // Enter 8-bit data: all keys cyan
            keypad_set_all(0x00, 0x10, 0x10);
            input_count = 2;
            input_valid = 0xFFFF;
            break;
        case 3:
            // Enter or cancel input
            keypad_set_led(0, 0x00, 0x10, 0x00);
            keypad_set_led(3, 0x10, 0x00, 0x00);
            input_count = 1;
            input_valid = 0x0009;
            break;
        default:
            // Enter action: lower keys green
            keypad_set_led(0, 0x00, 0x10, 0x00);
            keypad_set_led(1, 0x00, 0x10, 0x00);
            keypad_set_led(2, 0x00, 0x10, 0x00);
            keypad_set_led(3, 0x00, 0x10, 0x00);
            input_count = 1;
            input_valid = 0x000F;
            break;
    }

    keypad_update_leds();
    mode_changed = false;
}

uint8_t get_val(uint16_t input) {
    for (uint32_t i = 0 ; i < 16 ; ++i) {
        if ((input & (1 << i)) != 0) {
            return i;
        }
    }

    return 0;
}

uint16_t inkey() {
    // Wait for any button to be pushed, using debounce
    bool is_pressed = false;
    bool was_pressed = false;
    uint32_t debounce_count_press = 0;
    uint32_t debounce_count_release = 0;

    while (true) {

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