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
#include "hardware/spi.h"
#include "hardware/adc.h"
#include "ops.h"
#include "cpu.h"
#include "cpu_tests.h"
#include "keypad.h"


/*
 * CONSTANTS
 */
#define PIN_STEP_BUTTON             17
#define DEBOUNCE_TIME_US            5000

#define PIN_LED_C                   2
#define PIN_LED_V                   3
#define PIN_LED_Z                   4
#define PIN_LED_N                   5

/*
 * STRUCTURES
 */


/*
 * PROTOTYPES
 */
void        boot();
void        loop();
void        process_key(uint16_t);
void        set_keys();
uint8_t     get_val(uint16_t input);
uint16_t    inkey();
void        setup_cc_leds();
void        dump_registers();
void        run_tests();

#endif // _E6809_HEADER_
