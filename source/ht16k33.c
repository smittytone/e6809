/*
 * e6809 for Raspberry Pi Pico
 * Display driver
 *
 * @version     0.0.2
 * @author      smittytone
 * @copyright   2024
 * @licence     MIT
 *
 */

// Pico
#include "hardware/i2c.h"
// App
#include "ht16k33.h"


/*
 * STATICS
 */
static void ht16k33_power(uint8_t address, uint8_t on);


/*
 * GLOBALS
 */
const uint8_t CHARSET[18] = {0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 
                             0x6F, 0x77, 0x7C, 0x39, 0x5E, 0x79, 0x71, 0x40, 0x63};
const uint8_t POS[4] = {0, 2, 6, 8};
// 0x5F, 0x7C, 0x58, 0x5E, 0x7B, 0x71


/*
 * I2C FUNCTIONS
 */

/**
 * @brief Convenience function to write a single byte to the matrix.
 */
void i2c_write_byte(uint8_t address, uint8_t byte) {

    i2c_write_blocking(I2C_PORT, address, &byte, 1, false);
}

/**
 * @brief Convenience function to write a 'count' bytes to the matrix
 */
void i2c_write_block(uint8_t address, uint8_t *data, uint8_t count) {

    i2c_write_blocking(I2C_PORT, address, data, count, false);
}


/*
 * HT16K33 SEGMENT LED FUNCTIONS
 */

/**
 * @brief Initialise the display.
 *        NOTE Assumes the display is on I2C0.
 * @param address: The display's I2C address.
 * @param buffer:  Pointer to the display code's data buffer.
 */
void ht16k33_init(uint8_t address, uint8_t *buffer) {

    ht16k33_power(address, 1);
    ht16k33_brightness(address, 6);
    ht16k33_clear(address, buffer);
    ht16k33_draw(address, buffer);
}

/**
 * @brief Power the display on or off.
 *
 * @Param address: The display's I2C address.
 * @Param on:      Whether to power up the display (`true`) or turn it off (`false`).
 */
static void ht16k33_power(uint8_t address, uint8_t on) {

    i2c_write_byte(address, on == ON ? HT16K33_GENERIC_SYSTEM_ON : HT16K33_GENERIC_DISPLAY_OFF);
    i2c_write_byte(address, on == ON ? HT16K33_GENERIC_DISPLAY_ON : HT16K33_GENERIC_SYSTEM_OFF);
}

/**
 * @brief Power the display on or off.
 *
 * @param address:    The display's I2C address.
 * @param brightness: The brightness value, 1-15.
 */
void ht16k33_brightness(uint8_t address, uint8_t brightness) {

    // Set the LED brightness
    if (brightness < 0 || brightness > 15) brightness = 15;
    i2c_write_byte(address, HT16K33_GENERIC_CMD_BRIGHTNESS | brightness);
}

/**
 * @brief Clear the display.
 *        Writes to the buffer, but not the device: call `draw()` after.
 *
 * @param address: The display's I2C address.
 * @param buffer:  Pointer to the display code's data buffer.
 */
void ht16k33_clear(uint8_t address, uint8_t *buffer) {

    // Clear the display buffer and then write it out
    for (uint8_t i = 0 ; i < 16 ; ++i) buffer[i] = 0;
}

/**
 * @brief Writes the buffer to the device.
 *
 * @param address: The display's I2C address.
 * @param buffer:  Pointer to the display code's data buffer.
 */
void ht16k33_draw(uint8_t address, uint8_t *buffer) {

    // Set up the buffer holding the data to be
    // transmitted to the LED
    uint8_t tx_buffer[17];
    for (uint8_t i = 0 ; i < 17 ; ++i) tx_buffer[i] = 0;

    // Span the 8 bytes of the graphics buffer
    // across the 16 bytes of the LED's buffer
    for (uint8_t i = 0 ; i < 16 ; ++i) {
        tx_buffer[i + 1] = buffer[i];
    }

    // Write out the transmit buffer
    i2c_write_block(address, tx_buffer, sizeof(tx_buffer));
}

/**
 * @brief Set the specified digit to a hex number.
 *
 * @param address: The display's I2C address.
 * @param buffer:  Pointer to the display code's data buffer.
 * @param number:  The value to show, 0x00-0x0F.
 * @param digit:   The target display digit, 0-3.
 * @param has_dot: Illuminate the decimal point (`true`) or not (`false`).
 */
void ht16k33_set_number(uint8_t address, uint8_t *buffer, uint16_t number, uint8_t digit, bool has_dot) {

    if (digit > 3) return;
    if (number > 15) return;
    if (number < 10) ht16k33_set_alpha(address, buffer, '0' + number, digit, has_dot);
    if (number > 9) ht16k33_set_alpha(address, buffer, 'a' + (number - 10), digit, has_dot);
}

/**
 * @brief Set the specified digit to a character from the driver's
 *        character set -- see `CHARSET`, above.
 *
 * @param address: The display's I2C address.
 * @param buffer:  Pointer to the display code's data buffer.
 * @param chr:     The character to show, from the CHARSET.
 * @param digit:   The target display digit, 0-3.
 * @param has_dot: Illuminate the decimal point (`true`) or not (`false`).
 */
void ht16k33_set_alpha(uint8_t address, uint8_t *buffer, char chr, uint8_t digit, bool has_dot) {

    if (digit > 3) return;

    uint8_t char_val = 0xFF;
    if (chr == ' ') {
        char_val = HT16K33_SEGMENT_SPACE_CHAR;
    } else if (chr == '-') {
        char_val = HT16K33_SEGMENT_MINUS_CHAR;
    } else if (chr == 'o') {
        char_val = HT16K33_SEGMENT_DEGREE_CHAR;
    } else if (chr >= 'a' && chr <= 'f') {
        char_val = (uint8_t)chr - 87;
    } else if (chr >= '0' && chr <= '9') {
        char_val = (uint8_t)chr - 48;
    }

    if (char_val == 0xFF) return;
    buffer[POS[digit]] = CHARSET[char_val];
    if (has_dot) buffer[POS[digit]] |= 0x80;
}

/**
 * @brief Set the specified digit to an arbitrary glyph, one bit per segment.
 *
 * @param address: The display's I2C address.
 * @param buffer:  Pointer to the display code's data buffer.
 * @param glyph:   The glyph value, 0x00-0x7F.
 * @param digit:   The target display digit, 0-3.
 * @param has_dot: Illuminate the decimal point (`true`) or not (`false`).
 */
void ht16k33_set_glyph(uint8_t address, uint8_t *buffer, uint8_t glyph, uint8_t digit, bool has_dot) {

    if (glyph > 0x7F) return;
    buffer[POS[digit]] = glyph;
    if (has_dot) buffer[POS[digit]] |= 0x80;
}

/**
 * @brief Show or hide the display's colon symbol.
 *
 * @param address: The display's I2C address.
 * @param buffer:  Pointer to the display code's data buffer.
 * @param how:    Show (`true`) or hide (`false`) the colon.
 */
void ht16k33_show_colon(uint8_t address, uint8_t *buffer, bool show) {

    buffer[HT16K33_SEGMENT_COLON_ROW] = (show ? 0x02 : 0x00);
}
