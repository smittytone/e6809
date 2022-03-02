/*
 * e6809 for Raspberry Pi Pico
 * Monitor code
 *
 * @version     1.0.0
 * @author      smittytone
 * @copyright   2022
 * @licence     MIT
 *
 */
#include "main.h"


/*
 *      GLOBALS
 */
uint8_t     display_mode = 0;
uint8_t     input_count = 0;
uint16_t    input_value = 0;
uint16_t    input_mask = 0;
uint16_t    mode = 0;
uint16_t    previous_mode = 0;
uint16_t    current_address = 0x0000;
uint16_t    start_address = 0x0000;
bool        mode_changed = false;
bool        do_display_pc = true;
bool        is_running_steps = false;
bool        is_running_full = false;
bool        led_state = false;
uint8_t     buffer[32];
uint8_t    *display_buffer[2] = {buffer, buffer + 16};
uint8_t     display_address[2] = {0x71, 0x70};


/**
    Bring up the monitor board if it is present.

    - Returns: `true` if the board is present and enabled, otherwise `false`.
 */
bool init_board() {
    #ifdef DEBUG
    printf("Configuring the monitor board\n");
    #endif

    // Set up the keypad -- this sets up I2C0 @ 400,000bps
    if (!keypad_init()) {
        #ifdef DEBUG
        printf("No monitor board found\n");
        #endif
        return false;
    }

    // Turn down the key LED glare
    keypad_set_brightness(0.2);

    // Set up the displays
    ht16k33_init(display_address[0], display_buffer[0]);
    ht16k33_init(display_address[1], display_buffer[1]);
    return true;
}


/**
    Initialise and run the main event loop. This primarily continually reads the
    keypad, allowing for debounces on press and release actions. Buttons are
    triggered only on release.
 */
void monitor_event_loop() {
    #ifdef DEBUG
    printf("Entering UI at main menu\n");
    #endif

    bool is_key_pressed = false;
    bool can_key_release = false;

    uint32_t debounce_count_press = 0;
    uint32_t debounce_count_release = 0;
    uint32_t cpu_cycle_complete = 0;

    uint16_t the_key = 0;

    uint8_t input_buffer[10];
    uint8_t* buffer_ptr = input_buffer;
    uint8_t buffer_index = 0;
    const char test_text[] = "HAIL";


    // Set the button colours and the display
    set_keys();
    update_display();

    // Run the button press loop
    while (true) {
        // Read the keypad
        uint32_t now = time_us_32();
        uint16_t any_key = keypad_get_button_states();
        is_key_pressed = (any_key != 0);
        state.interrupts = sample_interrupts();

        if (now - cpu_cycle_complete > 250000) {
            cpu_cycle_complete = now;
            led_state = !led_state;
        }

        if (is_running_full) {
            // Execute the next instruction
            gpio_put(PIN_PICO_LED, led_state);
            uint32_t result = process_next_instruction();

            // Update the display
            update_display();

            if (result == BREAK_TO_MONITOR) {
                // Code hit RTI -- show we're not running
                // NOTE A key press then will take the
                //      user to the main menu
                mode = MENU_MODE_RUN_DONE;
                mode_changed = true;
                is_running_full = false;
                set_keys();
            }
        }

        if (is_key_pressed) {
            // Allow a debounce period, eg. 20ms
            if (debounce_count_press == 0) {
                debounce_count_press = now;
            } else if (now - debounce_count_press > DEBOUNCE_TIME_US) {
                // Key still pressed -- start to check for its release
                debounce_count_press == 0;
                can_key_release = true;
                if (the_key == 0) the_key = any_key;
            }
        } else if (can_key_release) {
            // Allow a debounce period, eg. 20ms
            if (debounce_count_press == 0) {
                debounce_count_press = now;
            } else if (now - debounce_count_press > DEBOUNCE_TIME_US) {
                // Key released -- check and action it
                can_key_release = false;
                process_key(the_key);
                the_key = 0;
            }
        } else {
            // pause to allow room for interrupts
            // TODO
        }
    }
}


/**
    Process a key press to determine if it is valid - a lit button was pressed -
    and to then trigger the action the key represents.

    - Parameters:
        - input: The key press value read from the keypad.
 */
