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


uint32_t errors = 0;
uint32_t passes = 0;
uint32_t tests = 0;


void test_main() {
    tests = 0;
    errors = 0;
    passes = 0;

    test_alu();
    test_index();
    test_logic();
    test_reg();

    printf("Tests: %i\n", tests);
    printf("Passes: %i\n", passes);
    printf("Fails: %i\n", errors);
    printf("--------------------------------------------------\n");
}


void test_alu() {

    uint8_t result;

    // ADC
    test_setup();
    reg.cc = 0x0B;
    result = add_with_carry(0x14, 0x22);
    if (result == 0x37 && reg.cc == 0x00) {
        passes++;
    } else {
        errors++;
    }

    // ADD
    test_setup();
    reg.cc = 0x13;
    result = add_no_carry(0xF2, 0x39);
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

    test_setup();
    reg.cc = 0x52;
    compare(0x05, 0x06);
    if (reg.cc == 0x59) {
        passes++;
    } else {
        errors++;
    }

    // COMP
    test_setup();
    result = complement(0x23);
    if (result == 0xDC && reg.cc == 0x09) {
        passes++;
    } else {
        errors++;
    }

    test_setup();
    reg.cc = 0x04;
    result = complement(0x9B);
    if (result == 0x64 && reg.cc == 0x01) {
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

    test_setup();
    reg.a = 0x7F;
    daa();
    if (reg.a == 0x85 && reg.cc == 0x08) {
        passes++;
    } else {
        errors++;  //KNWON
    }

    // DEC
    test_setup();
    reg.cc = 0x35;
    result = decrement(0x32);
    if (result == 0x31 && reg.cc == 0x31) {
        passes++;
    } else {
        errors++;
    }

    // INC
    test_setup();
    result = increment(0x35);
    if (result == 0x36 && reg.cc == 0x00) {
        passes++;
    } else {
        errors++;
    }

    test_setup();
    reg.cc = 0x00;
    result = increment(0x7F);
    if (result == 0x80 && reg.cc == 0x0A) {
        passes++;
    } else {
        errors++;
    }

    // mUL
    test_setup();
    reg.a = 0x0C;
    reg.b = 0x64;
    mul();
    if (reg.a == 0x04 && reg.b == 0xB0 && (reg.cc & 0x01) == 0x01) {
        passes++;
    } else {
        errors++;
    }

    // NEG
    test_setup();
    reg.cc = 34;
    result = negate(0xF3, false);
    // NOTE Zaks CC values weird and wrong
    if (result == 0x0D && reg.cc == 0x21) {
        passes++;
    } else {
        errors++;
    }

    // SBC

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
        errors++;
    }

    // LEA
    test_setup();
    reg.pc = 0;
    mem[0] = 0x4A;
    reg.u = 0x0455;
    lea(LEAU_indexed);
    if (reg.u == 0x045F) {
        passes++;
    } else {
        errors++;
    }
}


void test_logic() {
    uint8_t result;

    // AND
    test_setup();
    reg.cc = 0x32;
    result = do_and(0x8B, 0x0F);
    if (result == 0x0B && reg.cc == 0x30) {
        passes++;
    } else {
        errors++;
    }

    // ANDCC
    test_setup();
    reg.cc = 0x79;
    andcc(0xAF);
    if (reg.cc == 0x29) {
        passes++;
    } else {
        errors++;
    }

    // ASL/LSL
    test_setup();
    reg.cc = 0x04;
    result = logic_shift_left(0xA5);
    if (result == 0x4A && reg.cc == 0x03) {
        passes++;
    } else {
        errors++;
    }

    test_setup();
    result = logic_shift_left(0xB8);
    if (result == 0x70 && reg.cc == 0x03) {
        passes++;
    } else {
        errors++;
    }

    // ASR
    test_setup();
    result = arith_shift_right(0xE5);
    if (result == 0xF2 && reg.cc == 0x09) {
        passes++;
    } else {
        errors++;
    }

    // BIT
    /*
    test_setup();
    result = bit(BITA_immed, MODE_IMMEDIATE);
    if (result == 0xF2 && reg.cc == 0x09) {
        passes++;
    } else {
        errors++;
    }
    */

    // CLR
    test_setup();
    reg.a = 0xE2;
    clr(CLRA, MODE_INHERENT);
    if (reg.a == 0x00 && reg.cc == 0x04) {
        passes++;
    } else {
        errors++;
    }

    // EOR
    test_setup();
    reg.cc = 0x03;
    result = do_xor(0xF2, 0x98);
    if (result == 0x6A && reg.cc == 0x01) {
        passes++;
    } else {
        errors++;
    }

    // LSR
    test_setup();
    reg.cc = 0x0F;
    result = logic_shift_right(0x3E);
    // NOTE Error in Zaks: CC is 0x02 not 0x00
    if (result == 0x1F && reg.cc == 0x02) {
        passes++;
    } else {
        errors++;
    }

    // OR
    test_setup();
    reg.cc = 0x43;
    result = do_or(0xDA, 0x0F);
    if (result == 0xDF && reg.cc == 0x49) {
        passes++;
    } else {
        errors++;
    }

    // ORCC
    test_setup();
    reg.cc = 0x13;
    orcc(0x50);
    if (reg.cc == 0x53) {
        passes++;
    } else {
        errors++;
    }

    // ROL
    test_setup();
    reg.cc = 0x09;
    result = rotate_left(0x89);
    // NOTE Error in Zaks: CC is 0x02 not 0x00
    if (result == 0x13 && reg.cc == 0x03) {
        passes++;
    } else {
        errors++;
    }

    // ROR
    test_setup();
    reg.cc = 0x09;
    result = rotate_right(0x89);
    // NOTE Error in Zaks: CC is 0x02 not 0x00
    if (result == 0xC4 && reg.cc == 0x09) {
        passes++;
    } else {
        errors++;
    }
}


void test_reg() {

    // EXG
    test_setup();
    reg.a = 0x42;
    reg.dp = 0x00;
    transfer_decode2(0x8B, true);
    if (reg.a == 0x00 && reg.dp == 0x42) {
        passes++;
    } else {
        errors++;
    }

    // LD
    test_setup();
    reg.cc = 0x13;
    reg.pc = 0x00;
    mem[0x0000] = 0xee;
    mem[0x0001] = 0x01;
    mem[0xee01] = 0xF2;
    ld(LDA_extended, MODE_EXTENDED);
    if (reg.a == 0xF2 && reg.cc == 0x19 && ((reg.cc & 0x02) == 0x00)) {
        passes++;
    } else {
        errors++;
    }

    // PSHS
    test_setup();
    reg.s = 0x00;
    reg.a = 1;
    reg.b = 2;
    reg.cc = 0xFF;
    reg.dp = 4;
    reg.x = 0x8008;
    reg.y = 0x8010;
    reg.u = 0x8020;
    reg.u = 0x8040;
    uint8_t rmem[13] = {0,0,0,0,0,0,0,0,0,0,0,0,0};
    push(true, 0xFF);
    for (uint16_t i = reg.s ; i == 0 ; i++) {
        rmem[i - reg.s] = mem[i];
    }

    if (reg.s == 65524) {
        passes++;
    } else {
        errors++;
    }

    // SEX
    test_setup();
    reg.b = 0xE6;
    sex();
    if (reg.a == 0xFF && reg.b == 0xE6) {
        passes++;
    } else {
        errors++;
    }

}


void test_setup() {
    tests++;
    reset_registers();
}