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


REG_6809    reg;
uint8_t     mem[KB64];
bool        wait_for_interrupt;


/*
 * Process Instructions
 */
uint32_t process_next_instruction() {

    uint32_t cycles_used = 0;

    if (wait_for_interrupt) {
        // Process interrupts
        return cycles_used;
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
            if (lsn == ORCC_immed) orr(reg.cc, get_next_byte());
            if (lsn == ANDCC_immed) and(reg.cc, get_next_byte());
            if (lsn == SEX) sex();
            if (lsn == EXG_immed) transfer_decode(get_next_byte(), true);
            if (lsn == TFR_immed) transfer_decode(get_next_byte(), false);
            return cycles_used;
        }

        if (msn == 0x02) {
            // All 0x02 ops are branch ops
            do_branch(opcode, (extended_opcode != 0));
            return cycles_used;
        }

        if (msn == 0x03) {
            // These ops have only one, specific address mode each
            if (lsn  < 0x04) lea(opcode);
            if (lsn == 0x04) push(true,  get_next_byte());
            if (lsn == 0x06) push(false, get_next_byte());
            if (lsn == 0x05) pull(true,  get_next_byte());
            if (lsn == 0x07) pull(false, get_next_byte());
            if (lsn == 0x09) {
                // rts();
                return 99;
            }
            if (lsn == 0x0A) abx();
            if (lsn == 0x0B) rti();
            if (lsn == 0x0C) cwai();
            if (lsn == 0x0D) mul();

            // Cover all values of SWIx
            if (lsn == 0x0F) swi(extended_opcode);
            return cycles_used;
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

            return cycles_used;
        }

        if (lsn == 0x01) {
            cmp(opcode, address_mode);
            return cycles_used;
        }

        if (lsn == 0x02) {
            sbc(opcode, address_mode);
            return cycles_used;
        }

        if (lsn == 0x03) {
            if (msn > 0x0B) {
                add_16(opcode, address_mode);
            } else if (msn > 0x07) {
                sub_16(opcode, address_mode, extended_opcode);
            } else {
                com(opcode, address_mode);
            }

            return cycles_used;
        }

        if (lsn == 0x04) {
            if (msn < 0x08) {
                lsr(opcode, address_mode);
            } else {
                and(opcode, address_mode);
            }

            return cycles_used;
        }

        if (lsn == 0x05) {
            bit(opcode, address_mode);
            return cycles_used;
        }

        if (lsn == 0x06) {
            if (msn < 0x08) {
                ror(opcode, address_mode);
            } else {
                ld(opcode, address_mode);
            }

            return cycles_used;
        }

        if (lsn == 0x07) {
            if (msn < 0x08) {
                asr(opcode, address_mode);
            } else {
                st(opcode, address_mode);
            }

            return cycles_used;
        }

        if (lsn == 0x08) {
            if (msn < 0x08) {
                asl(opcode, address_mode);
            } else {
                eor(opcode, address_mode);
            }

            return cycles_used;
        }

        if (lsn == 0x09) {
            if (msn < 0x08) {
                rol(opcode, address_mode);
            } else {
                adc(opcode, address_mode);
            }

            return cycles_used;
        }

        if (lsn == 0x0A) {
            if (msn < 0x08) {
                dec(opcode, address_mode);
            } else {
                orr(opcode, address_mode);
            }

            return cycles_used;
        }

        if (lsn == 0x0B) {
            add(opcode, address_mode);
            return cycles_used;
        }

        if (lsn == 0x0C) {
            if (msn < 0x08) {
                inc(opcode, address_mode);
            } else if (msn > 0x0C) {
                // LDD
                ld_16(opcode, address_mode, extended_opcode);
            } else {
                cmp_16(opcode, address_mode, extended_opcode);
            }

            return cycles_used;
        }

        if (lsn == 0x0D) {
            if (msn < 0x08) {
                tst(opcode, address_mode);
            } else if (msn > 0x0C) {
                // STD
                st_16(opcode, address_mode, extended_opcode);
            } else {
                jsr(address_mode);
            }

            return cycles_used;
        }

        if (lsn == 0x0E) {
            if (msn < 0x08) {
                jmp(address_mode);
            } else {
                ld_16(opcode, address_mode, extended_opcode);
            }

            return cycles_used;
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
        if (is_bit_set(offset, SIGN_BIT_16)) offset -= 65536;
    } else {
        offset = get_next_byte();
        if (is_bit_set(offset, SIGN_BIT_8)) offset -= 256;
    }

    bool branch = false;

    if (bop == BRA) branch = true;

    if (bop == BEQ && is_cc_bit_set(Z_BIT)) branch = true;
    if (bop == BNE && is_cc_bit_set(Z_BIT)) branch = true;

    if (bop == BMI && is_cc_bit_set(N_BIT)) branch = true;
    if (bop == BPL && is_cc_bit_set(N_BIT)) branch = true;

    if (bop == BVS && is_cc_bit_set( V_BIT)) branch = true;
    if (bop == BVC && is_cc_bit_set(V_BIT)) branch = true;

    if (bop == BLO && is_cc_bit_set(C_BIT)) branch = true; // Also BCS
    if (bop == BHS && is_cc_bit_set(C_BIT)) branch = true; // Also BCC

    if (bop == BGE && (is_cc_bit_set(N_BIT) == is_cc_bit_set(V_BIT))) branch = true;
    if (bop == BGT && !is_cc_bit_set(Z_BIT) && is_cc_bit_set(N_BIT)) branch = true;
    if (bop == BHI && !is_cc_bit_set(C_BIT) && !is_cc_bit_set(Z_BIT)) branch = true;

    if (bop == BLE && (is_cc_bit_set(Z_BIT) || ((is_cc_bit_set( N_BIT) || is_cc_bit_set(V_BIT) && is_cc_bit_set(N_BIT) != is_cc_bit_set(V_BIT))))) branch = true;

    if (bop == BLS && (is_cc_bit_set(C_BIT) || is_cc_bit_set(Z_BIT))) branch = true;
    if (bop == BLT && (is_cc_bit_set(N_BIT) || is_cc_bit_set(V_BIT)) && is_cc_bit_set(V_BIT)) branch = true;


    if (bop == BSR) {
        // Branch to Subroutine: push PC to hardware stack (S) first
        branch = true;
        reg.s--;
        set_byte(reg.s, (reg.pc & 0xFF));
        reg.s--;
        set_byte(reg.s, ((reg.pc >> 8) & 0xFF));
    }

    if (branch) reg.pc += (uint16_t)offset;
}


