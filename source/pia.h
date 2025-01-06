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
#ifndef _PIA_HEADER_
#define _PIA_HEADER_


/*
 * INCLUDES
 */
#include <stdint.h>


/*
 *      CONSTANTS
 */
#define     INPUT       1
#define     OUTPUT      0


/*
 * STRUCTS
 */
typedef struct {
    uint8_t*    pa_pins;
    uint8_t*    ca_pins;
    uint8_t*    reg_control_a;
    uint8_t*    reg_data_a;
    uint8_t     reg_output_a;
    uint8_t     reg_direction_a;
    bool        emit_output;
    bool        enabled;
    bool        ca_1_can_interrupt;
    bool        ca_1_is_set_on_up;
    bool        ca_2_can_interrupt;
    bool        ca_2_is_set_on_up;
    bool        ca_2_is_output;
} MC6821;


/*
 *      PROTOTYPES
 */
void        pia_init(MC6821* pia);
void        pia_reset(MC6821* pia);

void        pia_set_gpio_direction(MC6821* pia, uint8_t pin);
uint8_t     pia_get_gpio_direction(MC6821* pia, uint8_t pin);
void        pia_set_gpio_output_state(MC6821* pia, uint8_t pin);
void        pia_get_gpio_input_state(MC6821* pia, uint8_t pin);

void        pia_update(MC6821* pia);
void        pia_update_flags(MC6821* pia);

void        pia_set_pia_ca(MC6821* pia);
void        pia_check_irqs(void);


#endif  // _PIA_HEADER_
