/*
 * e6809 for Raspberry Pi Pico
 * Display driver
 *
 * @version     1.0.0
 * @author      smittytone
 * @copyright   2021
 * @licence     MIT
 *
 */
#ifndef _HT16K33_HEADER_
#define _HT16K33_HEADER_


/*
 *      Constants
 */
#define I2C_PORT                                i2c0
#define I2C_FREQUENCY                           400000
#define ON                                      1
#define OFF                                     0
#define SDA_GPIO                                20
#define SCL_GPIO                                21

#define HT16K33_GENERIC_DISPLAY_ON              0x81
#define HT16K33_GENERIC_DISPLAY_OFF             0x80
#define HT16K33_GENERIC_SYSTEM_ON               0x21
#define HT16K33_GENERIC_SYSTEM_OFF              0x20
#define HT16K33_GENERIC_DISPLAY_ADDRESS         0x00
#define HT16K33_GENERIC_CMD_BRIGHTNESS          0xE0
#define HT16K33_GENERIC_CMD_BLINK               0x81
#define HT16K33_SEGMENT_COLON_ROW               0x04
#define HT16K33_SEGMENT_MINUS_CHAR              0x10
#define HT16K33_SEGMENT_DEGREE_CHAR             0x11
#define HT16K33_SEGMENT_SPACE_CHAR              0x00
#define HT16K33_SEGMENT_COLON_ROW               0x04


/*
 *      PROTOTYPES
 */
// I2C Functions
void        i2c_write_byte(uint8_t address, uint8_t byte);
void        i2c_write_block(uint8_t address, uint8_t *data, uint8_t count);

// Display Functions
void        ht16k33_init(uint8_t address, uint8_t *buffer);
void        ht16k33_power(uint8_t address, uint8_t on);
void        ht16k33_brightness(uint8_t address, uint8_t brightness);
void        ht16k33_clear(uint8_t address, uint8_t *buffer);
void        ht16k33_draw(uint8_t address, uint8_t *buffer);
void        ht16k33_set_number(uint8_t address, uint8_t *buffer, uint16_t number, uint8_t digit, bool has_dot);
void        ht16k33_set_alpha(uint8_t address, uint8_t *buffer, char chr, uint8_t digit, bool has_dot);
void        ht16k33_set_glyph(uint8_t address, uint8_t *buffer, uint8_t glyph, uint8_t digit, bool has_dot);
void        ht16k33_show_colon(uint8_t address, uint8_t *buffer, bool show);


#endif  // _HT16K33_HEADER_