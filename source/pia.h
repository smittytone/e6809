/*
 * e6809 for Raspberry Pi Pico
 * Peripheral Interface Adapter (PIA)
 *
 * @version     0.0.2
 * @author      smittytone
 * @copyright   2024
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
 *      PROTOTYPES
 */
void        pia_init(uint16_t cra, uint16_t ddra, uint8_t* pa_pins, uint8_t* ca_pins);
void        pia_init_gpio(void);

void        pia_reset(void);

void        set_gpio_direction(uint8_t pin);
uint8_t     get_gpio_direction(uint8_t pin);
void        set_gpio_output_state(uint8_t pin);
void        get_gpio_input_state(uint8_t pin);

void        pia_update(void);
void        pia_update_flags(void);

void        pia_set_pia_ca(void);
void        pia_check_irqs(void);


#endif  // _PIA_HEADER_
