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
#define PIN_6809_NMI                22
#define PIN_6809_IRQ                20
#define PIN_6809_FIRQ               21

#define PIN_PICO_LED                25

#define RP2040_FLASH_DATA_START     1048576
#define RP2040_FLASH_DATA_SIZE      65536


/*
 *      PROTOTYPES
 */
uint8_t     sample_interrupts(void);
void        flash_led(uint8_t count);


#endif // _E6809_HEADER_