void process_key(uint16_t input) {
    // Make sure the key pressed was a valid one
    input &= input_mask;
    if (input != 0) {
        // Check the key's action according to the menu mode
        bool show_on_completion = false;
        switch (mode) {
            case MENU_MAIN_ADDR:
            case MENU_MAIN_BYTE:
                // 8- and 16-bit value input -- determined by 'input_count'
                input_value = (input_value << 4) | keypress_to_value(input);
                input_count -= 1;

                if (mode == MENU_MAIN_BYTE) {
                    display_left(current_address);
                    display_right(input_value);
                } else {
                    display_left(input_value);
                }

                if (input_count == 0) {
                    // Got 2 or 4 presses -- jump to confirm menu
                    previous_mode = mode;
                    mode = MENU_MODE_CONFIRM;
                    mode_changed = true;
                }

                break;
            case MENU_MODE_STEP:
                // Single-step run menu
                // 0 -- Memory stop down                                   -- BLUE
                // 3 -- Memory step up                                     -- BLUE
                // C -- Run next instruction                               -- GREEN
                // D -- Toggle the display between address and CC register -- MAGENTA
                // E -- Track the PC register on the display               -- ORANGE
                // F -- Exit to main menu                                  -- RED
                if (input == INPUT_STEP_NEXT && is_running_steps) {
                    // Run next instruction
                    uint32_t result = process_next_instruction();

                    if (result == BREAK_TO_MONITOR) {
                        // Code hit RTI -- jump back to the main menu
                        mode = MENU_MODE_MAIN;
                        mode_changed = true;
                        is_running_steps = false;
                        current_address = start_address;
                    } else {
                        if (do_display_pc) current_address = reg.pc;
                    }

                    if (result == 99) show_on_completion = true;
                }

                if (input == INPUT_STEP_SHOW_CC) {
                    // Toggle the display between modes:
                    // 0 - Address : value
                    // 1 - CC register (binary)
                    // 2 - DP : A, B (D) registers
                    // 3 - X : Y registers
                    // 4 - S : U registers
                    display_mode++;
                    if (display_mode > 4) display_mode = 0;
                }

                if (input == INPUT_STEP_SHOW_AD) {
                    // Jump to current value of PC register
                    do_display_pc = true;
                    current_address = reg.pc;
                }

                if (input == INPUT_STEP_MEM_UP || input == INPUT_STEP_MEM_DOWN) {
                    // Step up and down from the current memory value
                    // NOTE This will 'fix' the memory displayed -- hit the Blue
                    //      key in the menu to continue tracking the PC register
                    current_address += (input == INPUT_STEP_MEM_UP ? 1 : -1);
                    do_display_pc = false;
                }

                if (input == INPUT_STEP_EXIT) {
                    // User hit Cancel -- go back to the main menu
                    mode = MENU_MODE_MAIN;
                    mode_changed = true;
                    is_running_steps = false;
                    current_address = start_address;
                }

                update_display();
                if (show_on_completion) {
                    // If the code completed, add a long minus sign to the right display
                    ht16k33_set_glyph(display_address[1], display_buffer[1], 0x40, 0, false);
                    ht16k33_set_glyph(display_address[1], display_buffer[1], 0x40, 1, false);
                    ht16k33_draw(display_address[1], display_buffer[1]);
                }
                break;
            case MENU_MODE_CONFIRM:
                // Confirm value entry menu
                // C -- Reject value and return to main menu  -- RED
                // E -- Accept value and return to data entry -- ORANGE
                // E -- Switch display                        -- MAGENTA
                // F -- Accept value and return to main menu  -- GREEN

                // There's always a mode change
                mode = MENU_MODE_MAIN;
                mode_changed = true;

                if (input == INPUT_CONF_OK) {
                    // User hit OK -- results depend on which menu mode
                    // they came from
                    if (previous_mode == MENU_MAIN_ADDR) {
                        current_address = input_value;
                    }

                    if (previous_mode == MENU_MAIN_BYTE) {
                        mem[current_address] = input_value;
                    }

                    if (previous_mode == MENU_MODE_RUN) {
                        // Continue running
                        mode = previous_mode;
                        is_running_full = true;
                    }
                }

                if (input == INPUT_CONF_CONTINUE) {
                    if (previous_mode == MENU_MODE_RUN) {
                        display_mode++;
                        if (display_mode > 4) display_mode = 0;
                        mode = MENU_MODE_CONFIRM;
                    } else {
                        // Accept input and jump back to data-entry
                        // NOTE Continue button only shown after byte entry
                        mem[current_address] = input_value;
                        current_address++;
                        mode = previous_mode;
                        mode_changed = false;
                    }
                }

                if (input == INPUT_CONF_CANCEL && previous_mode == MENU_MODE_RUN) {
                    led_state = false;
                    gpio_put(PIN_PICO_LED, false);
                }

                // Show the current values
                update_display();
                break;
            case MENU_MODE_RUN:
                // Key press during a run -- treat this as a pause
                // so show the Confirm Menu to continue or cancel
                previous_mode = mode;
                mode = MENU_MODE_CONFIRM;
                mode_changed = true;
                is_running_full = false;
                break;
            case MENU_MODE_RUN_DONE:
                // Key press after run returned, so go back to
                // the Main Menu
                mode = MENU_MODE_MAIN;
                mode_changed = true;
                is_running_full = false;
                current_address = start_address;
                break;
            default:
                // Main menu
                // 0 -- Memory stop down                                  -- BLUE
                // 3 -- Memory step up                                    -- BLUE
                // C -- Run code at current memory                        -- WHITE
                // D -- Run code at current memory, single stepping       -- GREEN
                // E -- Input 8-bit value to current memory location      -- CYAN
                // F -- Input 16-bit value to set current memory location -- YELLOW
                if (input == INPUT_MAIN_ADDR) {
                    mode = MENU_MAIN_ADDR;
                    mode_changed = true;
                }

                if (input == INPUT_MAIN_BYTE) {
                    mode = MENU_MAIN_BYTE;
                    mode_changed = true;
                }

                if (input == INPUT_MAIN_RUN_STEP) {
                    mode = MENU_MODE_STEP;
                    mode_changed = true;
                    is_running_steps = true;
                    start_address = current_address;
                    reg.pc = current_address;
                    do_display_pc = true;
                }

                if (input == INPUT_MAIN_RUN) {
                    mode = MENU_MODE_RUN;
                    mode_changed = true;
                    is_running_full = true;
                    start_address = current_address;
                    reg.pc = current_address;
                }

                if (input == INPUT_MAIN_MEM_UP || input == INPUT_MAIN_MEM_DOWN) {
                    current_address += (input == INPUT_MAIN_MEM_UP ? 1 : -1);
                    display_left(current_address);
                    display_right(mem[current_address]);
                }

                if (input == INPUT_MAIN_LOAD) {
                    load_code();
                    display_left(current_address);
                    display_right(mem[current_address]);
                }
        }
    }

    if (mode_changed) set_keys();
}


