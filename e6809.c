/*
 * e6809 for Raspberry Pi Pico
 *
 * @version     1.0.0
 * @author      smittytone
 * @copyright   2021
 * @licence     MIT
 *
 */
#include "e6809.h"


int main() {

    boot();
    return 0;
}


void boot() {
    reset_registers();
}


void loop() {

    while(1) {

    }
}


void process_next_instruction() {

    if (waitForInterrupt) {
        // Process interrupts
        return;
    }

    uint8_t opcode = get_next_byte();
    uint8_t extended_opcode = 0;
    uint8_t address_mode = MODE_UNKNOWN;

    if (opcode == OPCODE_EXTENDED_1 || opcode == OPCODE_EXTENDED_2) {
        extended_opcode = opcode;
        opcode = get_next_byte();
    }

    // Process all ops but NOP
    if (opcode != NOP) {
        uint8_t msn = (opcode & 0xF0) >> 4;
        uint8_t lsn = opcode & 0x0F;

        if (msn == 0x01) {
            // These ops all use inherent addressing
            if (lsn == 0x03) sync();
            if (lsn == 0x06) do_branch(BRA, true);  // Set correct op for LBRA handling
            if (lsn == 0x07) do_branch(BSR, true);  // Set correct op for LBSR handling
            if (lsn == DAA) daa();
            if (lsn == ORCC) orr(reg.cc, get_next_byte());
            if (lsn == ANDCC) and(reg.cc, get_next_byte());
            if (lsn == SEX) sex();
            if (lsn == EXG) transfer_registers(true);
            if (lsn == TFR) transfer_registers(false);
            return;
        }

        if (msn == 0x02) {
            // All 0x02 ops are branch ops
            do_branch(opcode, (extended_opcode != 0));
            return;
        }

        if (msn == 0x03) {
            // These ops have only one, specific address mode each
            if (lsn  < 0x04) lea(opcode);
            if (lsn == 0x04) push(true, get_next_byte());
            if (lsn == 0x06) push(false, get_next_byte());
            if (lsn == 0x05) pull(true, get_next_byte());
            if (lsn == 0x07) pull(false, get_next_byte());
            if (lsn == 0x09) rts();
            if (lsn == 0x0A) abx();
            if (lsn == 0x0B) rti();
            if (lsn == 0x0C) cwai();
            if (lsn == 0x0D) mul();

            // Cover all values of SWIx
            if (lsn == 0x0F) swi(extended_opcode);
            return;
        }

        // Set the address mode as far as we can
        if (msn == 0x08 || msn == 0x0C) address_mode = MODE_IMMEDIATE;
        if (msn == 0x00 || msn == 0x09 || msn == 0x0D) address_mode = MODE_DIRECT;
        if (msn == 0x06 || msn == 0x0A || msn == 0x0E) address_mode = MODE_INDEXED;
        if (msn == 0x07 || msn == 0x0B || msn == 0x0F) address_mode = MODE_EXTENDED;
        if (msn == 0x04 || msn == 0x05) address_mode = MODE_INHERENT;

        // Jump to specific ops or groups of ops
        if (lsn == 0x00) {
            if (msn == 0x07) {
                sub(opcode, address_mode);
            } else {
                neg(opcode, address_mode);
            }

            return;
        }

        if (lsn == 0x01) {
            cmp(opcode, address_mode);
            return;
        }

        if (lsn == 0x02) {
            sbc(opcode, address_mode);
            return;
        }

        if (lsn == 0x03) {
            if (msn > 0x0B) {
                add_16(opcode, address_mode);
            } else if (msn > 0x07) {
                sub_16(opcode, address_mode);
            } else {
                com(opcode, address_mode);
            }

            return;
        }

        if (lsn == 0x04) {
            if (msn < 0x08) {
                lsr(opcode, address_mode);
            } else {
                and(opcode, address_mode);
            }

            return;
        }

        if (lsn == 0x05) {
            bit(opcode, address_mode);
            return;
        }

        if (lsn == 0x06) {
            if (msn < 0x08) {
                ror(opcode, address_mode);
            } else {
                ld(opcode, address_mode);
            }

            return;
        }

        if (lsn == 0x07) {
            if (msn < 0x08) {
                asr(opcode, address_mode);
            } else {
                st(opcode, address_mode);
            }

            return;
        }

        if (lsn == 0x08) {
            if (msn < 0x08) {
                asl(opcode, address_mode);
            } else {
                eor(opcode, address_mode);
            }

            return;
        }

        if (lsn == 0x09) {
            if (msn < 0x08) {
                rol(opcode, address_mode);
            } else {
                adc(opcode, address_mode);
            }

            return;
        }

        if (lsn == 0x0A) {
            if (msn < 0x08) {
                dec(opcode, address_mode);
            } else {
                orr(opcode, address_mode);
            }

            return;
        }

        if (lsn == 0x0B) {
            add(opcode, address_mode);
            return;
        }

        if (lsn == 0x0C) {
            if (msn < 0x08) {
                inc(opcode, address_mode);
            } else if (msn > 0x0C) {
                // LDD
                ld_16(opecode, addres_mode, extended_opcode);
            } else {
                cmp_16(opcode, address_mode, extended_opcode);
            }

            return;
        }

        if (lsn == 0x0D) {
            if (msn < 0x08) {
                tst(opcode, address_mode);
            } else if (msn > 0x0C) {
                // STD
                st_16(opecode, addres_mode, extended_opcode);
            } else {
                jsr(address_mode);
            }

            return;
        }

        if (lsn == 0x0E) {
            if (msn < 0x08) {
                jmp(address_mode);
            } else {
                ld_16(opcode, address_mode, extended_opcode);
            }

            return;
        }

        if (lsn == 0x0F) {
            if (msn < 0x08) {
                clr(opcode, address_mode);
            } else {
                st_16(opcode, address_mode, extended_opcode);
            }
        }
    }
}