/*
 * Memory access
 */

uint8_t get_next_byte() {
    // Get the byte at the PC and increment PC
    return (mem[reg.pc++]);
}

uint8_t get_byte(uint16_t address) {
    // Get the byte at the specified address
    return mem[address];
}

void set_byte(uint16_t address, uint8_t value) {
    // Pokes  the 8-bit 'value' into the 16-bit 'address'
    mem[address] = value;
}

void move_pc(int16_t amount) {
    // Move the PC up or down by the specified amount
    reg.pc += amount;
}

bool is_bit_set(uint16_t value, uint8_t bit) {
    // Check a bit in any value
    return ((value & (1 << bit)) != 0);
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

void flp_cc_bit(uint8_t bit) {
    reg.cc = (reg.cc ^ (1 << bit));
}

void clr_cc_nzv() {
    // The N, V and Z bits are frequently cleared, so clear bits 1-3
    reg.cc &= MASK_NZV;
}

void set_cc_nz(uint16_t value, bool is_16_bit) {
    // Set Z and N based on a single 8- or 16-bit input value
    if (value == 0) set_cc_bit(Z_BIT);
    if (is_bit_set(value, (is_16_bit ? SIGN_BIT_16 : SIGN_BIT_8))) set_cc_bit(N_BIT);
}

void set_cc_after_clr() {
    // Affects N, Z, V, C
    //         All cleared, except Z, which is set
    reg.cc &= MASK_NZVC;
    set_cc_bit(Z_BIT);
}

void set_cc_after_load(uint16_t value, bool is_16_bit) {
    // Sets the CC after an 8- or 16-bit load
    // Affects N, Z, V -- V is always cleared
    clr_cc_nzv();
    set_cc_nz(value, is_16_bit);
}

void set_cc_after_store(uint16_t value, bool is_16_bit) {
    // Sets the CC after an 8- or 16-bit store
    // Affects N, Z, V -- V is always cleared
    set_cc_after_load(value, is_16_bit);
}


/*
 * Op Primary Functions
 */

void abx() {
    // ABX: B + X -> X (unsigned)
    // Affects NONE
    reg.x += (uint16_t)reg.b;
}

void adc(uint8_t op, uint8_t mode) {
    // ADC: A + M + C -> A,
    //      B + M + C -> A
    uint16_t address = address_from_mode(mode);
    if (op < ADCB_immed) {
        reg.a = add_with_carry(reg.a, get_byte(address));
    } else {
        reg.b = add_with_carry(reg.b, get_byte(address));
    }
}

void add(uint8_t op, uint8_t mode) {
    // ADD: A + M -> A
    //      B + M -> N
    uint16_t address = address_from_mode(mode);
    if (op < ADDB_immed) {
        reg.a = add_no_carry(reg.a, get_byte(address));
    } else {
        reg.b = add_no_carry(reg.b, get_byte(address));
    }
}

void add_16(uint8_t op, uint8_t mode) {
    // ADDD: D + M:M+1 -> D
    // Affects N, Z, V, C
    //         'alu()' affects H, V, C

    // Should not affect H, so preserve it
    bool h_is_set = is_bit_set(reg.cc, 5);

    // Clear N and Z
    clr_cc_bit(N_BIT);
    clr_cc_bit(Z_BIT);

    // Get bytes
    uint16_t address = address_from_mode(mode);
    uint8_t msb = get_byte(address);
    uint8_t lsb = get_byte(address);

    // Add the two LSBs (M+1, B) to set the carry,
    // then add the two MSBs (M, A) with the carry
    lsb = alu(reg.b, lsb, false);
    msb = alu(reg.a, msb, true);

    // Convert the bytes back to a 16-bit value and set N, Z
    uint16_t answer = (msb << 8) | lsb;
    set_cc_nz(answer, true);

    // Set D's component registers
    reg.a = (answer >> 8) & 0xFF;
    reg.b = answer & 0xFF;

    // Restore H -- may have been changed by 'alu()'
    if (h_is_set) {
        set_cc_bit(H_BIT);
    } else {
        clr_cc_bit(H_BIT);
    }
}

void and(uint8_t op, uint8_t mode) {
    // AND: A & M -> A
    //      B & M -> N
    uint16_t address = address_from_mode(mode);
    if (op < ANDB_immed) {
        reg.a = do_and(reg.a, get_byte(address));
    } else {
        reg.b = do_and(reg.b, get_byte(address));
    }
}

void andcc(uint8_t value) {
    // AND CC: CC & M -> CC
    reg.cc &= value;
}

void asl(uint8_t op, uint8_t mode) {
    // ASL: arithmetic shift left A -> A,
    //      arithmetic shift left B -> B,
    //      arithmetic shift left M -> M
    // This is is the same as LSL
    if (mode == MODE_INHERENT) {
        if (op == ASLA) {
            reg.a = logic_shift_left(reg.a);
        } else {
            reg.b = logic_shift_left(reg.b);
        }
    } else {
        uint16_t address = address_from_mode(mode);
        set_byte(address, logic_shift_left(get_byte(address)));
    }
}

void asr(uint8_t op, uint8_t mode) {
    // ASR: arithmetic shift right A -> A,
    //      arithmetic shift right B -> B,
    //      arithmetic shift right M -> M
    if (mode == MODE_INHERENT) {
        if (op == ASRA) {
            reg.a = arith_shift_right(reg.a);
        } else {
            reg.b = arith_shift_right(reg.b);
        }
    } else {
        uint16_t address = address_from_mode(mode);
        set_byte(address, arith_shift_right(get_byte(address)));
    }
}

void bit(uint8_t op, uint8_t mode) {
    // BIT: Bit test A (A & M)
    //      Bit test B (B & M)
    // Does not affect the operands, only the CC register
    uint16_t address = address_from_mode(mode);
    uint8_t ignored = do_and((op < BITB_immed ? reg.a : reg.b), get_byte(address));
}

void clr(uint8_t op, uint8_t mode) {
    // CLR: 0 -> A
    //      0 -> B
    //      0 -> M
    if (mode == MODE_INHERENT) {
        if (op == CLRA) {
            reg.a = 0;
        } else {
            reg.b = 0;
        }
    } else {
        uint16_t address = address_from_mode(mode);
        set_byte(address, 0);
    }

    set_cc_after_clr();
}

void cmp(uint8_t op, uint8_t mode) {
    // CMP: Compare M to A
    //              M to B
    uint16_t address = address_from_mode(mode);
    compare((op < CMPB_immed ? reg.a : reg.b), get_byte(address));
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

    // Set a pointer to the target register, D by default
    uint16_t d = (reg.a << 8) + reg.b;
    uint16_t *reg_ptr = &d;
    if (op == 0x83 && ex_op == OPCODE_EXTENDED_2) reg_ptr = &reg.u;
    if (op == CMPX_immed) reg_ptr = ex_op == 0 ? &reg.x : (ex_op == OPCODE_EXTENDED_1 ? &reg.y : &reg.s);

    // Get the data and subtract from the target register
    uint16_t comp_value = (get_byte(address) << 8) | (get_byte(address + 1));

    // Subtract 'compValue' from the target register - this will set the CC register
    // NOTE ignore the return value - we're just setting the CC bits
    subtract_16(*reg_ptr, comp_value);
}

void com(uint8_t op, uint8_t mode) {
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
        set_byte(address, complement(get_byte(address)));
    }
}

