/*
 * e6809 for Raspberry Pi Pico
 * Keypad driver
 *
 * @version     0.0.2
 * @author      Pimoroni, smittytone
 * @copyright   2024
 * @licence     MIT
 *
 */

#include <stdbool.h>
// Pico
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/i2c.h"
#include "hardware/spi.h"
// App
#include "ht16k33.h"
#include "keypad.h"


/*
 * STATICS
 */
static bool check_board_presence(void);
static void keypad_set_led_at(uint8_t x, uint8_t y, uint8_t r, uint8_t g, uint8_t b);
static void keypad_set_led_data(uint16_t o, uint8_t r, uint8_t g, uint8_t b);


/*
 * GLOBALS
 */
uint8_t led_buffer[72]; // Dimension = H x W x 4 + 8
uint8_t *led_data = led_buffer + 4;


/**
 * @brief Initialise the keypad, setting the default brightness and
 *        bringing up the I2C0 and SPI0 peripherals. All key pixels
 *        initially set to white.
 *
 * @retval `true` if the board is detected, otherwise `false`.
 */
bool keypad_init(void) {

    for (uint32_t i = 0 ; i < sizeof(led_buffer) ; ++i) {
        led_buffer[i] = 0x00;
    }

    // Must be called to init each LED frame
    keypad_set_brightness(DEFAULT_BRIGHTNESS);

    // Set up I2C to read buttons
    i2c_init(i2c0, 400000);
    gpio_set_function(KEYPAD_PIN_KEYS_SDA, GPIO_FUNC_I2C);
    gpio_set_function(KEYPAD_PIN_KEYS_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(KEYPAD_PIN_KEYS_SDA);
    gpio_pull_up(KEYPAD_PIN_KEYS_SCL);

    // Check a board is connected
    if (!check_board_presence()) {
        i2c_deinit(i2c0);
        return false;
    }

    // Set up SPI to set LEDs
    spi_init(spi0, 4194304);
    gpio_set_function(KEYPAD_PIN_LEDS_CS, GPIO_FUNC_SIO);
    gpio_set_dir(KEYPAD_PIN_LEDS_CS, GPIO_OUT);
    gpio_put(KEYPAD_PIN_LEDS_CS, 1);
    gpio_set_function(KEYPAD_PIN_LEDS_SCK, GPIO_FUNC_SPI);
    gpio_set_function(KEYPAD_PIN_LEDS_TX, GPIO_FUNC_SPI);

    // Set the LEDs
    keypad_set_all(0x20, 0x20, 0x20);
    keypad_update_leds();
    return true;
}


/**
 * @brief Write the pixel colour data out to SPI.
 */
void keypad_update_leds(void) {

    gpio_put(KEYPAD_PIN_LEDS_CS, 0);
    spi_write_blocking(spi0, led_buffer, sizeof(led_buffer));
    gpio_put(KEYPAD_PIN_LEDS_CS, 1);
}


/**
 * @brief Set the brightness for all pixels by writing the brightness bits
 *  to the LED data array for each pixel.
 *
 * @param brightness: The pixels brightness, 0.0-1.0 exclusive.
 */
void keypad_set_brightness(float brightness) {

    if (brightness < 0.0 || brightness > 1.0) return;
    for (uint16_t i = 0 ; i < NUM_KEYS ; ++i) {
        led_data[i * 4] = 0b11100000 | (uint8_t)(brightness * (float)0b11111);
    }
}


/**
 * @brief  Set the data for a single key's pixel, using the specified colour,
 *         and its key value.
 *
 * @param i: The pixel by its key value, 0x00-0F.
 * @param r: The red colour component, 0x00-FF.
 * @param g: The green colour component, 0x00-FF.
 * @param b: The blue colour component, 0x00-FF.
 */
void keypad_set_led(uint8_t i, uint8_t r, uint8_t g, uint8_t b) {

    if (i < 0 || i >= NUM_KEYS) return;
    keypad_set_led_data(i * 4, r, g, b);
}


/**
 * @brief Turn on every key's pixel to the specified colour.
 *
 * @param r: The red colour component, 0x00-FF.
 * @param g: The green colour component, 0x00-FF.
 * @param b: The blue colour component, 0x00-FF.
 */
void keypad_set_all(uint8_t r, uint8_t g, uint8_t b) {

    for (uint16_t i = 0 ; i < NUM_KEYS ; ++i) {
        keypad_set_led(i, r, g, b);
    }
}


/**
 * @brief Turn off every key's pixel.
 */
void keypad_clear(void) {

    for (uint16_t i = 0 ; i < NUM_KEYS ; ++i) {
        keypad_set_led(i, 0, 0, 0);
    }
}


/**
 * @brief Read the TCA9555 IO expander to determine key states.
 *
 * @retval A 16-bit value in which each represents that key's state,
 *         eg. bit 0 is key `0`, bit 15 is key `F`.
 */
uint16_t keypad_get_button_states(void) {

    uint8_t input_buffer[2];
    uint8_t tca9555_reg = 0;
    i2c_write_blocking(i2c0, KEYPAD_I2C_ADDRESS, &tca9555_reg, 1, true);
    i2c_read_blocking(i2c0, KEYPAD_I2C_ADDRESS, input_buffer, 2, false);

    // Read value is 0 = pressed, 1 = not pressed, so invert the return value
    return ~((input_buffer[0]) | (input_buffer[1] << 8));
}


/**
 * @brief Set the data for a single key's pixel, using the specified colour,
 *        and its co-ordinates on the key grid.
 *
 * @param x: The pixel by its x coordinate, 0-3.
 * @param y: The pixel by its y coordinate, 0-3.
 * @param r: The red colour component, 0x00-FF.
 * @param g: The green colour component, 0x00-FF.
 * @param b: The blue colour component, 0x00-FF.
 */
static void keypad_set_led_at(uint8_t x, uint8_t y, uint8_t r, uint8_t g, uint8_t b) {

    if (x < 0 || x >= KEYPAD_WIDTH || y < 0 || y >= KEYPAD_HEIGHT) return;
    keypad_set_led_data((x + (y * KEYPAD_WIDTH)) * 4, r, g, b);
}


/**
 * @brief Set the data for a single key's pixel, using the specified colour
 *        and its index in the data array.
 *
 * @param o: The pixel by its index in the data array.
 * @param r: The red colour component, 0x00-FF.
 * @param g: The green colour component, 0x00-FF.
 * @param b: The blue colour component, 0x00-FF.
 */
static void keypad_set_led_data(uint16_t o, uint8_t r, uint8_t g, uint8_t b) {

    led_data[o + 0] = 0xFF;
    led_data[o + 1] = b;
    led_data[o + 2] = g;
    led_data[o + 3] = r;
}


/**
 * @brief Check for the presence of the TCA9555 IO expander at 0x20.
 *  Will detect *any thing* at 0x20, however.
 *
 * @retval `true` if the board is detected, otherwise `false`.
 */
static bool check_board_presence(void) {

    uint8_t rxdata;
    int ret = i2c_read_blocking(i2c0, KEYPAD_I2C_ADDRESS, &rxdata, 1, false);
    return !(rxdata < 0);
}
