/*
 * e6809 for Raspberry Pi Pico
 *
 * @version     1.0.0
 * @author      smittytone
 * @copyright   2021
 * @licence     MIT
 *
 */
#include "main.h"


uint8_t buffer[72]; // H x W x 4 + 8
uint8_t *led_data = buffer + 4;


void keypad_init() {
    for (uint32_t i = 0 ; i < sizeof(buffer) ; ++i) {
        buffer[i] = 0x00;
    }

    keypad_set_brightness(DEFAULT_BRIGHTNESS); // Must be called to init each LED frame

    // Set up I2C to read buttons
    i2c_init(i2c0, 400000);
    gpio_set_function(KEYPAD_PIN_KEYS_SDA, GPIO_FUNC_I2C);
    gpio_set_function(KEYPAD_PIN_KEYS_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(KEYPAD_PIN_KEYS_SDA);
    gpio_pull_up(KEYPAD_PIN_KEYS_SCL);

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
}

void keypad_update_leds() {
    gpio_put(KEYPAD_PIN_LEDS_CS, 0);
    spi_write_blocking(spi0, buffer, sizeof(buffer));
    gpio_put(KEYPAD_PIN_LEDS_CS, 1);
}

void keypad_set_brightness(float brightness) {
    if (brightness < 0.0 || brightness > 1.0) return;
    for (uint16_t i = 0 ; i < NUM_PADS ; ++i) {
        led_data[i * 4] = 0b11100000 | (uint8_t)(brightness * (float)0b11111);
    }
}

void keypad_set_led_at(uint8_t x, uint8_t y, uint8_t r, uint8_t g, uint8_t b) {
    if (x < 0 || x >= KEYPAD_WIDTH || y < 0 || y >= KEYPAD_HEIGHT) return;
    keypad_set_leds((x + (y * KEYPAD_WIDTH)) * 4, r, g, b);
}

void keypad_set_led(uint8_t i, uint8_t r, uint8_t g, uint8_t b) {
    if (i < 0 || i >= NUM_PADS) return;
    keypad_set_leds(i * 4, r, g, b);
}

void keypad_set_leds(uint16_t o, uint8_t r, uint8_t g, uint8_t b) {
    led_data[o + 0] = 0xFF;
    led_data[o + 1] = b;
    led_data[o + 2] = g;
    led_data[o + 3] = r;
}

void keypad_set_all(uint8_t r, uint8_t g, uint8_t b) {
    for (uint16_t i = 0 ; i < NUM_PADS ; ++i) {
        keypad_set_led(i, r, g, b);
    }
}

void keypad_clear() {
    for (uint16_t i = 0 ; i < NUM_PADS ; ++i) {
        keypad_set_led(i, 0, 0, 0);
    }
}

uint16_t keypad_get_button_states() {
    uint8_t i2c_read_buffer[2];
    uint8_t reg = 0;
    i2c_write_blocking(i2c0, KEYPAD_I2C_ADDRESS, &reg, 1, true);
    i2c_read_blocking(i2c0, KEYPAD_I2C_ADDRESS, i2c_read_buffer, 2, false);
    return ~((i2c_read_buffer[0]) | (i2c_read_buffer[1] << 8));
}