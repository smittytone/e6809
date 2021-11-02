/*
 * e6809 for Raspberry Pi Pico
 * Peripheral Interface Adapter (PIA)
 *
 * @version     1.0.0
 * @author      smittytone
 * @copyright   2021
 * @licence     MIT
 *
 */
#ifndef _PIA_HEADER_
#define _PIA_HEADER_


/*
 *      CONSTANTS
 */
#define     INPUT       1
#define     OUTPUT      0


/*
 *      PROTOTYPES
 */
void        pia_init(uint16_t cra, uint16_t ddra);
void        pia_init_gpio();

void        pia_reset();

void        set_gpio_direction(uint8_t pin);
uint8_t     get_gpio_direction(uint8_t pin);
void        set_gpio_output_state(uint8_t pin);
void        get_gpio_input_state(uint8_t pin);

void        pia_update();
void        pia_update_flags();

void        pia_set_pia_ca();
void        pia_check_irqs();


#endif  // _PIA_HEADER_