/*
 * e6809 for Raspberry Pi Pico
 *
 * @version     0.0.2
 * @author      smittytone
 * @copyright   2025
 * @licence     MIT
 *
 */

// C
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
// Pico
#include "pico/stdlib.h"
#include "hardware/flash.h"
#include "hardware/sync.h"
// App
#include "ops.h"
#include "cpu.h"
#include "cpu_tests.h"
#include "monitor.h"
#include "pia.h"
#include "main.h"


/*
 *  STATICS
 */
static void boot_cpu(void);
static void init_rp2040_gpio(void);
// EXPERIMENTAL
static void read_into_ram(void);
static void save_ram(void);


/*
 *  GLOBALS
 */
STATE_RP2040 pico_state;

extern REG_6809    reg;
extern STATE_6809  state;
extern uint8_t     mem[KB64];

MC6821 pia01;


/*
 * ENTRY POINT
 */
int main() {
    // Enable STDIO
    stdio_usb_init();
#ifdef DEBUG
    // Pause to allow the USB path to initialize
    sleep_ms(2000);
#endif
    
    // Basic RP2040 config
    pico_state.has_led = true;
    pico_state.has_mc6821 = false;
    
    // Prepare the board
    bool is_using_monitor = init_board();
    
    // Set up the host MCU
    init_rp2040_gpio();

    // Boot the CPU
    boot_cpu();
    
    // Boot the PIA
    if (pico_state.has_mc6821) {
        pia01.pa_pins = &pico_state.pia_gpio[0];
        pia01.ca_pins = &pico_state.pia_gpio[8];
        pia01.reg_control_a = &mem[0xFF00];
        pia01.reg_data_a = &mem[0xFF01];
        pia_init(&pia01);
    }
    
    // Branch according to whether the Pico is connected to a
    // monitor board or not (in which case run tests for now)
    if (is_using_monitor) {
        // Enter the monitor UI
#ifdef DEBUG
        printf("Using monitor board\n");
#endif
        monitor_event_loop();
    } else {
        // Run tests -- for now
#ifdef DEBUG
        printf("Running tests\n");
#endif
        test_main();
    }

    return 0;
}


/**
 * @brief Bring up the virtual 6809e and 64KB of memory.
 *        In future, this will offer alternative memory layouts.
 */
static void boot_cpu(void) {

#ifdef DEBUG
    printf("Resetting the registers\n");
#endif

    // Interrupt Vectors:
    uint16_t vectors[] = {0x0400,       //   RESET
                          0x0500,       //   NMI
                          0x0500,       //   SWI
                          0x0500,       //   IRQ
                          0x0500,       //   FIRQ
                          0x0500,       //   SWI2
                          0x0500,       //   SWI3
                          0x0000};      //   Reserved
    init_vectors(vectors);
    init_cpu();

#ifdef DEBUG
    printf("Initializing memory\n");
#endif
    // Clear the RAM below the IRQ vector table
    memset(mem, 0x00, START_VECTORS);

#ifdef DEBUG
    printf("Entering sample program at 0x4000\n");

    uint16_t start = 0x4000;
    uint8_t prog[] = {0x86,0xFF,0x8E,0x80,0x00,0xA7,0x80,0x8C,0x80,0x09,0x2D,0xF9,0x8E,0x40,0x00,0x6E,0x84};
    memcpy(&mem[start], prog, sizeof(prog));

    reg.pc = start;
    reg.cc = 0x6B;      // WHY THIS SETTING?

    /*
     * TEST PROGS
     */

    /*
    {0x34,0x46,0x33,0x64,0xA6,0x42,0xAE,0x43,0xE6,0x80,0x34,0x04,0x34,0x04,0x4A,0x27,0x13,0xE6,0x80,0xE1,0xE4,0x2E,0x08,0xE1,0x61,0x2E,0x06,0xE7,0x61,0x20,0x02,0xE7,0xE4,0x4A,0x26,0xED,0xA6,0xE0,0xA7,0x45,0xA6,0xE0,0xA7,0x46,0x35,0xC6,0x32,0x7E,0x8E,0x80,0x42,0x34,0x10,0xB6,0x80,0x41,0x34,0x02,0x8D,0xC4,0xA6,0x63,0xE6,0x64,0x39,0x0A,0x01,0x02,0x03,0x04,0x00,0x06,0x07,0x09,0x08,0x0B};
    {0x86, 0x41, 0x8E, 0x04, 0x00, 0xA7, 0x80, 0x8C, 0x06, 0x00, 0x26, 0xF9, 0x1A, 0x0F, 0x3B};
    */
#endif
}


