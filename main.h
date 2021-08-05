/*
 * e6809 for Raspberry Pi Pico
 *
 * @version     1.0.0
 * @author      smittytone
 * @copyright   2021
 * @licence     MIT
 *
 */
#ifndef _E6809_HEADER_
#define _E6809_HEADER_


#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "hardware/gpio.h"
#include "hardware/i2c.h"
#include "hardware/adc.h"
#include "ops.h"
#include "cpu.h"

/*
 * CONSTANTS
 */
#define PIN_STEP_BUTTON             24
#define DEBOUNCE_TIME_US            5000


/*
 * STRUCTURES
 */


/*
 * PROTOTYPES
 */
void        boot();
void        loop();
void        inkey();
void        dump_registers();


#endif // _E6809_HEADER_