void cwai() {
    // CWAI
    // Clear CC bits and Wait for Interrupt
    // AND CC with the operand, set e, then push every register,
    // including CC to the hardware stack
    reg.cc = reg.cc & mem[reg.pc];
    set_cc_bit(E_BIT);
    push(PUSH_TO_HARD_STACK, PUSH_PULL_EVERY_REG);
    wait_for_interrupt = true;
}

void daa() {
    // DAA: Decimal Adjust after Addition
    bool carry = is_bit_set(reg.cc, C_BIT);
    clr_cc_bit(C_BIT);

    // Set Least Significant Conversion Factor, which will be either 0 or 6
    uint8_t correction = 0;
    uint8_t lsn = reg.a & 0x0F;
    if (is_bit_set(reg.cc, H_BIT) || lsn > 9) correction = 6;
    reg.a += correction;

    // Set Most Significant Conversion Factor, which will be either 0 or 6
    uint8_t msn = (reg.a & 0xF0) >> 4;
    correction = 0;
    if (carry || msn > 8 || lsn > 9) correction = 6;
    msn += correction;
    if (msn > 0x0F) set_cc_bit(C_BIT);

    reg.a = (msn << 4) | (reg.a & 0x0F);
    set_cc_nz(reg.a, false);
}

void dec(uint8_t op, uint8_t mode) {
    // DEC: A - 1 -> A,
    //      B - 1 -> B,
    //      M - 1 -> M
    if (mode == MODE_INHERENT) {
        if (op == DECA) {
            reg.a = decrement(reg.a);
        } else {
            reg.b = decrement(reg.b);
        }
    } else {
        uint16_t address = address_from_mode(mode);
        set_byte(address, decrement(get_byte(address)));
    }
}

void eor(uint8_t op, uint8_t mode) {
    // EOR: A ^ M -> A
    //      B ^ M -> B
    uint16_t address = address_from_mode(mode);
    if (op < EORB_immed) {
        reg.a = do_xor(reg.a, get_byte(address));
    } else {
        reg.b = do_xor(reg.b, get_byte(address));
    }
}