/**
 * @brief Configure the RP2040’s GPIO pins.
 *
 * @note Assumes the use of the Pico board.
 */
static void init_rp2040_gpio(void) {

#ifdef DEBUG
    printf("Setting RP2040 GPIO pins\n");
#endif
    // Configuration
    pico_state.irq_gpio[0] = PIN_6809_NMI;
    pico_state.irq_gpio[1] = PIN_6809_IRQ;
    pico_state.irq_gpio[2] = PIN_6809_FIRQ;
    pico_state.irq_gpio[3] = PIN_6809_RESET;
    
    pico_state.pia_gpio[0] = PIN_6821_PA0;
    pico_state.pia_gpio[1] = PIN_6821_PA1,
    pico_state.pia_gpio[2] = PIN_6821_PA2,
    pico_state.pia_gpio[3] = PIN_6821_PA3,
    pico_state.pia_gpio[4] = PIN_6821_PA4,
    pico_state.pia_gpio[5] = PIN_6821_PA5,
    pico_state.pia_gpio[6] = PIN_6821_PA6,
    pico_state.pia_gpio[7] = PIN_6821_PA7,
    pico_state.pia_gpio[8] = PIN_6821_CA1,
    pico_state.pia_gpio[9] = PIN_6821_CA2;
    
    // Set up the IRQ pins
    for (uint8_t i = 0 ; i < RP2040_IRQ_GPIO_COUNT ; ++i) {
        gpio_init(pico_state.irq_gpio[i]);
        gpio_set_dir(pico_state.irq_gpio[i], GPIO_IN);
        gpio_pull_down(pico_state.irq_gpio[i]);
    }
    
    // Initialize PIA pins if PIA is present
    // TODO Sync with pia.c
    if (pico_state.has_mc6821) {
        for (uint8_t i = 0 ; i < RP2040_PIA_GPIO_COUNT ; ++i) {
            // On RESET, set PA0-7, CA1, CA2 to inputs
            // See MC6821 Data Sheet p6
            gpio_init(pico_state.pia_gpio[i]);
            gpio_set_dir(pico_state.pia_gpio[i], GPIO_IN);
            gpio_pull_down(pico_state.pia_gpio[i]);
        }
    }
    
    // Set up the Pico LED
    gpio_init(PIN_PICO_LED);
    gpio_set_dir(PIN_PICO_LED, GPIO_OUT);
    gpio_put(PIN_PICO_LED, false);
}


/**
 * @brief Sample the interrupt pins and return a bitfield.
 *        This will be called by the CPU code, initially on a per-cycle
 *        basis but later as true interrupts.
 */
uint8_t sample_interrupts(void) {

    uint8_t irqs = 0;
    for (uint8_t i = 0 ; i < 3 ; ++i) {
        if (gpio_get(pico_state.irq_gpio[i])) irqs |= (1 << i);
    }
    return irqs;
}


/**
 * @brief Flash the Pico LED.
 *
 * @param count: The number of blinks in the sequence.
 */
void flash_led(uint8_t count) {

    if (pico_state.has_led) {
        while (count > 0) {
            gpio_put(PIN_PICO_LED, true);
            sleep_ms(250);
            gpio_put(PIN_PICO_LED, false);
            sleep_ms(250);
            count--;
        }
    }
}


/*
 * EXPERIMENTAL
 */
static void read_into_ram(void) {

    // See https://kevinboone.me/picoflash.html?i=1
    // 2MB Flash = 2,097,152
    // Allow 1MB for app code, so start at
    // XIP_BASE + 1,048,576
    // RAM SIZE = 64KB = 65,536
    char *p = (char *)XIP_BASE;
    p += 1048576;

    // Read 64KB from Flash into RAM
    for (uint16_t i = 0 ; i < 65536 ; ++i) {
        mem[i] = (uint8_t)(*p);
    }
}


static void save_ram(void) {

    // See https://kevinboone.me/picoflash.html?i=1
    uint32_t irqs = save_and_disable_interrupts();
    flash_range_erase (RP2040_FLASH_DATA_START, RP2040_FLASH_DATA_SIZE);
    flash_range_program (RP2040_FLASH_DATA_START, mem, RP2040_FLASH_DATA_SIZE);
    restore_interrupts (irqs);
}
