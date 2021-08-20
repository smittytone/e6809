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

    test_addressing();
    test_alu();
    test_index();
    test_logic();
    test_reg();
    test_branch();

    printf("Tests: %i\n", tests);
    printf("Passes: %i\n", passes);
    printf("Fails: %i\n", errors);
    printf("--------------------------------------------------\n");
}


void test_addressing() {

    uint16_t result;

    // Immediate
    test_setup();
    reg.pc = 0x00FF;
    mem[0x00FF] = 0x0A;
    result = address_from_mode(MODE_IMMEDIATE);
    if (mem[result] == 0x0A) {
        passes++;
    } else {
        errors++;
    }

    // Direct
    test_setup();
    reg.pc = 0x00FF;
    reg.dp = 0x20;
    mem[0x00FF] = 0x0A;
    mem[0x200A] = 0x0C;
    result = address_from_mode(MODE_DIRECT);
    if (mem[result] == 0x0C) {
        passes++;
    } else {
        errors++;
    }

    // Extended
    test_setup();
    reg.pc = 0x00FF;
    reg.dp = 0x20;
    mem[0x00FF] = 0x0A;
    mem[0x0100] = 0x0C;
    mem[0x0A0C] = 0x42;
    result = address_from_mode(MODE_EXTENDED);
    if (mem[result] == 0x42) {
        passes++;
    } else {
        errors++;
    }
}