/**
    Prime the keypad for the current menu mode, setting key colours,
    masking the keys that can be pressed, and, for data-entry screens,
    the number of digits that can be entered.
 */
void set_keys() {
    // Clear the keyboard
    keypad_clear();
    keypad_update_leds();

    // Illuminate available keys according to menu mode
    switch (mode) {
        case MENU_MAIN_ADDR:
            // Enter 16-bit address: all keys yellow
            keypad_set_all(0x10, 0x10, 0x00);
            input_count = 4;
            input_mask = 0xFFFF;
            input_value = 0;
            break;
        case MENU_MAIN_BYTE:
            // Enter 8-bit data: all keys cyan
            keypad_set_all(0x00, 0x10, 0x10);
            input_count = 2;
            input_mask = 0xFFFF;
            input_value = 0;
            break;
        case MENU_MODE_STEP:
            // Run code in single-step mode
            keypad_set_led(15, 0x00, 0x10, 0x00);
            keypad_set_led(14, 0x10, 0x00, 0x10);
            keypad_set_led(13, 0x20, 0x10, 0x00);
            keypad_set_led(12, 0x10, 0x00, 0x00);
            keypad_set_led(3,  0x00, 0x00, 0x40);
            keypad_set_led(0,  0x00, 0x00, 0x40);
            input_count = 1;
            input_mask = INPUT_STEP_MASK;
            break;
        case MENU_MODE_CONFIRM:
            // Enter or cancel input
            keypad_set_led(15, 0x00, 0x10, 0x00);
            keypad_set_led(12, 0x10, 0x00, 0x00);
            input_count = 1;
            input_mask = INPUT_CONF_MASK_ADDR;

            // Show continue for program entry
            if (previous_mode == MENU_MAIN_BYTE) {
                keypad_set_led(14, 0x20, 0x10, 0x00);
                input_mask = INPUT_CONF_MASK_BYTE;
            }

            // Show display change for run pause
            if (previous_mode == MENU_MODE_RUN) {
                keypad_set_led(14, 0x10, 0x00, 0x10);
                input_mask = INPUT_CONF_MASK_BYTE;
            }

            break;
        case MENU_MODE_RUN:
            keypad_set_all(0x10, 0x20, 0x20);
            input_count = 1;
            input_mask = INPUT_RUN_MASK;
            break;
        case MENU_MODE_RUN_DONE:
            keypad_set_all(0x03, 0x06, 0x06);
            input_count = 1;
            input_mask = INPUT_RUN_MASK;
            break;
        default:
            // Main menu: enter an action
            keypad_set_led(15, 0x10, 0x10, 0x00);
            keypad_set_led(14, 0x00, 0x10, 0x10);
            keypad_set_led(13, 0x00, 0x10, 0x00);
            keypad_set_led(12, 0x10, 0x20, 0x20);
            keypad_set_led(3,  0x00, 0x00, 0x40);
            keypad_set_led(0,  0x00, 0x00, 0x40);
            keypad_set_led(11, 0x20, 0x00, 0x20);
            input_count = 1;
            input_mask = INPUT_MAIN_MASK;
            break;
    }

    keypad_update_leds();
    mode_changed = false;
}


