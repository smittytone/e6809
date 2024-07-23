/*
 * e6809 for Raspberry Pi Pico
 * Peripheral Interface Adapter (PIA)
 *
 * @version     1.0.0
 * @author      smittytone
 * @copyright   2024
 * @licence     MIT
 *
 */
#include "main.h"


uint8_t     pia_pa_pins[]    = {6, 7, 8, 9, 10, 11, 12, 13};
uint8_t     pia_ca_pins[]    = {14, 15};
uint8_t     reg_control_a = 0;
uint8_t     reg_datadir_a = 0;
uint8_t     reg_output_a  = 0;

uint16_t    mem_control_a = 0x0000;
uint16_t    mem_datadir_a = 0x0000;

bool        enabled = false;
bool        ca_1_can_interrupt = false;
bool        ca_1_is_set_on_up = false;
bool        ca_2_can_interrupt = false;
bool        ca_2_is_set_on_up = false;
bool        ca_2_is_output = false;


void pia_init(uint16_t cra, uint16_t ddra) {
    // Record register memory locations
    mem_control_a = cra;
    mem_datadir_a = ddra;

    // Initialise the GPIO
    pia_init_gpio();

    // Perform a reset
    pia_reset();
}


void pia_reset() {
    enabled = true;

    // All output, all low
    reg_control_a = 0;
    reg_datadir_a = 0;
    reg_output_a = 0;

    // CA interrupts disabled, CA2 is input
    ca_1_can_interrupt = false;
    ca_1_is_set_on_up = false;
    ca_2_can_interrupt = false;
    ca_2_is_set_on_up = false;
    ca_2_is_output = false;

    // Update GPIO directions (all inputs)
    for (uint8_t i = 0 ; i < 8 ; i++) {
        gpio_set_dir(pia_pa_pins[i], false);
    }

    // Set up the CA pins (inputs)
    gpio_set_dir(pia_ca_pins[0], false);
    gpio_set_dir(pia_ca_pins[1], false);
}


void pia_init_gpio() {
    // On RESET, set PA0-7, CA1, CA2 is inputs
    // See MC6821 Data Sheet p6

    // Set up the PA pins
    for (uint8_t i = 0 ; i < 8 ; i++) {
        gpio_init(pia_pa_pins[i]);
    }

    // Set up the CA pins
    gpio_init(pia_ca_pins[0]);  // CA 1
    gpio_init(pia_ca_pins[1]);  // CA 2
}


/*
    Set the PA direction based on the DDR
 */
void set_gpio_direction(uint8_t pin) {
    // Pico SDK for put() -- false is input, true is output
    uint8_t value = ((reg_datadir_a & (1 << pin)) >> pin);
    gpio_set_dir(pia_pa_pins[pin], (value == OUTPUT));

    // If it's an output, set its state
    // according to the OR
    if (value == OUTPUT) set_gpio_output_state(pin);
}


/*
    Get the RP2040 pin direction.

    - Returns: 1 for output, 0 for input
 */
uint8_t get_gpio_direction(uint8_t pin) {
    return ((reg_datadir_a & (1 << pin)) >> pin);
}


/*
    Set the PA output value based on the OR.
    NOTE Assumes we have already checked that the pin
         *is* an output.
 */
void set_gpio_output_state(uint8_t pin) {
    uint8_t value = ((reg_output_a & (1 << pin)) >> pin);
    gpio_put(pia_pa_pins[pin], (value == 1));
}

/*
    Read a specific input and set its OR bit
    accordingly.
 */
void get_gpio_input_state(uint8_t pin) {
    if (gpio_get(pia_pa_pins[pin])) {
        reg_output_a |= (1 << pin);
    } else {
        reg_output_a &= !(1 << pin);
    }
}


void pia_update() {
    if (!enabled) return;

    // Update CR
    // Get bits 0-5 from memory; retain bits 6 and 7
    reg_control_a = (reg_control_a & 0xC0) | (mem[mem_control_a] & 0x3F);
    pia_set_pia_ca();
    pia_update_flags();

    // Check the DDR Access bit
    uint8_t new = mem[mem_datadir_a];
    if (reg_control_a & 0x04) {
        // Read in the OR from memory (set by CPU)
        // and set the pin state -- it its an output
        if (new != reg_output_a) {
            reg_output_a = new;
            for (uint8_t i = 0 ; i < 8 ; i++) {
                // Update all output pins states
                if (get_gpio_direction(i) == OUTPUT) set_gpio_output_state(i);
            }
        }
    } else {
        // Read in the DDR and set pin direction
        // NOTE This uses OR for state on output
        if (new != reg_datadir_a) {
            reg_datadir_a = new;
            for (uint8_t i = 0 ; i < 8 ; i++) {
                set_gpio_direction(i);
            }
        }
    }

    // Read inputs
    for (uint8_t i = 0 ; i < 8 ; i++) {
        // Update all input pins states: affects OR
        if (get_gpio_direction(i) == INPUT) get_gpio_input_state(i);
    }

    // Handle IRQs
    pia_check_irqs();
}


void pia_update_flags() {
    ca_1_can_interrupt = ((reg_control_a & 0x01) > 0);

    // Bit is 1, IRQ trigged on LOW to HIGH (up), else
    // on HIGH to LOW (down)
    ca_1_is_set_on_up = ((reg_control_a & 0x02) > 0);

    bool is_output = ((reg_control_a & 0x20) > 0);
    if (ca_2_is_output != is_output) {
        // Settings changed
        ca_2_is_output = is_output;

        if (!ca_2_is_output) {
            ca_2_can_interrupt = ((reg_control_a & 0x08) > 0);

            // Bit is 1, IRQ trigged on LOW to HIGH (up), else
            // on HIGH to LOW (down)
            ca_2_is_set_on_up = ((reg_control_a & 0x10) > 0);
        } else {

        }
    }

}


void pia_check_irqs() {

}

/*
    Set CA2 -- if it's an output -- to value of CR bit 3
 */
void pia_set_pia_ca() {
    if (ca_2_is_output & (reg_control_a & 0x10) > 0) {
        gpio_put(pia_ca_pins[1], (reg_control_a & 0x08 > 0));
    }
}
