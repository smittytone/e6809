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


void test(main) {
    tests = 0;
    errors = 0;
    passes = 0;

    test_alu();
    test_index();
    test_logic();

    printf("Tests: %i\n", tests);
    printf("Passes: %i\n", passes);
    printf("Fails: %i\n", errors);
    printf("--------------------------------------------------\n");
}


void test_alu() {

    // ADC
    test_setup();
    reg.cc = 0x0B;
    uint8_t result = add_with_carry(0x14, 0x22);
    if (result == 0x36 && reg.cc == 0x00) {
        passes++;
    } else {
        errors++;
    }

    // ADD
    test_setup();
    reg.cc = 0x13;
    uint8_t result = add_no_carry(0xF2, 0x39);
    if (result == 0x2B && reg.cc == 0x11) {
        passes++;
    } else {
        errors++;
    }

    // CMP
    test_setup();
    compare(0xF6, 0x18);
    if (reg.cc == 0x08) {
        passes++;
    } else {
        errors++;
    }

    // COMP
    test_setup();
    uint8_t result = complement(0x23);
    if (result == 0xDC && reg.cc == 0x09) {
        passes++;
    } else {
        errors++;
    }

    // DAA
    test_setup();
    reg.a = add_no_carry(0x39, 0x47);
    daa();
    if (reg.a == 0x86) {
        passes++;
    } else {
        errors++;
    }
}


void test_index() {

    // ABX
    test_setup();
    reg.x = 0x8006;
    reg.b = 0xCE;
    abx();
    if (reg.x == 0x80D4) {
        passes++;
    } else {
        errors++:
    }
}

void test_logic() {

    // AND
    test_setup();
    reg.cc = 0x32;
    uint8_t result = do_and(0x8B, 0x0F);
    if (result == 0x0B && reg.cc == 0x30) {
        passes++;
    } else {
        errors++:
    }

    // ANDCC
    test_setup();
    reg.cc = 0x79;
    andcc(0xAF);
    if (reg.cc == 0x29) {
        passes++;
    } else {
        errors++:
    }

    // ANDCC
    test_setup();
    reg.cc = 0x04;
    arith
    if (reg.cc == 0x29) {
        passes++;
    } else {
        errors++:
    }

    // ASL/LSL
    test_setup();
    reg.cc = 0x04;
    uint8_t result = logic_shift_left(0xA5);
    if (result == 0x4A && reg.cc == 0x03) {
        passes++;
    } else {
        errors++:
    }

    // ASR
    test_setup();
    uint8_t result = arith_shift_right(0xE5);
    if (result == 0xF2 && reg.cc == 0x09) {
        passes++;
    } else {
        errors++:
    }

    // CLR
    test_setup();
    reg.a = 0xE2;
    clr(CLRA, MODE_INHERENT);
    if (reg.a == 0x00 && reg.cc == 0x04) {
        passes++;
    } else {
        errors++:
    }

}


void test_setup() {
    tests++;
    reg.cc = 0x00;
}