/**
    Convert the key press value into an actual value.

    - Parameters:
        - input: The key press value read from the keypad.

    - Returns: The actual value represented by the pressed key.
 */
uint8_t keypress_to_value(uint16_t input) {
    // NOTE Assumes one key press and exits at the first
    //      low bit that is set
    for (uint32_t i = 0 ; i < 16 ; ++i) {
        if ((input & (1 << i)) != 0) return i;
    }

    return 0;
}


/**
    Update the display by mode.
 */
void update_display() {
    uint16_t left = 0;
    uint16_t right = 0;
    uint16_t address = 0;
    uint16_t val = 0;
    switch(display_mode) {
        case 0:
            address = is_running_full ? reg.pc : current_address;
            val = is_running_full ? (uint16_t)mem[reg.pc] : (uint16_t)mem[current_address];
            display_left(address);
            display_right(val);
            printf("0x%04X -> 0x%02X\n", address, val);
            return;
        case 1:
            display_cc();
            return;
        case 2:
            display_ab_dp();
            return;
        case 3:
            left = reg.x;
            right = reg.y;
            break;
        default:
            left = reg.s;
            right = reg.u;
    }

    display_value(left,  DISPLAY_LEFT,  true, false);
    display_value(right, DISPLAY_RIGHT, true, false);
}

/**
    Display the CC register as eight binary digits.
 */
void display_cc() {
    ht16k33_clear(display_address[DISPLAY_LEFT],  display_buffer[DISPLAY_LEFT]);
    ht16k33_clear(display_address[DISPLAY_RIGHT], display_buffer[DISPLAY_RIGHT]);

    for (uint8_t i = 0 ; i < 4 ; ++i) {
        ht16k33_set_number(display_address[DISPLAY_RIGHT], display_buffer[DISPLAY_RIGHT], ((reg.cc >> i) & 0x01), 3 - i, false);
    }

    for (uint8_t i = 4 ; i < 8 ; ++i) {
        ht16k33_set_number(display_address[DISPLAY_LEFT], display_buffer[DISPLAY_LEFT], ((reg.cc >> i) & 0x01), 7 - i, false);
    }

    ht16k33_draw(display_address[DISPLAY_LEFT],  display_buffer[DISPLAY_LEFT]);
    ht16k33_draw(display_address[DISPLAY_RIGHT], display_buffer[DISPLAY_RIGHT]);
}

/**
    Display the A, B and DP registers evenly spaced on the display.
 */
void display_ab_dp() {
    ht16k33_clear(display_address[DISPLAY_LEFT],  display_buffer[DISPLAY_LEFT]);
    ht16k33_clear(display_address[DISPLAY_RIGHT], display_buffer[DISPLAY_RIGHT]);

    // A register
    ht16k33_set_number(display_address[DISPLAY_LEFT], display_buffer[DISPLAY_LEFT], ((reg.a >> 4) & 0x0F), 0, false);
    ht16k33_set_number(display_address[DISPLAY_LEFT], display_buffer[DISPLAY_LEFT], (reg.a & 0x0F), 1, false);

    // B register
    ht16k33_set_number(display_address[DISPLAY_LEFT], display_buffer[DISPLAY_LEFT], ((reg.b >> 4) & 0x0F), 3, false);
    ht16k33_set_number(display_address[DISPLAY_RIGHT], display_buffer[DISPLAY_RIGHT], (reg.b & 0x0F), 0, false);

    // DP register
    ht16k33_set_number(display_address[DISPLAY_RIGHT], display_buffer[DISPLAY_RIGHT], ((reg.dp >> 4) & 0x0F), 2, false);
    ht16k33_set_number(display_address[DISPLAY_RIGHT], display_buffer[DISPLAY_RIGHT], (reg.dp & 0x0F), 3, false);

    ht16k33_draw(display_address[DISPLAY_LEFT],  display_buffer[DISPLAY_LEFT]);
    ht16k33_draw(display_address[DISPLAY_RIGHT], display_buffer[DISPLAY_RIGHT]);
}


