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


/*
 *      INCLUDES
 */
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
#include "ht16k33.h"


/*
 *      CONSTANTS
 */
#define PIN_STEP_BUTTON             17
#define DEBOUNCE_TIME_US            5000

#define DISPLAY_LEFT                0
#define DISPLAY_RIGHT               1

#define MENU_MODE_MAIN              0
#define MENU_MAIN_ADDR              1
#define MENU_MAIN_BYTE              2
#define MENU_MAIN_RUN_STEP          3
#define MENU_MAIN_RUN               4
#define INPUT_MAIN_ADDR             0x8000
#define INPUT_MAIN_BYTE             0x4000
#define INPUT_MAIN_RUN_STEP         0x2000
#define INPUT_MAIN_RUN              0x1000
#define INPUT_MAIN_MEM_UP           0x0008
#define INPUT_MAIN_MEM_DOWN         0x0001
#define INPUT_MAIN_MASK             0xF009

#define MENU_MODE_STEP              10
#define INPUT_STEP_NEXT             0x8000
#define INPUT_STEP_SHOW_CC          0x4000
#define INPUT_STEP_SHOW_AD          0x2000
#define INPUT_STEP_EXIT             0x1000
#define INPUT_STEP_MEM_UP           0x0008
#define INPUT_STEP_MEM_DOWN         0x0001
#define INPUT_STEP_MASK             0xF009

#define MENU_MODE_CONFIRM           20
#define MENU_CONF_OK                21
#define MENU_CONF_CANCEL            22
#define MENU_CONF_CONTINUE          23
#define INPUT_CONF_OK               0x8000
#define INPUT_CONF_CANCEL           0x1000
#define INPUT_CONF_CONTINUE         0x4000
#define INPUT_CONF_MASK_ADDR        0x9000
#define INPUT_CONF_MASK_BYTE        0xD000

#define MENU_MODE_RUN               30
#define MENU_MODE_RUN_DONE          31
#define INPUT_RUN_MASK              0xFFFF


/*
 *      PROTOTYPES
 */
bool        setup_board();
void        boot_cpu();

void        ui_input_loop();
void        process_key(uint16_t);
void        set_keys();
uint8_t     keypress_to_value(uint16_t input);

void        update_display();
void        display_cc();
void        display_left(uint16_t value);
void        display_right(uint16_t value);
void        display_value(uint16_t value, uint8_t index, bool is_16_bit);

void        run_tests();


#endif // _E6809_HEADER_
