/*
 * e6809 for Raspberry Pi Pico
 *
 * @version     0.0.2
 * @author      smittytone
 * @copyright   2024
 * @licence     MIT
 *
 */
#ifndef _E6809_HEADER_
#define _E6809_HEADER_


/*
 *      INCLUDES
 */
// C
#include <stdint.h>


/*
 *      CONSTANTS
 */
// NOTE These are RP2040 GPIO pin numbers
#define PIN_6809_NMI                0
#define PIN_6809_IRQ                1
#define PIN_6809_FIRQ               2
#define PIN_6809_RESET              3

#define PIN_PICO_LED                25

#define PIN_6821_PA0                6
#define PIN_6821_PA1                7
#define PIN_6821_PA2                8
#define PIN_6821_PA3                9
#define PIN_6821_PA4                10
#define PIN_6821_PA5                11
#define PIN_6821_PA6                12
#define PIN_6821_PA7                13
#define PIN_6821_CA1                14
#define PIN_6821_CA2                15

#define RP2040_IRQ_GPIO_COUNT       4
#define RP2040_PIA_GPIO_COUNT       10

#define RP2040_FLASH_DATA_START     1048576
#define RP2040_FLASH_DATA_SIZE      65536


/*
 * STRUCTS
 */
typedef struct {
    bool        has_led;
    bool        has_mc6821;
    uint8_t     irq_gpio[RP2040_IRQ_GPIO_COUNT];
    uint8_t     pia_gpio[RP2040_PIA_GPIO_COUNT];
} STATE_RP2040;


/*
 *      PROTOTYPES
 */
uint8_t     sample_interrupts(void);
void        flash_led(uint8_t count);


#endif // _E6809_HEADER_