/**
    Display a 16-bit value on the left display.

    - Parameters:
        - value: The 16-bit value to display.
 */
void display_left(uint16_t value) {
    display_value(value, DISPLAY_LEFT, true, false);
}


/**
    Display an 8-bit value on the right display.

    - Parameters:
        - value: The 8-bit value to display.
 */
void display_right(uint16_t value) {
    display_value(value, DISPLAY_RIGHT, false, false);
}


/*
    Display an 8- or 16-bit value on a display, specified by index:
    0 = left, 1 = right.

    - Parameters:
        - value:     The value to display.
        - index:     The display's index, 0 or 1.
        - is_16_bit: Whether the value is 16-bit (`true`) or 8-bit (`false`).
 */
void display_value(uint16_t value, uint8_t index, bool is_16_bit, bool show_colon) {
    if (value > 0xFFFF) value = 0x0000;
    if (!is_16_bit) value &= 0xFF;
    ht16k33_clear(display_address[index], display_buffer[index]);

    if (is_16_bit) {
        ht16k33_set_number(display_address[index], display_buffer[index], (value >> 12) & 0x0F, 0, false);
        ht16k33_set_number(display_address[index], display_buffer[index], (value >> 8) & 0x0F, 1, false);
    }

    ht16k33_set_number(display_address[index], display_buffer[index], (value >> 4) & 0x0F, 2, false);
    ht16k33_set_number(display_address[index], display_buffer[index], value & 0x0F, 3, false);

    if (show_colon) ht16k33_show_colon(display_address[index], display_buffer[index], true);
    ht16k33_draw(display_address[index], display_buffer[index]);
}


/**
    Load code via STDIO on USB and store in memory. The incoming data structure
    indicates at what address this will start.

    - Returns: `true` on a successful load, otherwise `false`.
 */
bool load_code() {
    uint8_t load_buffer[262];
    uint16_t bytes_read = 0;
    uint16_t block_count = 0;
    uint16_t address = 0;
    uint16_t start_addr = 0;
    uint32_t start = time_us_32();


    // Light the LED to show we're clear to receive
    gpio_put(PIN_PICO_LED, true);

    while(true) {
        bytes_read = get_block(load_buffer);
        if (bytes_read > 0) {
            block_count++;
            if (load_buffer[0] == 0x55 && load_buffer[1] == 0x3C) {
                // Got a valid block header
                uint8_t block_type = load_buffer[2];
                uint16_t block_len = load_buffer[3];

                if (block_type == 0xFF) {
                    // Done -- signal the sender back and
                    // turn off the LED
                    printf("OK END\n");
                    gpio_put(PIN_PICO_LED, false);
                    current_address = start_addr;
                    return true;
                }

                // Get the checksum on all blocks but the last
                uint32_t checksum = 0;
                for (uint16_t i = 2 ; i < 4 + block_len ; i++) {
                    checksum += load_buffer[i];
                }

                // Break to error if the checksum is bad
                if ((checksum & 0xFF) != load_buffer[4 + block_len]) break;

                // Look for the address block
                if (block_type == 0x00) {
                    if (load_buffer[3] != 0x02) break;
                    address = (load_buffer[4] << 8) | load_buffer[5];
                    start_addr = address;

                    // ACK the block
                    printf("OK ADDRESS BLOCK: %i\n", start_addr);
                }

                // Look for a data block
                if (block_type == 0x01) {
                    for (uint16_t i = 0 ; i < block_len ; ++i) {
                        mem[address++] = load_buffer[4 + i];
                    }

                    // ACK the block
                    printf("OK DATA BYTES: %i,%i\n", block_count, block_len);
                }
            }

            bytes_read = 0;
        }

        // Check for a timeout: break on fail
        if (time_us_32() - start > UPLOAD_TIMEOUT_US) break;
    }

    // Signal an error and turn off the LED
    flash_led(5);
    gpio_put(PIN_PICO_LED, false);
    return false;
}


/**
    Read in a single transmitted block (up to 262 bytes).

    - Parameters:
        - buff: A pointer to the byte store buffer,

    - Returns: The index of the last read byte in the buffer.
 */
uint16_t get_block(uint8_t *buff) {
    uint16_t buff_ptr = 0;
    while (true) {
        int c = getchar_timeout_us(100);
        if (c != PICO_ERROR_TIMEOUT && buff_ptr < 262) {
            buff[buff_ptr++] = (c & 0xFF);
            display_left(buff_ptr);
        } else {
            break;
        }
    }

    sleep_ms(10);
    return buff_ptr;
}
