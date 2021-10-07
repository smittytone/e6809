/*
 * e6809 for Raspberry Pi Pico
 *
 * @version     1.0.0
 * @author      smittytone
 * @copyright   2021
 * @licence     MIT
 *
 */
#ifndef _KEYPAD_HEADER_
#define _KEYPAD_HEADER_


/*
 * CONSTANTS
 */
#define DEFAULT_BRIGHTNESS                  0.5f
#define KEYPAD_WIDTH                        4
#define KEYPAD_HEIGHT                       4
#define NUM_PADS                            16

#define KEYPAD_I2C_ADDRESS                  0x20
#define KEYPAD_PIN_KEYS_SDA                 4
#define KEYPAD_PIN_KEYS_SCL                 5
#define KEYPAD_PIN_LEDS_CS                  17
#define KEYPAD_PIN_LEDS_SCK                 18
#define KEYPAD_PIN_LEDS_TX                  19


/*
 * PROTOTYPES
 */
void    keypad_init();
void    keypad_update_leds();
void    keypad_set_brightness(float brightness);

void    keypad_set_led_at(uint8_t x, uint8_t y, uint8_t r, uint8_t g, uint8_t b);
void    keypad_set_led(uint8_t i, uint8_t r, uint8_t g, uint8_t b);
void    keypad_set_leds(uint16_t o, uint8_t r, uint8_t g, uint8_t b);
void    keypad_set_all(uint8_t r, uint8_t g, uint8_t b);
void    keypad_clear();

uint16_t keypad_get_button_states();


#endif  // _KEYPAD_HEADER_