void test_alu() {

    uint8_t result;

    // ADC
    // Zaks p.122
    test_setup();
    reg.cc = 0x0B;
    result = add_with_carry(0x14, 0x22);
    if (result == 0x37 && reg.cc == 0x00) {
        passes++;
    } else {
        errors++;
    }

    // Leventhal p.22-3
    test_setup();
    reg.cc = 0x01;
    result = add_with_carry(0x3A, 0x7C);
    if (result == 0xB7 && reg.cc == 0x2A) {
        passes++;
    } else {
        errors++;
    }

    // ADD 8-bit
    // Zaks p.123
    test_setup();
    reg.cc = 0x13;
    result = add_no_carry(0xF2, 0x39);
    if (result == 0x2B && reg.cc == 0x11) {
        passes++;
    } else {
        errors++;
    }

    // Leventhal p.22-3
    test_setup();
    result = add_no_carry(0x24, 0x8B);
    if (result == 0xAF && reg.cc == 0x08) {
        passes++;
    } else {
        errors++;
    }

    // ADD 16-bit
    // Zaks p.124
    test_setup();
    reg.a = 0x00;
    reg.b = 0x0F;
    reg.pc = 0x00;
    mem[0] = 0x03;
    mem[1] = 0x22;
    add_16(0x00, MODE_IMMEDIATE);    // First arg, op, is not used
    if (reg.a == 0x03 && reg.b == 0x31 && reg.cc == 0x00) {
        passes++;
    } else {
        errors++;
    }

    // Leventhal p.22-6
    test_setup();
    reg.a = 0x10;
    reg.b = 0x55;
    reg.pc = 0x00;
    mem[0] = 0x10;
    mem[1] = 0x11;
    add_16(0x00, MODE_IMMEDIATE);    // First arg, op, is not used
    if (reg.a == 0x20 && reg.b == 0x66 && reg.cc == 0x00) {
        passes++;
    } else {
        errors++;
    }

    // CMP 8-bit
    // Leventhal p.22-28
    test_setup();
    compare(0xF6, 0x18);
    if (reg.cc == 0x08) {
        passes++;
    } else {
        errors++;
    }

    // Zaks p.150
    test_setup();
    reg.cc = 0x52;
    compare(0x05, 0x06);
    if (reg.cc == 0x59) {
        passes++;
    } else {
        errors++;
    }

    // CMP 16-bit
    // Zaks p.151
    test_setup();
    reg.cc = 0x23;
    reg.x = 0x5410;
    reg.pc = 0x00FF;
    mem[0x00FF] = 0x3B;
    mem[0x0100] = 0x33;
    mem[0x3B33] = 0x54;
    mem[0x3B34] = 0x10;
    cmp_16(CMPX_immed, MODE_EXTENDED, 0x00);
    if (reg.cc == 0x24 && reg.x == 0x5410) {
        passes++;
    } else {
        errors++;
    }

    // Leventhal p.22-29
    test_setup();
    reg.x = 0x1AB0;
    reg.pc = 0x00FF;
    mem[0x00FF] = 0xA4;
    mem[0x0100] = 0xF1;
    mem[0xA4F1] = 0x1B;
    mem[0xA4F2] = 0xB0;
    cmp_16(CMPX_immed, MODE_EXTENDED, 0x00);
    if ((reg.cc & 0x0F) == 0x09 && reg.x == 0x1AB0) {
        passes++;
    } else {
        errors++;
    }

    // COM
    // Leventhal p.22-30
    test_setup();
    result = complement(0x23);
    if (result == 0xDC && reg.cc == 0x09) {
        passes++;
    } else {
        errors++;
    }

    // Zaks p.152
    test_setup();
    reg.cc = 0x04;
    result = complement(0x9B);
    if (result == 0x64 && reg.cc == 0x01) {
        passes++;
    } else {
        errors++;
    }

    // DAA
    // Leventhal p.22-32
    test_setup();
    reg.a = add_no_carry(0x39, 0x47);
    daa();
    if (reg.a == 0x86) {
        passes++;
    } else {
        errors++;
    }

    // Zaks p.154
    // Bad example

    // DEC
    // Zaks p.155
    test_setup();
    reg.cc = 0x35;
    result = decrement(0x32);
    if (result == 0x31 && reg.cc == 0x31) {
        passes++;
    } else {
        errors++;
    }

    // Leventhal p.22-33
    test_setup();
    reg.cc = 0xFF;
    result = decrement(0x3A);
    if (result == 0x39 && reg.cc == 0xF1) {
        passes++;
    } else {
        errors++;
    }

    // INC
    // Zaks p.158
    test_setup();
    result = increment(0x35);
    if (result == 0x36 && reg.cc == 0x00) {
        passes++;
    } else {
        errors++;
    }

    test_setup();
    result = increment(0x7F);
    if (result == 0x80 && reg.cc == 0x0A) {
        passes++;
    } else {
        errors++;
    }

    // Leventhal p.22-38
    test_setup();
    reg.cc = 0x01;
    result = increment(0xC0);
    if (result == 0xC1 && reg.cc == 0x09) {
        passes++;
    } else {
        errors++;
    }

    // MUL
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
    // Zaks p.121
    test_setup();
    reg.x = 0x8006;
    reg.b = 0xCE;
    abx();
    if (reg.x == 0x80D4) {
        passes++;
    } else {
        errors++;
    }

    // Leventhal p.22-2
    test_setup();
    reg.x = 0x1097;
    reg.b = 0x84;
    abx();
    if (reg.x == 0x111B) {
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
    // Zaks p.125
    test_setup();
    reg.cc = 0x32;
    result = do_and(0x8B, 0x0F);
    if (result == 0x0B && reg.cc == 0x30) {
        passes++;
    } else {
        errors++;
    }

    // Leventhal p.22-7
    test_setup();
    reg.cc = 0x0F;
    result = do_and(0xFC, 0x13);
    if (result == 0x10 && reg.cc == 0x01) {
        passes++;
    } else {
        errors++;
    }

    // ANDCC
    // Zaks p.126
    test_setup();
    reg.cc = 0x79;
    andcc(0xAF);
    if (reg.cc == 0x29) {
        passes++;
    } else {
        errors++;
    }

    // Leventhal p.22-8
    test_setup();
    reg.cc = 0xD4;
    andcc(0xBF);
    if (reg.cc == 0x94) {
        passes++;
    } else {
        errors++;
    }

    // ASL/LSL
    // Zaks p.127
    test_setup();
    reg.cc = 0x04;
    result = logic_shift_left(0xA5);
    if (result == 0x4A && reg.cc == 0x03) {
        passes++;
    } else {
        errors++;
    }

    // Zaks p.127
    test_setup();
    result = logic_shift_left(0xB8);
    if (result == 0x70 && reg.cc == 0x03) {
        passes++;
    } else {
        errors++;
    }

    // Leventhal p.22-9
    test_setup();
    result = logic_shift_left(0x7A);
    if (result == 0xF4 && reg.cc == 0x0A) {
        passes++;
    } else {
        errors++;
    }

    // ASR
    // Zaks p.128
    test_setup();
    result = arith_shift_right(0xE5);
    if (result == 0xF2 && reg.cc == 0x09) {
        passes++;
    } else {
        errors++;
    }

    // Leventhal p22-10
    test_setup();
    result = arith_shift_right(0xCB);
    if (result == 0xE5 && reg.cc == 0x09) {
        passes++;
    } else {
        errors++;
    }

    // BIT
    // Leventhal p.22-16
    test_setup();
    reg.cc = 0xFF;
    reg.a = 0xA6;
    mem[0] = 0xE0;
    bit(BITA_immed, MODE_IMMEDIATE);
    if (reg.a = 0xA6 && reg.cc == 0xF9) {
        passes++;
    } else {
        errors++;
    }

    // CLR
    // Zaks p.149
    test_setup();
    reg.a = 0xE2;
    clr(CLRA, MODE_INHERENT);
    if (reg.a == 0x00 && reg.cc == 0x04) {
        passes++;
    } else {
        errors++;
    }

    // Leventhal p.22-27
    test_setup();
    reg.a = 0x43;
    clr(CLRA, MODE_INHERENT);
    if (reg.a == 0x00 && reg.cc == 0x04) {
        passes++;
    } else {
        errors++;
    }

    // EOR
    // Zaks p.156
    test_setup();
    reg.cc = 0x03;
    result = do_xor(0xF2, 0x98);
    if (result == 0x6A && reg.cc == 0x01) {
        passes++;
    } else {
        errors++;
    }

    // Leventhal p.22-35
    test_setup();
    reg.cc = 0xFF;
    result = do_xor(0xE3, 0xA0);
    if (result == 0x43 && reg.cc == 0xF1) {
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

    // CWAI
    // TODO

    // EXG
    // Zaks p.157
    test_setup();
    reg.a = 0x42;
    reg.dp = 0x00;
    transfer_decode2(0x8B, true);
    if (reg.a == 0x00 && reg.dp == 0x42) {
        passes++;
    } else {
        errors++;
    }

    // Leventhal p.22-36
    test_setup();
    reg.a = 0x7E;
    reg.b = 0xA5;
    transfer_decode2(0x89, true);
    if (reg.a == 0xA5 && reg.b == 0x7E) {
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


void test_branch() {

    // JMP
    test_setup();
    reg.pc = 0x00FF;
    mem[0x00FF] = 0xAA;
    mem[0x0100] = 0xAA;
    mem[0xAAAA] = 0xFF;
    mem[0xAAAB] = 0xFF;
    jmp(MODE_EXTENDED);
    if (reg.pc == 0xFFFF) {
        passes++;
    } else {
        errors++;
    }

    // Leventhal p.22-40
    test_setup();
    reg.pc = 0x00FF;
    mem[0x00FF] = 0x3A;
    mem[0x0100] = 0x05;
    mem[0x3A05] = 0xD1;
    mem[0x3A06] = 0xE5;
    jmp(MODE_EXTENDED);
    if (reg.pc == 0xD1E5) {
        passes++;
    } else {
        errors++;
    }

    // Zaks p.159
    test_setup();
    reg.pc = 0x3041;
    reg.x = 0xB290;
    mem[0x00FF] = 0x84;
    jmp(MODE_INDEXED);
    if (reg.pc == 0xB290) {
        passes++;
    } else {
        errors++;
    }
}


void test_setup() {
    tests++;
    reset_registers();
}