void do_branch(uint8_t bop, bool is_long) {

    int32_t offset = 0;

    if (is_long) {
        offset = address_from_next_two_bytes();
        if (is_bit_set(offset, 15)) offset -= 65536;
    } else {
        offset = get_next_byte();
        if (is_bit_set(offset, SIGN_BIT)) offset -= 256;
    }

    bool branch = false;

    if (op == BRA) branch = true;

    if (op == BEQ && is_bit_set(reg.cc, Z_BIT)) branch = true;
    if (op == BNE && is_bit_set(reg.cc, Z_BIT)) branch = true;

    if (op == BMI && is_bit_set(reg.cc, N_BIT)) branch = true;
    if (op == BPL && is_bit_set(reg.cc, N_BIT)) branch = true;

    if (op == BVS && is_bit_set(reg.cc, V_BIT)) branch = true;
    if (op == BVC && is_bit_set(reg.cc, V_BIT)) branch = true;

    if (op == BLO && is_bit_set(reg.cc, C_BIT)) branch = true; // Also BCS
    if (op == BHS && is_bit_set(reg.cc, C_BIT)) branch = true; // Also BCC

    if (op == BGE && (is_bit_set(reg.cc, N_BIT) == is_bit_set(reg.cc, V_BIT)) branch = true;
    if (op == BGT && !is_bit_set(reg.cc, Z_BIT) && is_bit_set(reg.cc, N_BIT)) branch = true;
    if (op == BHI && !is_bit_set(reg.cc, C_BIT) && !is_bit_set(reg.cc, Z_BIT)) branch = true;

    if (op == BLE && (is_bit_set(reg.cc, Z_BIT) || ((is_bit_set(reg.cc, N_BIT) || is_bit_set(reg.cc, V_BIT) && is_bit_set(reg.cc, N_BIT) != is_bit_set(reg.cc, V_BIT))))) branch = true;

    if (op == BLS && (is_bit_set(reg.cc, C_BIT) || is_bit_set(reg.cc, Z_BIT)) branch = true;
    if (op == BLT && (is_bit_set(reg.cc, N_BIT) || is_bit_set(reg.cc, V_BIT)) && is_bit_set(reg.cc, V_BIT)) branch = true;


    if (op == BSR) {
        // Branch to Subroutine: push PC to hardware stack (S) first
        branch = true;
        reg.h--;
        to_ram(reg.h, (reg.pc & 0xFF));
        reg.h--;
        to_ram(reg.h, ((reg.pc >> 8) & 0xFF));
    }

    if (branch) reg.pc += (uint16_t)offset;
}




/*
 * Memory access
 */

uint8_t get_next_byte() {
    return (mem[reg.pc++]);
}

uint8_t get_byte(uint16_t address) {
    return mem[address];
}

void increment_pc(int16_t amount) {
    uint16_t address = reg.pc;
    address += amount;
    reg.pc = address;
}

uint8_t from_ram(uint16_t address) {
    // Returns the byte at the specified addres
    return (mem[address] & 0xFF);
}

void to_ram(uint16_t address, uint8_t value) {
    // Pokes in the 8-but 'value' at the the 16-bit 'address'
    mem[(address & 0xFFFF)] = (value & 0xFF);
}


/*
 * Condition code register bit-level getters and setters
 */

bool is_cc_bit_set(uint8_t bit) {
    return (((reg.cc >> bit) & 1) == 1);
}

void set_cc_bit(uint8_t bit) {
    reg.cc = (reg.cc | (1 << bit));
}

void clr_cc_bit(uint8_t bit) {
    reg.cc = (reg.cc & ~(1 << bit));
}


/*
 * Specific op functions
 */

void abx() {
    // ABX: B + X -> X (unsigned)
    reg.x += (uint16_t)reg.b;
}


void adc(uint8_t op, uint8_t mode) {
    // ADC: A + M + C -> A,
    //      B + M + C -> A
    uint16_t address = address_from_mode(mode);
    if (op < 0xC9) {
        reg.a = add_with_carry(reg.a, from_ram(address));
    } else {
        reg.b = add_with_carry(reg.b, from_ram(address));
    }
}


void add(uint8_t op, uint8_t mode) {
    // ADD: A + M -> A
    //      B + M -> N
    uint16_t address = address_from_mode(mode);
    if (op < 0xCB) {
        reg.a = do_add(reg.a, from_ram(address));
    } else {
        reg.b = do_add(reg.b, from_ram(address));
    }
}


void add_16(uint8_t op, uint8_t mode) {
    // ADDD: D + M:M + 1 -> D
    clr_cc_bit(N_BIT);
    clr_cc_bit(Z_BIT);

    // Cast 'address' as unsigned short so the + 1 correctly rolls over, if necessary
    uint16_t address = address_from_mode(mode);
    uint8_t msb = from_ram(address);
    uint8_t lsb = from_ram(address);

    // Add the two LSBs (M + 1, B) to set the carry,
    // then add the two MSBs (M, A) with the carry
    lsb = alu(reg.b, lsb, false);
    msb = alu(reg.a, msb, true);

    // Convert the bytes back to a 16-bit value and set the CC
    uint16_t answer = (msb << 8) | lsb;
    if (answer == 0) set_cc_bit(Z_BIT);
    if (is_bit_set(answer, 15)) set_cc_bit(N_BIT);

    // Set D's component registers
    reg.a = (answer >> 8) & 0xFF;
    reg.b = answer & 0xFF;
}


void and(uint8_t op, uint8_t mode) {
    // AND: A & M -> A
    //      B & M -> N
    uint16_t address = address_from_mode(mode);
    if (op < 0xC4) {
        reg.a = do_and(reg.a, from_ram(address));
    } else {
        reg.b = do_and(reg.b, from_ram(address));
    }
}


void andcc(uint8_t value) {
    // AND CC: CC & M -> CC
    reg.cc = reg.cc & value;
}


void asl(uint8_t op, uint8_t mode) {
    // ASL: arithmetic shift left A -> A,
    //      arithmetic shift left B -> B,
    //      arithmetic shift left M -> M
    // This is is the same as LSL
    if (mode == MODE_INHERENT) {
        if (op == 0x48) {
            reg.a = logic_shift_left(reg.a);
        } else {
            reg.b = logic_shift_left(reg.b);
        }
    } else {
        uint16_t address = address_from_mode(mode);
        to_ram(address, logic_shift_left(from_ram(address)));
    }
}


void asr(uint8_t op, uint8_t mode) {
    // ASR: arithmetic shift right A -> A,
    //      arithmetic shift right B -> B,
    //      arithmetic shift right M -> M
    if (mode == MODE_INHERENT) {
        if (op == 0x47) {
            reg.a = arith_shift_right(reg.a);
        } else {
            reg.b = arith_shift_right(reg.b);
        }
    } else {
        uint16_t address = address_from_mode(mode);
        to_ram(address, arith_shift_right(from_ram(address)));
    }
}


void bit(uint8_t op, uint8_t mode) {
    // BIT: Bit test A (A & M)
    //      Bit test B (B & M)
    // Does not affect the operands, only the CC register
    uint16_t address = address_from_mode(mode);
    do_and((op < 0xC5 ? reg.a : reg.b), from_ram(address));
}


void clr(uint8_t op, uint8_t mode) {
    // CLR: 0 -> A
    //      0 -> B
    //      0 -> M
    if (mode == MODE_INHERENT) {
        if (op == 0x4F) {
            reg.a = 0;
        } else {
            reg.b = 0;
        }
    } else {
        uint16_t address = address_from_mode(mode);
        to_ram(address, 0);
    }

    set_cc_after_clr();
}


void cmp(uint8_t op, uint8_t mode) {
    // CMP: Compare M to A
    //              M to B
    uint16_t address = address_from_mode(mode);
    compare((op < 0xC1 ? reg.a : reg.b), from_ram(address));
}


void cmp_16(uint8_t op, uint8_t mode, uint8_t ex_op) {
    // CMP: Compare M:M + 1 to D,
    //              M:M + 1 to X,
    //              M:M + 1 to Y,
    //              M:M + 1 to S,
    //              M:M + 1 to U

    // Get the effective address of the data
    uint16_t address = (uint16_t)address_from_mode(mode);

    // 'addressFromMode:' assumes an 8-bit read, so we need to increase PC by 1
    reg.pc++;

    // Set a pointer to the target register
    uint16_t d = (reg.a << 8) + reg.b;
    uint16_t *reg_ptr = &d;
    if (op == 0x83 && ex_op == 0x10) reg_ptr = &reg.u;
    if (op == 0x8C) reg_ptr = ex_op == 0x10 ? &reg.y : (ex_op == 0x11 ? &reg.h : & reg.x);

    // Get the data and subtract from the target register
    uint16_t comp_value = (from_ram(address) << 8) | (from_ram(address + 1));

    // Subtract 'compValue' from the target register - this will set the CC register
    // NOTE ignore the return value - we're just setting the CC bits
    subtract_16(*regPtr, comp_value);
}


void com(uint8_t op, uint8_t mode, uint8_t ex_op) {
    // COM: !A -> A,
    //      !B -> B,
    //      !M -> A
    if (mode == MODE_INHERENT) {
        if (op == 0x43) {
            reg.a = complement(reg.a);
        } else {
            reg.b = complement(reg.b);
        }
    } else {
        uint16_t address = address_from_mode(mode);
        to_ram(address, complement(from_ram(address)));
    }
}


void cwai() {
    // CWAI
    // Clear CC bits and Wait for Interrupt
    // AND CC with the operand, set e, then push every register,
    // including CC to the hardware stack
    reg.cc = do_and(reg.cc, mem[reg.pc]);
    reg.cc = set_cc_bit(E_BIT);
    push(true, PUSH_PULL_EVERY_REG);
    wait_for_interrupt = true;
}


void daa() {
    // DAA: Decimal Adjust A
    bool carry = is_bit_set(reg.cc, C_BIT);

    uint8_t msnb = (reg.a & 0xF0) >> 4;
    uint8_t lsnb = reg.a & 0x0F;

    // Set Least Significant Conversion Factor, which will be either 0 or 6
    uint8_t correction = 0;
    if (is_bit_set(reg.cc, K_BIT) || lsnb > 9) correction = 6;

    // Set Most Significant Conversion Factor, which will be either 0 or 6

    uint8_t clsn = 0;
    if ((lsn > 9) || (is_cc_bit_set(H_BIT))) clsn = 6;
    if ((msn > 9) || (is_cc_bit_set(C_BIT)) || (msn > 8 && lsn > 9)) cmsn = 6;

    lsn += clsn;
    msn += cmsn;

    reg.a = (msn << 4) | lsn;
    set_cc(reg.a, (N_BIT | Z_BIT));
}


void dec(uint8_t op, uint8_t mode) {
    // DEC: A - 1 -> A,
    //      B - 1 -> B,
    //      M - 1 -> M
    if (mode == MODE_INHERENT) {
        if (op == 0x4A) {
            reg.a = decrement(reg.a);
        } else {
            reg.b = decrement(reg.b);
        }
    } else {
        uint16_t address = address_from_mode(mode);
        to_ram(address, decrement(from_ram(address)));
    }
}


void eor(uint8_t op, uint8_t mode) {
    // EOR: A ^ M -> A
    //      B ^ M -> B
    uint16_t address = address_from_mode(mode);
    if (op < 0xC8) {
        reg.a = do_xor(reg.a, from_ram(address));
    } else {
        reg.b = do_xor(reg.b, from_ram(address));
    }
}


void inc(uint8_t op, uint8_t mode) {
    // INC: A + 1 -> A,
    //      B + 1 -> B,
    //      M + 1 -> M
    if (mode == MODE_INHERENT) {
        if (op == 0x4C) {
            reg.a = increment(reg.a);
        } else {
            reg.b = increment(reg.b);
        }
    } else {
        uint16_t address = address_from_mode(mode);
        to_ram(address, increment(from_ram(address)));
    }
}


void jmp(uint8_t mode) {
    // JMP: M -> PC
    reg.pc = address_from_mode(mode);
}


void jsr(uint8_t mode) {
    // JSR: S = S - 1;
    //      PC LSB to stack;
    //      S = S- 1;
    //      PC MSB to stack;
    //      M -> PC
    uint16_t address = address_from_mode(mode);
    reg.s--;
    to_ram(reg.s, (reg.pc & 0xFF));
    reg.s--;
    to_ram(reg.s, ((reg.pc >> 8) & 0xFF));
    reg.pc = address;
}


void ld(uint8_t op, uint8_t mode) {
    // LD: M -> A,
    //     M -> B
    uint16_t address = address_from_mode(mode);
    if (op < 0xC6) {
        reg.a = from_ram(address);
        set_cc_after_load(reg.a, false);
    } else {
        reg.b = from_ram(address);
        set_cc_after_load(reg.b, false);
    }
}


void ld_16(uint8_t op, uint8_t mode, uint8_t ex_op) {
    // LD: M:M + 1 -> D,
    //     M:M + 1 -> X,
    //     M:M + 1 -> Y,
    //     M:M + 1 -> S,
    //     M:M + 1 -> U
    uint16_t address = address_from_mode(mode);

    // 'address_from_mode()' assumes an 8-bit read, so we need to increase PC by 1
    reg.pc++;

    // Set a pointer to the target register
    uint16_t d = 0;
    uint16_t *reg_ptr = &d;
    if (op == 0xCE) reg_ptr = ex_op == 0x10 ? &reg.s : &reg.u;
    if (op == 0x82) reg_ptr = ex_op == 0x10 ? &reg.y : &reg.x;

    // Write the data into the target register
    *reg_ptr = (from_ram(address) << 8) | from_ram(address + 1);

    // Set the A and B registers if we're targeting D
    if (op == 0xCC) {
        reg.a = (d >> 8) & 0xFF;
        reg.b = d & 0xFF;
    }

    // Update the CC register
    set_cc_after_load(*reg_ptr, true);
}


void lea(uint8_t op) {
    // LEA: EA -> S,
    //      EA -> U,
    //      EA -> X,
    //      EA -> Y
    load_effective(indexed_address(get_next_byte()), (op & 0x03));
}


void lsr(uint8_t op, uint8_t mode) {
    // LSR: logic shift right A -> A,
    //      logic shift right B -> B,
    //      logic shift right M -> M
    if (mode == MODE_INHERENT) {
        if (op == 0x44) {
            reg.a = logic_shift_right(reg.a);
        } else {
            reg.b = logic_shift_right(reg.b);
        }
    } else {
        uint16_t address = address_from_mode(mode);
        to_ram(address, logic_shift_right(from_ram(address)));
    }
}


void mul() {
    // MUL: A x B -> D (unsigned)
    clr_cc_bit(Z_BIT);
    clr_cc_bit(C_BIT);

    uint16_t d = reg.a * reg.b;
    if (is_bit_set(d, 7)) set_cc_bit(C_BIT);
    if (d == 0) set_cc_bit(Z_BIT);

    reg.a = (d >> 8) & 0xFF;
    reg.b = d & 0xFF;
}


void neg(uint8_t op, uint8_t mode) {
    // NEG: !R + 1 -> R, !M + 1 -> M
    if (mode == MODE_INHERENT) {
        if (op == 0x40) {
            reg.a = negate(reg.a);
        } else {
            reg.b = negate(reg.b);
        }
    } else {
        uint16_t address = address_from_mode(mode);
        to_ram(address, negate(from_ram(address)));
    }
}


void orr(uint8_t op, uint8_t mode) {
    // OR: A | M -> A
    //     B | M -> B
    uint16_t address = address_from_mode(mode);
    if (op < 0xCA) {
        reg.a = do_or(reg.a, from_ram(address));
    } else {
        reg.b = do_or(reg.b, from_ram(address));
    }
}


void orcc:(uint8_t value) {
    // OR CC: CC | M -> CC
    reg.cc = (reg.cc | value) & 0xFF;
}


void rol(uint8_t op, uint8_t mode) {
    // ROL: rotate left A -> A,
    //      rotate left B -> B,
    //      rotate left M -> M
    if (mode == MODE_INHERENT) {
        if (op == 0x49) {
            reg.a = rotate_left(reg.a);
        } else {
            reg.b = rotate_left(reg.b);
        }
    } else {
        uint16_t address = address_from_mode(mode);
        to_ram(address, rotate_left(from_ram(address)));
    }
}


void ror(uint8_t op, uint8_t mode) {
    // ROR: rotate right A -> A,
    //      rotate right B -> B,
    //      rotate right M -> M
    if (mode == MODE_INHERENT) {
        if (op == 0x46) {
            reg.a = rotate_right(reg.a);
        } else {
            reg.b = rotate_right(reg.b);
        }
    } else {
        uint16_t address = address_from_mode(mode);
        to_ram(address, rotate_right(from_ram(address)));
    }
}


void rti() {
    // RTI
    // Pull CC from the hardware stack; if e is set, pull all the registers
    // from the hardware stack, otherwise pull the PC register only
    pull(true, PUSH_PULL_CC_REG);
    if (is_bit_set(reg.cc, E_BIT)) pull(true, PUSH_PULL_ALL_REGS);
}


void rts() {
    // RTS
    // Pull the PC from the hardware stack
    reg.pc = (mem[reg.s] << 8);
    reg.s++;
    reg.pc += mem[reg.s];
    reg.s++;
}


void sbc(uint8_t op, uint8_t mode) {
    // SBC: A - M - C -> A,
    //      B - M - C -> A
    uint16_t address = address_from_mode(mode);
    if (op < 0xC2) {
        reg.a = sub_with_carry(reg.a, from_ram(address));
    } else {
        reg.b = sub_with_carry(reg.b, from_ram(address));
    }
}


void sex() {
    // SEX: sign-extend B into A
    // Affects n, z
    clr_cc_bit(N_BIT);
    clr_cc_bit(Z_BIT);
    reg.a = 0;
    if (is_bit_set(reg.b, SIGN_BIT_8)) {
        reg.a = 0xFF;
        set_cc_bit(N_BIT);
    }

    if (reg.b == 0) set_cc_bit(Z_BIT); // ****CHECK****
}


void st(uint8_t op, uint8_t mode) {
    // ST: A -> M,
    //     B -> M
    uint16_t address = address_from_mode(mode);
    if (op < 0xD7) {
        to_ram(address, reg.a);
        set_cc_after_store(reg.a, false);
    } else {
        to_ram(address, reg.b);
        set_cc_after_store(reg.b, false);
    }
}


void st_16(uint8_t op, uint8_t mode, uint8_t ex_op) {
    // ST: D -> M:M + 1,
    //     X -> M:M + 1,
    //     Y -> M:M + 1,
    //     S -> M:M + 1,
    //     U -> M:M + 1

    // Get the effective address of the data
    uint16_t address = address_from_mode(mode);

    // Set a pointer to the target register (assume D)
    uint16_t d = (reg.a << 8) | reg.b;
    uint16_t *reg_ptr = &d;
    if (op == 0xDF) reg_ptr = op == 0x10 ? &reg.s : &reg.u;
    if (op == 0x9F) reg_ptr = op == 0x10 ? &reg.y : &reg.x;

    // Write the target register out
    to_ram(address, ((*regPtr >> 8) & 0xFF));
    to_ram(address + 1, (*regPtr & 0xFF));

    // Update the CC register
    set_cc_after_store(*reg_ptr, true);
}


void sub(uint8_t op, uint8_t mode) {
    // SUB: A - M -> A,
    //      B - M -> B
    uint16_t address = address_from_mode(mode);
    if (op < 0x82) {
        reg.a = subtract(reg.a, from_ram(address));
    } else {
        reg.b = subtract(reg.b, from_ram(address));
    }
}


void sub_16(uint8_t op, uint8_t mode, uint8_t ex_op) {
    // SUBD: D - M:M + 1 -> D
    clr_cc_bit(N_BIT);
    clr_cc_bit(Z_BIT);
    clr_cc_bit(V_BIT);

    // Complement the value at M:M + 1
    uint16_t address = address_from_mode(mode);
    uint8_t msb = complement(from_ram(address));
    uint8_t lsb = complement(from_ram(address + 1));

    // Add 1 to form the 2's complement
    lsb = alu(lsb, 1, false);
    msb = alu(msb, 0, true);

    // Add D - A and B
    lsb = alu(reg.b, lsb, false);
    msb = alu(reg.a, msb, true);

    // Convert the bytes back to a 16-bit value and set the CC
    uint16_t answer = (msb << 8) | lsb;
    if (answer == 0) set_cc_bit(Z_BIT);
    if (is_bit_set(answer, 15)) set_cc_bit(N_BIT);

    // Set D's component registers
    reg.a = (answer >> 8) & 0x0FF;
    reg.b = answer & 0xFF;
}


void swi(uint8_t number) {
    // SWI: SoftWare Interrupt
    // Covers 1, 2 and 3

    // Set e to 1 then push every register to the hardware stac
    set_cc_bit(E_BIT);
    push(true, PUSH_PULL_EVERY_REG);
    wait_for_interrupt = false;

    if (number == 1) {
        // Set i and f
        set_cc_bit(I_BIT);
        set_cc_bit(F_BIT);

        // Set the PC to the interrupt vector
        reg.pc = (mem[INTERRUPT_VECTOR_1] << 8) | mem[INTERRUPT_VECTOR_1 + 1];
    }

    if (number == 2) {
        reg.pc = (mem[INTERRUPT_VECTOR_2] << 8) | mem[INTERRUPT_VECTOR_2 + 1];
    }

    if (number == 3) {
        reg.pc = (mem[INTERRUPT_VECTOR_3] << 8) | mem[INTERRUPT_VECTOR_3 + 1];
    }
}


void sync() {
    // SYNC
    wait_for_interrupt = true;
}


void tst(uint8_t op, uint8_t mode) {
    // TST: test A,
    //      test B,
    //      test M
    if (mode == MODE_INHERENT) {
        if (op = 0x4D) {
            test(reg.a);
        } else {
            test(reg.b);
        }
    } else {
        uint16_t address = address_from_mode(mode);
        test(from_ram(address));
    }
}


/*
 * Specific op helper functions
 */
uint8_t alu(uint8_t value_1, uint8_t value_2, bool use_carry) {
    // Simulates addition of two unsigned 8-bit values in a binary ALU
    // Checks for half-carry, carry and overflow (CC bits h, c, v)
    uint8_t binary_1[8], binary_2[8], answer[8];
    bool bit_carry = false;
    bool bit_6_carry = false;

    clr_cc_bit(V_BIT);
    for (uint32_t i = 0 ; i < 8 ; i++) {
        binary_1[i] = ((value_1 >> i) & 0x01);
        binary_2[i] = ((value_2 >> i) & 0x01);
    }

    if (use_carry) bit_carry = is_cc_bit_set(C_BIT);

    for (uint32_t i = 0 ; i < 8 ; i++) {
        if (binary_1[i] == binary_2[i]) {
            // Both digits are the same, ie. 1 and 1, or 0 and 0,
            // so added value is 0 + a carry to the next digit
            answer[i] = bit_carry ? 1 : 0;
            bit_carry = binary_1[i] == 1;
        } else {
            // Both digits are different, ie. 1 and 0, or 0 and 1, so the
            // added value is 1 plus any carry from the previous addition
            // 1 + 1 -> 0 + carry 1, or 0 + 1 -> 1
            answer[i] = bit_carry ? 0 : 1;
        }

        // Check for half carry, ie. result of bit 3 add is a carry into bit 4
        if (i == 3 && bit_carry) set_cc_bit(H_BIT);

        // Record bit 6 carry for overflow check
        if (i == 6) bit_6_carry = bit_carry;
    }

    // Preserve the final carry value in the CC register's c bit
    if (bit_carry) {
        set_cc_bit(C_BIT)
    } else {
        clr_cc_bit(C_BIT);
    }

    // Check for an overflow:
    // v = 1 if c = 1 | c6 = 1 & c != c6
    if ((bit_carry | bit_6_carry) && (bit_carry != bit_6_carry)) {
        set_cc_bit(V_BIT)
    } else {
        clr_cc_bit(V_BIT);
    }

    // Copy answer into bits[] array for conversion to decimal
    uint8_t final = 0;
    for (uint32_t i = 0 ; i < 8 ; i++) {
        if (answer[i] != 0) final = (final | (1 << i));
    }

    // Return the answer (condition codes already set elsewhere)
    return final;
}


/*
 * Addressing Functions
 */
uint16_t address_from_mode(uint8_t mode) {
    // Calculate the intended 16-bit address based on the opcode's
    // addressing mode
    // NOTE All of the called methods update the PC register
    uint16_t address = 0;
    if (mode == MODE_IMMEDIATE) {
        address = reg.pc;
        reg.pc++;
    } else if (mode == MODE_DIRECT) {
        address = address_from_dpr();
        reg.pc++;
    } else if (mode == MODE_INDEXED) {
        address = indexed_address(get_next_byte());
    } else {
        address = address_from_next_two_bytes();
    }

    return address;
}

uint16_t address_from_next_two_bytes() {
    // Reads the bytes at regPC and regPC + 1 and returns them as a 16-bit address
    // NOTE 'loadFromRAM:' auto-increments regPC and handles rollover
    uint8_t msb = get_next_byte();
    uint8_t lsb = get_next_byte();
    return (msb << 8) | lsb;
}

uint16_t address_from_dpr(int16_t offset) {
    // Returns the address composed from regDP (MSB) and the byte at
    // regPC (LSB) plus any supplied offset
    // Does not increment regPC
    // TODO Should we increment regPC?
    uint16_t address = reg.pc;
    address += offest;
    return (reg.dp << 8) | from_ram((uint32_t)address);
}

uint16_t indexed_address(uint8_t post_byte) {
    // This function increases the PC

    // Determine the source register
    uint8_t source_reg = ((post_byte & 0x60) >> 5);

    // Set 'returnValue' to the contents of the source register
    uint16_t address = register_value(source_reg);

    // Process the opcode encoded in the first five bits of the postbyte
    uint8_t op = post_byte & 0x1F;
    if ((post_byte & 0x80) == 0x00) {
        // 5-bit non-indirect offset
        int16_t offset = is_bit_set(op, 4) ? op - 0x20 : op;
        address += offset;
    } else {
        // All other opcodes have bit 7 set to 1
        uint16_t value;
        switch(op) {
            case 0:
                // Auto-increment (,R+)
                address = register_value(source_reg);
                increment_register(source_reg, 1);
                break;
            case 1:
                // Auto-increment (,R++)
                address = register_value(source_reg);
                increment_register(source_reg, 2);
                break;
            case 2:
                // Auto-decrement (,-R)
                increment_register(source_reg, -1);
                address = register_value(source_reg);
                break;
            case 3:
                // Auto-decrement (,--R)
                increment_register(source_reg, -2);
                address = register_value(source_reg);
                break;
            case 4:
                // No offset (,R) - 'address' already set
                break;
            case 5:
                // Accumulator offset B; B is 2s-comp
                address += is_bit_set(reg.b, SIGN_BIT_8) ? reg.b - 256 : reg.b;
                break;
            case 6:
                // Accumulator offset A; A is 2s-comp
                address += is_bit_set(reg.a, SIGN_BIT_8) ? reg.a - 256 : reg.a;
                break;
            case 8:
                // 8-bit constant offset; offset is 2s-comp
                value = get_next_byte();
                address += is_bit_set(value, SIGN_BIT_8) ? value - 256 : value;
                break;
            case 9:
                // 16-bit constant offset; offset is 2s-comp
                uint8_t msb = get_next_byte();
                uint8_t lsb = get_next_byte();
                value = (msb << 8) | lsb;
                address += is_bit_set(value, SIGN_BIT_16) ? value - 65536 : value;
                break;
            case 11:
                // Accumulator offset D; D is a 2s-comp offset
                value = (reg.a << 8) | reg.b;
                address += is_bit_set(value, SIGN_BIT_16) ? value - 65536 : value;
                break;
            case 12:
                // PC relative 8-bit offset; offset is 2s-comp
                uint8_t value = get_next_byte();
                address = reg.pc + (is_bit_set(value, SIGN_BIT_8) ? value - 256 : value);
                break;
            case 13:
                // regPC relative 16-bit offset; offset is 2s-comp
                uint8_t msb = get_next_byte();
                uint8_t lsb = get_next_byte();
                value = (msb << 8) | lsb;
                address = reg.pc + (is_bit_set(value, SIGN_BIT_16) ? value - 65536 : value);
                break;

            // From here on, the addressing is indirect: the Effective Address is a handle not a pointer
            // So we need to add a second memoryMap[] lookup to return the ultimate address we are pointing to

            case 17:
                // Indirect auto-increment + 2
                increment_register(source_reg, 2);
                break;
            case 19:
                // Indirect auto-decrement - 2
                increment_register(source_reg, -2);
                address = register_value(source_reg);
                break;
            case 20:
                // Indirect constant zero offset
                // address = [ram peek:address];
                break;
            case 21:
                // Indirect Accumulator offset B
                // eg. LDA [B,X]
                address += (is_bit_set(reg.b, SIGN_BIT_8) ? reg.b - 256 : reg.b);
                break;
            case 22:
                // Indirect Accumulator offset A
                // eg. LDA [A,Y]
                address += (is_bit_set(reg.a, SIGN_BIT_8) ? reg.a - 256 : reg.a);
                break;
            case 24:
                // Indirect constant 8-bit offset
                // eg. LDA [n,X]
                value = get_next_byte();
                address += (is_bit_set(value, SIGN_BIT_8) ? value - 256 : value);
                break;
            case 25:
                // Indirect constant 16-bit offset
                // eg. LDA [n,X]
                uint8_t msb = get_next_byte();
                uint8_t lsb = get_next_byte();
                value = (msb << 8) | lsb;
                address += (is_bit_set(value, SIGN_BIT_16) ? value - 65536 : value);
                break;
            case 27:
                // Indirect Accumulator offset D
                // eg. LDA [D,X]
                value = (reg.a << 8) | reg.b;
                address += (is_bit_set(value, SIGN_BIT_16) ? value - 65536 : value);
                break;
            case 28:
                // Indirect regPC relative 8-bit offset
                // eg. LDA [n,PCR]
                value = get_next_byte();
                address = reg.pc + (is_bit_set(value, SIGN_BIT_8) ? value - 256 : value);
                break;
            case 29:
                // Indirect regPC relative 16-bit offset
                // eg. LDX [n,PCR]
                uint8_t msb = get_next_byte();
                uint8_t lsb = get_next_byte();
                value = (msb << 8) | lsb;
                address = reg.pc + (is_bit_set(value, SIGN_BIT_16) ? value - 65536 : value);
                break;
            case 31:
                // Extended indirect
                // eg. LDA [n]
                uint8_t msb = get_next_byte();
                uint8_t lsb = get_next_byte();
                address = (msb << 8) | lsb;
                break;
        }

        // For indirect address, use 'address' as a handle
        if (op > 16) {
            // Keep current value of PC
            uint16_t pc = reg.pc;
            reg.pc = address;
            address = address_from_next_two_bytes();

            // Put original PC value back
            reg.pc = pc;
        }
    }

    return address;
}


uint16_t register_value(uint8_t source_reg) {
    // Return the value of the appropriate index register
    // x = 0 ; y = 1 ; u = 2 ; s = 3
    if (source_reg == 0) return reg.x;
    if (source_reg == 1) return reg.y;
    if (source_reg == 2) return reg.u;
    return reg.s;
}


void increment_register(uint8_t source_reg, int16_t amount) {
    // Calculate the offset from the Two's Complement of the 8- or 16-bit value
    uint16_t *reg_ptr = &reg.x;
    if (source_reg == 1) reg_ptr = &reg.y;
    if (source_reg == 2) reg_ptr = &reg.u;
    if (source_reg == 3) reg_ptr = &reg.s;

    uint16_t *new_value = *reg_ptr;
    new_value += amount;
    *reg_ptr = new_value;
}


void reset_registers() {
    reg.a = 0;
    reg.b = 0;
    reg.x = 0;
    reg.y = 0;
    reg.s = 0;
    reg.u = 0;
    reg.pc = 0;
    reg.cc = 0;
    reg.dp = 0;
}