void inc(uint8_t op, uint8_t mode) {
    // INC: A + 1 -> A,
    //      B + 1 -> B,
    //      M + 1 -> M
    if (mode == MODE_INHERENT) {
        if (op == INCA) {
            reg.a = increment(reg.a);
        } else {
            reg.b = increment(reg.b);
        }
    } else {
        uint16_t address = address_from_mode(mode);
        set_byte(address, increment(get_byte(address)));
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
    set_byte(reg.s, (reg.pc & 0xFF));
    reg.s--;
    set_byte(reg.s, ((reg.pc >> 8) & 0xFF));
    reg.pc = address;
}

void ld(uint8_t op, uint8_t mode) {
    // LD: M -> A,
    //     M -> B
    uint16_t address = address_from_mode(mode);
    if (op < LDB_immed) {
        reg.a = get_byte(address);
        set_cc_after_load(reg.a, false);
    } else {
        reg.b = get_byte(address);
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
    if (op == LDU_immed) reg_ptr = ex_op == OPCODE_EXTENDED_1 ? &reg.s : &reg.u;
    if (op == LDX_immed) reg_ptr = ex_op == OPCODE_EXTENDED_1 ? &reg.y : &reg.x;

    // Write the data into the target register
    *reg_ptr = (get_byte(address) << 8) | get_byte(address + 1);

    // Set the A and B registers if we're targeting D
    if (op == LDD_immed) {
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
        if (op == LSRA) {
            reg.a = logic_shift_right(reg.a);
        } else {
            reg.b = logic_shift_right(reg.b);
        }
    } else {
        uint16_t address = address_from_mode(mode);
        set_byte(address, logic_shift_right(get_byte(address)));
    }
}

void mul() {
    // MUL: A x B -> D (unsigned)
    // Affects Z, C
    reg.cc = reg.cc & MASK_ZC;

    uint16_t d = reg.a * reg.b;
    if (is_bit_set(d, 7)) set_cc_bit(C_BIT);
    if (d == 0) set_cc_bit(Z_BIT);

    // Decompose D
    reg.a = (d >> 8) & 0xFF;
    reg.b = d & 0xFF;
}

void neg(uint8_t op, uint8_t mode) {
    // NEG: !R + 1 -> R, !M + 1 -> M
    if (mode == MODE_INHERENT) {
        if (op == NEGA) {
            reg.a = negate(reg.a);
        } else {
            reg.b = negate(reg.b);
        }
    } else {
        uint16_t address = address_from_mode(mode);
        set_byte(address, negate(get_byte(address)));
    }
}

void orr(uint8_t op, uint8_t mode) {
    // OR: A | M -> A
    //     B | M -> B
    uint16_t address = address_from_mode(mode);
    if (op < ORB_immed) {
        reg.a = do_or(reg.a, get_byte(address));
    } else {
        reg.b = do_or(reg.b, get_byte(address));
    }
}

void orcc(uint8_t value) {
    // OR CC: CC | M -> CC
    reg.cc = reg.cc | value;
}

void rol(uint8_t op, uint8_t mode) {
    // ROL: rotate left A -> A,
    //      rotate left B -> B,
    //      rotate left M -> M
    if (mode == MODE_INHERENT) {
        if (op == ROLA) {
            reg.a = rotate_left(reg.a);
        } else {
            reg.b = rotate_left(reg.b);
        }
    } else {
        uint16_t address = address_from_mode(mode);
        set_byte(address, rotate_left(get_byte(address)));
    }
}

void ror(uint8_t op, uint8_t mode) {
    // ROR: rotate right A -> A,
    //      rotate right B -> B,
    //      rotate right M -> M
    if (mode == MODE_INHERENT) {
        if (op == RORA) {
            reg.a = rotate_right(reg.a);
        } else {
            reg.b = rotate_right(reg.b);
        }
    } else {
        uint16_t address = address_from_mode(mode);
        set_byte(address, rotate_right(get_byte(address)));
    }
}

void rti() {
    // RTI
    // Pull CC from the hardware stack; if E is set, pull all the registers
    // from the hardware stack, otherwise pull the PC register only
    pull(true, PUSH_PULL_CC_REG);
    if (is_cc_bit_set(E_BIT)) {
        pull(true, PUSH_PULL_ALL_REGS);
    } else {
        pull(true, PUSH_PULL_PC_REG);
    }
}

void rts() {
    // RTS
    // Pull the PC from the hardware stack
    pull(true, PUSH_PULL_PC_REG);

}

void sbc(uint8_t op, uint8_t mode) {
    // SBC: A - M - C -> A,
    //      B - M - C -> A
    uint16_t address = address_from_mode(mode);
    if (op < SBCB_immed) {
        reg.a = sub_with_carry(reg.a, get_byte(address));
    } else {
        reg.b = sub_with_carry(reg.b, get_byte(address));
    }
}

void sex() {
    // SEX: sign-extend B into A
    // Affects N, Z
    reg.cc = reg.cc & MASK_NZ;
    reg.a = 0;
    if (is_bit_set(reg.b, SIGN_BIT_8)) {
        reg.a = 0xFF;
        set_cc_bit(N_BIT);
    }

    if (reg.b == 0) set_cc_bit(Z_BIT);
}

void st(uint8_t op, uint8_t mode) {
    // ST: A -> M,
    //     B -> M
    // Affects N, Z, V
    //         V is always cleared
    uint16_t address = address_from_mode(mode);
    if (op < STB_direct) {
        set_byte(address, reg.a);
        set_cc_after_store(reg.a, false);
    } else {
        set_byte(address, reg.b);
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
    uint16_t *reg_ptr = (uint16_t *)reg.a;
    if (op == STU_direct) reg_ptr = op == OPCODE_EXTENDED_1 ? &reg.s : &reg.u;
    if (op == STX_direct) reg_ptr = op == OPCODE_EXTENDED_1 ? &reg.y : &reg.x;

    // Write the target register out
    set_byte(address, ((*reg_ptr >> 8) & 0xFF));
    set_byte(address + 1, (*reg_ptr & 0xFF));

    // Update the CC register
    set_cc_after_store(*reg_ptr, true);
}

void sub(uint8_t op, uint8_t mode) {
    // SUB: A - M -> A,
    //      B - M -> B
    uint16_t address = address_from_mode(mode);
    if (op < SUBB_immed) {
        reg.a = subtract(reg.a, get_byte(address));
    } else {
        reg.b = subtract(reg.b, get_byte(address));
    }
}

void sub_16(uint8_t op, uint8_t mode, uint8_t ex_op) {
    // SUBD: D - M:M + 1 -> D
    // Affects N, Z, V, C
    //         V, C (H) set by 'alu()'
    reg.cc = reg.cc & MASK_NZV;

    // Complement the value at M:M + 1
    // NOTE Don't use 'negate()' because we need to
    //      include the carry into the MSB
    uint16_t address = address_from_mode(mode);
    uint8_t msb = complement(get_byte(address));
    uint8_t lsb = complement(get_byte(address + 1));

    // Add 1 to form the 2's complement
    lsb = alu(lsb, 1, false);
    msb = alu(msb, 0, true);

    // Add D - A and B
    lsb = alu(reg.b, lsb, false);
    msb = alu(reg.a, msb, true);

    // Convert the bytes back to a 16-bit value and set the CC
    uint16_t answer = (msb << 8) | lsb;
    set_cc_nz(answer, true);

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
        // Set I and F
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
        if (op == TSTA) {
            test(reg.a);
        } else {
            test(reg.b);
        }
    } else {
        uint16_t address = address_from_mode(mode);
        test(get_byte(address));
    }
}


/*
 * Op helper functions
 */
uint8_t alu(uint8_t value_1, uint8_t value_2, bool use_carry) {
    // Simulates addition of two unsigned 8-bit values in a binary ALU
    // Checks for half-carry, carry and overflow
    // Affects H, C, V
    uint8_t binary_1[8], binary_2[8], answer[8];
    bool bit_carry = false;
    bool bit_6_carry = false;

    for (uint32_t i = 0 ; i < 8 ; i++) {
        binary_1[i] = ((value_1 >> i) & 0x01);
        binary_2[i] = ((value_2 >> i) & 0x01);
    }

    if (use_carry) bit_carry = is_cc_bit_set(C_BIT);

    clr_cc_bit(C_BIT);
    clr_cc_bit(V_BIT);

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

    // Preserve the final carry value in the CC register's C bit
    if (bit_carry) set_cc_bit(C_BIT);

    /* Check for an overflow:
       V = 1 if C XOR c6 == 1
       "A processor [sets] the overflow flag when the carry out of the
        most significant bit is different from the carry out of the next most
        significant bit; that is, an overflow is the exclusive-OR of the carries
        into and out of the sign bit." */
    if (bit_carry != bit_6_carry) set_cc_bit(V_BIT);

    // Copy answer into bits[] array for conversion to decimal
    uint8_t final = 0;
    for (uint32_t i = 0 ; i < 8 ; i++) {
        if (answer[i] != 0) final = (final | (1 << i));
    }

    // Return the answer (condition codes already set elsewhere)
    return final;
}

uint16_t alu_16(uint16_t value_1, uint16_t value_2, bool use_carry) {
    // Add the LSBs
    uint8_t v_1 = value_1 & 0xFF;
    uint8_t v_2 = value_2 & 0xFF;
    uint8_t total = alu(v_1, v_2, use_carry);

    // Now add the MSBs, using the carry (if any) from the LSB calculation
    v_1 = (value_1 >> 8) & 0xFF;
    v_2 = (value_2 >> 8) & 0xFF;
    uint8_t subtotal = alu(v_1, v_2, true);
    return ((subtotal << 8) + total);
}

uint8_t add_no_carry(uint8_t value, uint8_t amount) {
    // Adds two 8-bit values
	// Affects H, N, Z, V, C
    //         'alu:' sets H, V, C
    reg.cc = reg.cc & MASK_NZ;
    uint8_t answer = alu(value, amount, false);
    set_cc_nz(answer, false);
    return answer;
}

uint8_t add_with_carry(uint8_t value, uint8_t amount) {
    // Adds two 8-bit values plus CC C
    // Affects H, N, Z, V, C
    //         'alu()' sets H, V, C
    reg.cc = reg.cc & MASK_NZ;
    uint8_t answer = alu(value, amount, true);
    set_cc_nz(answer, false);
    return answer;
}

uint8_t subtract(uint8_t value, uint8_t amount) {
    // Subtract 'amount' from 'value' by adding 'value' to -'amount'
    // Affects N, Z, V, C
    //         V, C (H) set by 'alu()'
    reg.cc = reg.cc & MASK_NZVC;
    uint8_t comp = negate(amount);
    uint8_t answer = alu(value, amount, false);
    set_cc_nz(answer, false);

    // C represents a borrow and is set to the complement of the carry
    // of the internal binary addition
    flp_cc_bit(C_BIT);
    return answer;
}

uint16_t subtract_16(uint16_t value, uint16_t amount) {
    // Subtract amount from value
    // Affects N, Z, V, C
    //         N, Z, V, C affected by 'complement()'
    //         V, C are set by 'alu()'

    // Complement the value at M:M + 1
    uint8_t msb = complement((amount >> 8) & 0xFF);
    uint8_t lsb = complement(amount & 0xFF);

    reg.cc = reg.cc & MASK_NZ;

    // Add 1 to form the 2's complement
    lsb = alu(lsb, 1, false);
    msb = alu(msb, 0, true);

    // Add the register value
    lsb = alu(value & 0xFF, lsb, false);
    msb = alu((value >> 8) & 0xFF, msb, true);

    // Convert the bytes back to a 16-bit value and set the CC
    uint16_t answer = (msb << 8) | lsb;
    set_cc_nz(answer, false);

    // c represents a borrow and is set to the complement of the carry
    // of the internal binary addition
    flp_cc_bit(C_BIT);

    return answer;
}

uint8_t sub_with_carry(uint8_t value, uint8_t amount) {
    // Subtract with Carry (borrow)
    // Affects N, Z, V, C
    //         V, C (H) set by 'alu()'
    reg.cc = reg.cc & MASK_NZV;
    uint8_t comp = negate(amount);
    uint8_t answer = alu(value, comp, true);
    set_cc_nz(answer, false);

    // C represents a borrow and is set to the complement of the carry
    // of the internal binary addition
    flp_cc_bit(C_BIT);
    return answer;
}

uint8_t do_and(uint8_t value, uint8_t amount) {
    // ANDs the two supplied values
    // Affects N, Z, V
    //         V is always cleared
    clr_cc_nzv();
    uint8_t answer = value & amount;
    set_cc_nz(answer, false);
    return answer;
}

uint8_t do_or(uint8_t value, uint8_t amount) {
    // OR
    // Affects N, Z, V
    //         V is always cleared
    clr_cc_nzv();
    uint8_t answer = value | amount;
    set_cc_nz(answer, false);
    return answer;
}

uint8_t do_xor(uint8_t value, uint8_t amount) {
    // Perform an exclusive OR
    // Affects N, Z, V
    //         V always cleared
    clr_cc_nzv();
    uint8_t answer = value ^ amount;
    set_cc_nz(answer, false);
    return answer;
}

uint8_t arith_shift_right(uint8_t value) {
    // Arithmetic shift right
    // Affects N, Z, C
    partial_shift_right(value);
    set_cc_nz(value, false);
    return value;
}

uint8_t logic_shift_left(uint8_t value) {
    // Logical shift left
    // Affects N, Z, V, C
    reg.cc = reg.cc & MASK_NZVC;
    if (is_bit_set(value, 7)) set_cc_bit(C_BIT);
    if (is_bit_set(value, 7) != is_bit_set(value, 6)) set_cc_bit(V_BIT);

    for (uint32_t i = 7 ; i > 0  ; i--) {
        if (is_bit_set(value, i - 1)) {
            value = value | (1 << i);
        } else {
            value = value & ~(1 << i);
        }
    }

    // Clear bit 0
    value = value & 0xFE;
    set_cc_nz(value, false);
    return value;
}

uint8_t logic_shift_right(uint8_t value) {
    // Logical shift right
    // Affects N, Z, C
    //         N is always cleared
    partial_shift_right(value);

    // Clear bit 7
    value = value & 0x7F;
    if (value == 0) set_cc_bit(Z_BIT);
    return value;
}

uint8_t partial_shift_right(uint8_t value) {
    // Code common to LSR and ASR
    reg.cc = reg.cc & MASK_NZC;
    if (is_bit_set(value, 0)) set_cc_bit(C_BIT);

    for (uint32_t i = 0 ; i < 7 ; i++) {
        if (is_bit_set(value, i + 1)) {
            value = value | (1 << i);
        } else {
            value = value & ~(1 << i);
        }
    }

    return value;
}

uint8_t rotate_left(uint8_t value) {
    // Rotate left
    // Affects N, Z, V, C

    // C becomes bit 7 of original operand
    bool carry = is_cc_bit_set(C_BIT);
    reg.cc = reg.cc & MASK_NZVC;
    if (is_bit_set(value, 7)) set_cc_bit(C_BIT);

    // N is bit 7 XOR bit 6 of value
    if (is_bit_set(value, 7) != is_bit_set(value, 6)) set_cc_bit(V_BIT);

    for (uint32_t i = 7 ; i > 0  ; i--) {
        if (is_bit_set(value, i - 1)) {
            value = value | (1 << i);
        } else {
            value = value & ~(1 << i);
        }
    }

    // Set bit 0 from the carry
    if (carry) {
        value = value | 1;
    } else {
        value = value & 0xFE;
    }

    set_cc_nz(value, false);
    return value;
}

uint8_t rotate_right(uint8_t value) {
    // Rotate right
    // Affects N, Z, C

    // C becomes bit 0 of original operand
    bool carry = is_cc_bit_set(C_BIT);
    reg.cc = reg.cc & MASK_NZC;
    if (is_bit_set(value, 0)) set_cc_bit(C_BIT);

    for (uint32_t i = 0 ; i < 7 ; i++) {
        if (is_bit_set(value, i + 1)) {
            value = value | (1 << i);
        } else {
            value = value & ~(1 << i);
        }
    }

    // Set bit 7 from the carry
    if (carry) {
        value = value | 0x80;
    } else {
        value = value & 0x7F;
    }

    set_cc_nz(value, false);
    return value;
}

void compare(uint8_t value, uint8_t amount) {
    // Compare two values by subtracting the second from the first.
    // Result is discarded
    // Affects N, Z, V, C
    //         C, V (H) set by 'alu()' via 'subtract()'
    reg.cc = reg.cc & MASK_NZVC;
    uint8_t answer = subtract(value, amount);
    set_cc_nz(value, false);
}

uint8_t negate(uint8_t value) {
    // Returns 2's complement of 8-bit value
    // Affects Z, N, V, C
    //         C, V (H) set by 'alu()'
    //         V set only if value is 0x80 (see Zaks p 167)
    clr_cc_nzv();
    if (value == 0x80) set_cc_bit(V_BIT);

    // Flip value's bits to make the 1s complenent
    for (uint32_t i = 0 ; i < 8 ; i++) {
        value = value ^ (1 << i);
    }

    // Add 1 to the bits to get the 2s complement
    uint8_t answer = alu(value, 1, false);
    set_cc_nz(answer, false);

    // c represents a borrow and is set to the complement of
    // the carry of the internal binary addition
    flp_cc_bit(C_BIT);
    return answer;
}

uint8_t complement(uint8_t value) {
    // One's complement the passed value
    // Affects N, Z, V, C
    //         V, C take fixed values: 0, 1
    clr_cc_nzv();
    set_cc_bit(C_BIT);

    for (uint32_t i = 0 ; i < 8 ; i++) {
        value = value ^ (1 << i);
    }

    set_cc_nz(value, false);
    return value;
}

uint8_t decrement(uint8_t value) {
    // Subtract 1 from the operand
    // Affects: N, Z, V
    //          V set only if value is $80
    clr_cc_nzv();

    // See 'Programming the 6809' p 155
    if (value == 0x80) set_cc_bit(V_BIT);

    uint8_t answer = value - 1;
    set_cc_nz(answer, false);
    return answer;
}

uint8_t increment(uint8_t value) {
    // Add 1 to the operand
    // Affects N, Z, V
    clr_cc_nzv();

    // See 'Programming the 6809' p 158
    if (value == 0x7F) set_cc_bit(V_BIT);

    uint8_t answer = value + 1;
    set_cc_nz(answer, false);
    return answer;
}


uint8_t *set_reg_ptr(uint8_t reg_code) {
    switch(reg_code) {
        case 0x09:
            return &reg.b;
        case 0x0A:
            return &reg.cc;
        case 0x0B:
            return &reg.dp;
    }

    return &reg.a;
}

uint16_t *set_reg_16_ptr(uint8_t reg_code) {
    switch(reg_code) {
        case 0x01:
            return &reg.x;
        case 0x02:
            return &reg.y;
        case 0x03:
            return &reg.u;
        case 0x04:
            return &reg.s;
        case 0x05:
            return &reg.pc;
    }

    // Proxy for D
    return (uint16_t *)&reg.a;
}


void transfer_decode2(uint8_t reg_code, bool is_swap) {

    uint8_t source_reg = (reg_code & 0xF0) >> 4;
    uint8_t dest_reg = reg_code & 0x0F;

    uint8_t *src_ptr;
    uint8_t *dst_ptr;
    uint16_t *src_16_ptr;
    uint16_t *dst_16_ptr;
    uint16_t val_16;
    uint8_t val;

    if (source_reg < 0x08 && dest_reg > 0x05) return;
    if (source_reg > 0x05 && dest_reg < 0x08) return;

    if (source_reg > 0x05) {
        // 8-bit transfers
        src_ptr = set_reg_ptr(source_reg);
        dst_ptr = set_reg_ptr(dest_reg);
        val = *dst_ptr;
        *dst_ptr = *src_ptr;
        if (is_swap) *src_ptr = val;
    } else {
        // 16-bit transfers
        src_16_ptr = set_reg_16_ptr(source_reg);
        dst_16_ptr = set_reg_16_ptr(dest_reg);
        val_16 = *dst_16_ptr;
        *dst_16_ptr = *src_16_ptr;
        if (is_swap) *src_16_ptr = val_16;
    }
}


void transfer_decode(uint8_t reg_code, bool is_swap) {
    // 'reg_code' contains an 8-bit number that identifies the two registers
    // to be swapped. Top four bits give source, bottom four bits give
    // the destination.

    uint8_t source_reg = (reg_code & 0xF0) >> 4;
    uint8_t dest_reg = reg_code & 0x0F;
    uint16_t d = 0;

    switch(source_reg) {
        case 0x00:
            // D - can only swap to X, Y, U, S, PC
            if (dest_reg > 0x05) return;
            d = exchange_16(((reg.a << 8) | reg.b), dest_reg);
            if (is_swap) {
                reg.a = (d >> 8) & 0xFF;
                reg.b = d & 0xFF;
            }
            break;
        case 0x01:
            // X - can only swap to D, Y, U, S, PC
            if (dest_reg > 0x05) return;
            if (is_swap) {
                reg.x = exchange_16(reg.x, dest_reg);
            } else {
                exchange_16(reg.x, dest_reg);
            }
            break;
        case 0x02:
            // Y - can only swap to D, X, U, S, PC
            if (dest_reg > 0x05) return;
            if (is_swap) {
                reg.y = exchange_16(reg.y, dest_reg);
            } else {
                exchange_16(reg.y, dest_reg);
            }
            break;
        case 0x03:
            // U - can only swap to D, X, Y, S, PC
            if (dest_reg > 0x05) return;
            if (is_swap) {
                reg.u = exchange_16(reg.u, dest_reg);
            } else {
                exchange_16(reg.u, dest_reg);
            }
            break;
        case 0x04:
            // S - can only swap to D, X, Y, U, PC
            if (dest_reg > 0x05) return;
            if (is_swap) {
                reg.s = exchange_16(reg.s, dest_reg);
            } else {
                exchange_16(reg.s, dest_reg);
            }
            break;
        case 0x05:
            // PC - can only swap to D, X, Y, U, S
            if (dest_reg > 0x05) return;
            if (is_swap) {
                reg.pc = exchange_16(reg.pc, dest_reg);
            } else {
                exchange_16(reg.pc, dest_reg);
            }
            break;
        case 0x08:
            // A - can only swap to B, CC, DP
            if (dest_reg < 0x08) return;
            if (is_swap) {
                reg.a = exchange(reg.a, dest_reg);
            } else {
                exchange(reg.a, dest_reg);
            }
            break;
        case 0x09:
            // B - can only swap to A, CC, DP
            if (dest_reg < 0x08) return;
            if (is_swap) {
                reg.b = exchange(reg.b, dest_reg);
            } else {
                exchange(reg.b, dest_reg);
            }
            break;
        case 0x0A:
            // CC - can only swap to A, B, DP
            if (dest_reg < 0x08) return;
            if (is_swap) {
                reg.cc = exchange(reg.cc, dest_reg);
            } else {
                exchange(reg.cc, dest_reg);
            }
            break;
        case 0x0B:
            // DPR - can only swap to A, B, CC
            if (dest_reg < 0x08) return;
            if (is_swap) {
                reg.dp = exchange(reg.dp, dest_reg);
            } else {
                exchange(reg.dp, dest_reg);
            }
            break;
    }
}


uint8_t exchange(uint8_t value, uint8_t reg_code) {
    // Returns the value *from* the register identified by 'reg_code'
    // after placing the passed value into that register
    uint8_t return_value = 0x00;
    switch(reg_code) {  // This is the DESTINATION register
        case 0x08: // A
            return_value = reg.a;
            reg.a = value;
            break;
        case 0x09: // B
            return_value = reg.b;
            reg.b = value;
            break;
        case 0x0A: // CC
            return_value = reg.cc;
            reg.cc = value;
            break;
        case 0x0B: // DPR
            return_value = reg.dp;
            reg.dp = value;
            break;
        default:
            // Incorrect Register - signal error
            // [self halt:@"Incorrect register in 8-bit swap"];
            return_value = 0xFF;
    }

    return return_value;
}


uint16_t exchange_16(uint16_t value, uint8_t reg_code) {
    // Returns the value *from* the register identified by 'reg_code'
    // after placing the passed value into that register
    uint16_t return_value = 0x0000;
    switch(reg_code) {  // This is the DESTINATION register
        case 0x00: // D
            return_value = (reg.a << 8) | reg.b;
            reg.a = (value >> 8) & 0xFF;
            reg.b = value & 0xFF;
            break;
        case 0x01: // X
            return_value = reg.x;
            reg.x = value;
            break;
        case 0x02: // Y
            return_value = reg.y;
            reg.y = value;
            break;
        case 0x03: // U
            return_value = reg.u;
            reg.u = value;
            break;
        case 0x04: // S
            return_value = reg.s;
            reg.s = value;
            break;
        case 0x05: // PC
            return_value = reg.pc;
            reg.pc = value;
            break;
        default:
            // Incorrect Register - signal error
            // [self halt:@"Incorrect register in 8-bit swap"];
            return_value = 0xFFFF;
    }

    return return_value;
}


void load_effective(uint16_t amount, uint8_t reg_code) {
    // Put passed value into an 8-bit register
    // Affects Z -- but only for X and Y registers
    switch(reg_code) {
        case 0x00:
            reg.x = amount;
            if (amount == 0) set_cc_bit(Z_BIT);
            break;
        case 0x01:
            reg.y = amount;
            if (amount == 0) set_cc_bit(Z_BIT);
            break;
        case 0x02:
            reg.s = amount;
            break;
        case 0x03:
            reg.u = amount;
    }
}


void push(bool to_hardware, uint8_t post_byte) {
    // Push the specified registers (in 'post_byte') to the hardware or user stack
    // ('to_hardware' should be true for S, false for U)
    // See 'Programming the 6809' p.171-2
    uint16_t source = to_hardware ? reg.u : reg.s;
    uint16_t dest   = to_hardware ? reg.s : reg.u;

    if (is_bit_set(post_byte, 7)) {
        // Push PC
        dest--;
        set_byte(dest, (reg.pc & 0xFF));
        dest--;
        set_byte(dest, ((reg.pc >> 8) & 0xFF));
    }

    if (is_bit_set(post_byte, 6)) {
        // Push U/S
        dest--;
        set_byte(dest, (source & 0xFF));
        dest--;
        set_byte(dest, ((source >> 8) & 0xFF));
    }

    if (is_bit_set(post_byte, 5)) {
        // Push Y
        dest--;
        set_byte(dest, (reg.y & 0xFF));
        dest--;
        set_byte(dest, ((reg.y >> 8) & 0xFF));
    }

    if (is_bit_set(post_byte, 4)) {
        // Push X
        dest--;
        set_byte(dest, (reg.x & 0xFF));
        dest--;
        set_byte(dest, ((reg.x >> 8) & 0xFF));
    }

    if (is_bit_set(post_byte, 3)) {
        // Push DP
        dest--;
        set_byte(dest, reg.dp);
    }

    if (is_bit_set(post_byte, 2)) {
        // Push B
        dest--;
        set_byte(dest, reg.b);
    }

    if (is_bit_set(post_byte, 1)) {
        // Push A
        dest--;
        set_byte(dest, reg.a);
    }

    if (is_bit_set(post_byte, 0)) {
        // Push CC
        dest--;
        set_byte(dest, reg.cc);
    }

    if (to_hardware) {
        reg.s = dest;
    } else {
        reg.u = dest;
    }
}


void pull(bool from_hardware, uint8_t post_byte) {
    // Pull the specified registers (in 'post_byte') from the hardware or user stack
    // ('from_hardware' should be true for S, false for U)
    // See 'Programming the 6809' p.173-4
    uint16_t source = from_hardware ? reg.s : reg.u;
    uint16_t dest   = from_hardware ? reg.u : reg.s;

    if (is_bit_set(post_byte, 0)) {
        // Pull CC
        reg.cc = get_byte(source);
        source++;
    }

    if (is_bit_set(post_byte, 1)) {
        // Pull A
        reg.a = get_byte(source);
        source++;
    }

    if (is_bit_set(post_byte, 2)) {
        // Pull B
        reg.b = get_byte(source);
        source++;
    }

    if (is_bit_set(post_byte, 3)) {
        // Pull DP
        reg.dp = get_byte(source);
        source++;
    }

    if (is_bit_set(post_byte, 4)) {
        // Pull X
        reg.x = (get_byte(source) << 8);
        source++;
        reg.x |= get_byte(source);
        source++;
    }

    if (is_bit_set(post_byte, 5)) {
        // Pull Y
        reg.y = (get_byte(source) << 8);
        source++;
        reg.y |= get_byte(source);
        source++;
    }

    if (is_bit_set(post_byte, 6)) {
        // Pull S or U
        dest = (get_byte(source) << 8);
        source++;
        dest |= get_byte(source);
        source++;
    }

    if (is_bit_set(post_byte, 7)) {
        // Pull PC
        reg.pc = (get_byte(source) << 8);
        source++;
        reg.pc |= get_byte(source);
        source++;
    }

    if (from_hardware) {
        reg.s = source;
        reg.u = dest;
    } else {
        reg.u = source;
        reg.s = dest;
    }
}

void test(uint8_t value) {
    // Tests value for zero or negative
	// Affects N, Z, V
    //         V is always cleared
    clr_cc_nzv();
    set_cc_nz(value, false);
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
        address = address_from_dpr(0);
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
    address += offset;
    return (reg.dp << 8) | get_byte((uint32_t)address);
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
        uint8_t msb, lsb, value;
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
                msb = get_next_byte();
                lsb = get_next_byte();
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
                value = get_next_byte();
                address = reg.pc + (is_bit_set(value, SIGN_BIT_8) ? value - 256 : value);
                break;
            case 13:
                // regPC relative 16-bit offset; offset is 2s-comp
                msb = get_next_byte();
                lsb = get_next_byte();
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
                msb = get_next_byte();
                lsb = get_next_byte();
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
                msb = get_next_byte();
                lsb = get_next_byte();
                value = (msb << 8) | lsb;
                address = reg.pc + (is_bit_set(value, SIGN_BIT_16) ? value - 65536 : value);
                break;
            case 31:
                // Extended indirect
                // eg. LDA [n]
                msb = get_next_byte();
                lsb = get_next_byte();
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

    uint16_t new_value = *reg_ptr;
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
