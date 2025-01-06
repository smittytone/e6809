/*
 * e6809 for Raspberry Pi Pico
 * Peripheral Interface Adapter (PIA)
 *
 * @version     0.0.2
 * @author      smittytone
 * @copyright   2025
 * @licence     MIT
 *
 */

#include <stdbool.h>
// Pico
#include "pico/stdlib.h"
#include "hardware/gpio.h"
// App
#include "main.h"
#include "cpu.h"
#include "pia.h"


/*
 * STATICS
 */
static void pia_set_data_byte(MC6821* pia);


/*
 * GLOBALS
 */
extern REG_6809     reg;
extern uint8_t      mem[KB64];
extern STATE_RP2040 pico_state;


/**
 * @brief Initialise a PIA.
 *
 * @param pia: Pointee to an MC6821 struc
 */
void pia_init(MC6821* pia) {

    // Perform a reset
    pia_reset(pia);
}


void pia_reset(MC6821* pia) {

    pia->enabled = true;

    // All output, all low
    // NOTE This is a local store, for reference
    pia->reg_output_a = 0;
    pia->reg_direction_a = 0;

    // CA interrupts disabled, CA2 is input
    pia->ca_1_can_interrupt = false;
    pia->ca_1_is_set_on_up = false;
    pia->ca_2_can_interrupt = false;
    pia->ca_2_is_set_on_up = false;
    pia->ca_2_is_output = false;

    // Update GPIO directions (set all to input with pullup)
    // See MC6821 Datasheet p.8
    for (uint8_t i = 0 ; i < 8 ; i++) {
        gpio_set_dir(*(pia->pa_pins + i), GPIO_IN);
        gpio_pull_up(*(pia->pa_pins + i));
    }

    // Set the CA pins (inputs)
    gpio_set_dir(*(pia->ca_pins), GPIO_IN);
    gpio_set_dir(*(pia->ca_pins + 1), GPIO_IN);
    
    // Update the Control Register
    
}


/**
 * @brief Update the PIA status based on the current value of
 *        the Control Register.
 */
void pia_process_control(MC6821* pia) {
    
    uint16_t reg_value = (uint16_t)*(pia->reg_control_a);
    
    pia->ca_1_can_interrupt = is_bit_set(reg_value, 0);
    pia->ca_1_is_set_on_up = is_bit_set(reg_value, 1);
    
    pia->ca_2_can_interrupt = is_bit_set(reg_value, 5);
    
    if (pia->ca_2_can_interrupt) {
        pia->ca_2_is_output = true;
        if (is_bit_set(reg_value, 4)) {
            gpio_put(*(pia->ca_pins + 1), is_bit_set(reg_value, 3));
        } else {
            // See MCP6821 Datasheet p.10
        }
    } else {
        pia->ca_2_is_output = false;
        pia->ca_2_is_set_on_up = is_bit_set(reg_value, 4);
    }
    
    // Data direction or Output register?
    pia->emit_output = is_bit_set(reg_value, 2);
    pia_set_data_byte(pia);
}


static void pia_set_data_byte(MC6821* pia) {
    
    if (pia->emit_output) {
        *(pia->reg_data_a) = pia->reg_output_a;
    } else {
        *(pia->reg_data_a) = pia->reg_direction_a;
    }
}


/*
    Set the PA direction based on the DDR
 */
void pia_set_gpio_direction(MC6821* pia, uint8_t pin) {

    // Pico GPIO directions: false is input, true is output
    uint8_t value = ((*pia->reg_data_a & (1 << pin)) >> pin);
    gpio_set_dir(*(pia->pa_pins + pin), (value == OUTPUT));

    // If the pin is an output, set its pin level according
    // to the output register value
    if (value == OUTPUT) pia_set_gpio_output_state(pia, pin);
}


/*
    Get the RP2040 pin direction.

    - Returns: 1 for output, 0 for input
 */
uint8_t pia_get_gpio_direction(MC6821* pia, uint8_t pin) {

    return ((*pia->reg_data_a & (1 << pin)) >> pin);
}


/**
 * @brief Set the PA output value based on the OR.
 *
 * @note Assumes we have already checked that the pin
 *       *is* an output.
 */
void pia_set_gpio_output_state(MC6821* pia, uint8_t pin) {

    uint8_t value = ((pia->reg_output_a & (1 << pin)) >> pin);
    gpio_put(*(pia->pa_pins + pin), (value == 1));
}

/**
 * @brief Read a specific input and set its output register bit
 *        accordingly.
 */
void pia_get_gpio_input_state(MC6821* pia, uint8_t pin) {

    if (gpio_get(*(pia->pa_pins + pin))) {
        pia->reg_output_a |= (1 << pin);
    } else {
        pia->reg_output_a &= !(1 << pin);
    }
}


/**
 * @brief Update the memory for the specified PIA.
 */
void pia_update(MC6821* pia) {

    if (!pia->enabled) return;

    // Update CR
    // Get bits 0-5 from memory; retain bits 6 and 7
    //pia->reg_control_a = (pia->reg_control_a & 0xC0) | (mem[*pia->reg_control_a] & 0x3F);
    pia_set_pia_ca(pia);
    pia_update_flags(pia);

    // Check the DDR Access bit
    uint8_t new = *pia->reg_data_a;
    if (*pia->reg_control_a & 0x04) {
        // Read in the Output Register from memory (set by CPU)
        // and set the pin state -- if its an output
        if (new != pia->reg_output_a) {
            pia->reg_output_a = new;
            for (uint8_t i = 0 ; i < 8 ; i++) {
                // Update all output pins states
                if (pia_get_gpio_direction(pia, i) == OUTPUT) pia_set_gpio_output_state(pia, i);
            }
        }
    } else {
        // Read in the DDR and set pin direction
        // NOTE This uses OR for state on output
        if (new != *pia->reg_data_a) {
            *pia->reg_data_a = new;
            for (uint8_t i = 0 ; i < 8 ; i++) {
                pia_set_gpio_direction(pia, i);
            }
        }
    }

    // Read inputs
    for (uint8_t i = 0 ; i < 8 ; i++) {
        // Update all input pins states: affects OR
        if (pia_get_gpio_direction(pia, i) == INPUT) pia_get_gpio_input_state(pia, i);
    }

    // Handle IRQs
    pia_check_irqs();
}


void pia_update_flags(MC6821* pia) {

    pia->ca_1_can_interrupt = ((*pia->reg_control_a & 0x01) > 0);

    // Bit is 1, IRQ trigged on LOW to HIGH (up), else
    // on HIGH to LOW (down)
    pia->ca_1_is_set_on_up = ((*pia->reg_control_a & 0x02) > 0);

    bool is_output = ((*pia->reg_control_a & 0x20) > 0);
    if (pia->ca_2_is_output != is_output) {
        // Settings changed
        pia->ca_2_is_output = is_output;

        if (!pia->ca_2_is_output) {
            pia->ca_2_can_interrupt = ((*pia->reg_control_a & 0x08) > 0);

            // Bit is 1, IRQ trigged on LOW to HIGH (up), else
            // on HIGH to LOW (down)
            pia->ca_2_is_set_on_up = ((*pia->reg_control_a & 0x10) > 0);
        } else {

        }
    }
}


void pia_check_irqs(void) {

    // TODO
}


/*
    Set CA2 -- if it's an output -- to value of CR bit 3
 */
void pia_set_pia_ca(MC6821* pia) {

    if (pia->ca_2_is_output & (*pia->reg_control_a & 0x10) > 0) {
        gpio_put(*(pia->ca_pins + 1), (*pia->reg_control_a & 0x08 > 0));
    }
}
