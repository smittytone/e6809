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


void alu_tests() {
    uint32_t errors = 0;
    uint32_t passes = 0;
    uint32_t tests = 0;

    // Test 1
    tests++;
    reg.cc = 0x00;
    compare(0xF6, 0x18);
    if (reg.cc == 0x08) {
        passes++;
    } else {
        errors++;
    }

    // Test 2
    tests++;
    reg.cc = 0x00;
    uint8_t result = complement(0x23);
    if (result == 0xDC && reg.cc == 0x09) {
        passes++;
    } else {
        errors++;
    }

    // Test 3
    tests++;
    reg.cc = 0x00;
    reg.a = add_no_carry(0x39, 0x47);
    daa();
    if (reg.a == 0x86) {
        passes++;
    } else {
        errors++;
    }

    printf("Tests: %i\n", tests);
    printf("Passes: %i\n", passes);
    printf("Fails: %i\n", errors);
    printf("--------------------------------------------------\n");
}