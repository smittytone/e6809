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
    return 0;
}


void loop() {

    while(1) {
        bool is_long = false;
        uint16_t op = NOP;

        // Get the next byte
        uint8_t byte = get_next_byte();
        op = byte;

        // Decode op
        if (byte == 0x10 || byte == 0x11) {
            // Long-form op.
            is_long = true;
            byte = get_next_byte();
            op = (op << 8) | byte;
        }

        // Handle branches
        if ((byte & 0xF0) == BRANCH_MARKER) {
            do_branch(byte, is_long);
        } else {
            do_op(byte, is_long);
        }
    }
}


void do_op(uint8_t op, bool is_long) {
    uint8_t mode = MODE_UNKNOWN;

    uint8_t  msn = (op & 0xF0) >> 4;
    uint8_t  lsn = op & 0x0F;

    if (msn == 0x00) mode = MODE_DIRECT;
    if (msn == 0x06) mode = MODE_INDEXED;
    if (msn == 0x07) mode = MODE_EXTENDED;
    if (msn == 0x04 || msn == 0x05) mode = MODE_INHERENT;

    if lsn == 0x00 && mode != MODE_UNKNOWN {
        // NEG M
    }

    if lsn == 0x03 && mode != MODE_UNKNOWN {
        // COM M
    }

    if lsn == 0x04 && mode != MODE_UNKNOWN {
        // LSR M
    }

    if lsn == 0x06 && mode != MODE_UNKNOWN {
        // ROR M
    }

    if lsn == 0x07 && mode != MODE_UNKNOWN {
        // ASR M
    }

    if lsn == 0x08 && mode != MODE_UNKNOWN {
        // ASL M
    }

    if lsn == 0x09 && mode != MODE_UNKNOWN {
        // ROL M
    }

    if lsn == 0x0A && mode != MODE_UNKNOWN {
        // DEC M
    }

    if lsn == 0x0C && mode != MODE_UNKNOWN {
        // INC M
    }

    if lsn == 0x0D && mode != MODE_UNKNOWN {
        // TST M
    }

    if lsn == 0x0F && mode != MODE_UNKNOWN {
        // CLR M
        if (msn = 0x04) {
            do_clra();
        } else if (msn = 0x05) {
            do_clrb();
        } else {
            do_clrm(mode);
        }
    }

    if (msn < 0x05 && nibble > 1) {
        // Process inherent op.
    } else if (msn == 0x01) {
        // Proces Immediate, not register specific

    } else if (msn > 0x07 && msn < 0x0C) {
        // Targets A (BUT note cmpx)
        mode = get_mode(nibble);
    } else if  (nibble > 0x07 && nibble < 0x0C) {
        // Targets B
        mode = get_mode(nibble);
    }



    switch(op) {
        case NOP:
            break;
        case SYNC:
            break;
        case DAA:
            decimal_adjust_a();
            break;
        case ORCC:
            reg_6809.cc = do_or(reg_6809.cc, get_next_byte());
            break;
        case ANDCC:
            reg_6809.cc = do_and(reg_6809.cc, get_next_byte());
            break;
        case SEX:
            reg_6809.a = (reg_6809.b & 0x80) ? 0xFF : 0x00;
            set_cc(reg_6809.a, (N_BIT | Z_BIT));
            break;
        case EXG:
            reg_transfer(true);
            break;
        case TFR:
            reg_transfer(false);


        case NEGA:
            reg_6809.a = do_neg(reg_6809.a);
            break;
        case COMA:
            reg_6809.a = do_com(reg_6809.a);
            break;
        case NEGB:
            reg_6809.b = do_neg(reg_6809.b);
            break;
        case COMB:
            reg_6809.b = do_com(reg_6809.b);
            break;
    }
}


uint8_t get_next_byte() {
    return (mem_6809[reg_6809.pc++]);
}


uint8_t get_mode(uint8_t n) {
    return (4 - (0x0B - n));
}


uint8_t add_two_8_bit_values(uint8_t a, uint8_t b) {
    uint16_t acc = (uint16_t)a + (uint16_t)b;
    if (acc > 0xFF) set_carry();
    return (uint8_t)(acc & 0x00FF);
}


void set_carry() {
    reg_6809.cc = reg_6809.cc | 0x01;
}

void clr_carry() {
    reg_6809.cc = reg_6809.cc & 0xFE;
}

bool is_cc_bit(uint8_t bit) {
    return ((reg_6809.cc >> bit) & 1);
}

void set_cc_bit(uint8_t bit) {
    reg_6809.cc = (reg_6809.cc | (1 << bit));
}

void clr_cc_bit(uint8_t bit) {
    reg_6809.cc = (reg_6809.cc & ~(1 << bit));
}

void set_cc(uint8_t value, uint8_t field) {
    if ((field >> N_BIT) & 1) {
        if ((value >> 7) & 1) {
            set_cc_bit(N_BIT);
        } else {
            clr_cc_bit(N_BIT);
        }
    }

    if ((field >> Z_BIT) & 1) {
        if (value ==  0) {
            set_cc_bit(Z_BIT);
        } else {
            clr_cc_bit(Z_BIT);
        }
    }
}


void decimal_adjust_a() {
    uint8_t msn = (reg_6809.a & 0xF0) >> 4;
    uint8_t cmsn = 0;
    uint8_t lsn = reg_6809.a & 0x0F;
    uint8_t clsn = 0;
    if ((lsn > 9) || (is_cc_bit(H_BIT))) clsn = 6;
    if ((msn > 9) || (is_cc_bit(C_BIT)) || (msn > 8 && lsn > 9)) cmsn = 6;

    lsn += clsn;
    msn += cmsn;

    reg_6809.a = (msn << 4) | lsn;
    set_cc(reg_6809.a, (N_BIT | Z_BIT));
}


void reg_transfer(bool is_exg) {
    uint8_t post_byte = get_next_byte();
    if ((post_byte & 0x0F) < 0x08) {
        // 16-bit swaps
        uint16_t *source = nibble_to_reg_16((post_byte & 0xF0) >> 4);
        uint16_t *destination = nibble_to_reg_16(post_byte & 0x0F);
        uint16_t v = is_exg ? *destination : 0;
        *destination = *source;
        if (is_exg) *source = v;
    } else {
        // 8-bit swaps
        uint8_t *source = nibble_to_reg_8((post_byte & 0xF0) >> 4);
        uint8_t *destination = nibble_to_reg_8(post_byte & 0x0F);
        uint8_t v = is_exg ? *destination : 0;
        *destination = *source;
        if (is_exg) *source = v;
    }
}

uint8_t *nibble_to_reg_8(uint8_t n) {
    switch(n) {
        case 8:
            return &reg_6809.a;
        case 9:
            return &reg_6809.b;
        case 10:
            return &reg_6809.cc;
        case 11:
            return &reg_6809.dp;
    }
}

uint16_t *nibble_to_reg_16(uint8_t n) {
    switch(n) {
        case 0:
            //return &reg_6809.a;
        case 1:
            return &reg_6809.x;
        case 2:
            return &reg_6809.y;
        case 3:
            return &reg_6809.u;
        case 4:
            return &reg_6809.s;
        case 5:
            return &reg_6809.pc;
    }
}


uint8_t do_or(uint8_t value, uint8_t with) {
    return (value | with);
}

uint8_t do_and(uint8_t value, uint8_t with) {
    return (value & with);
}


uint8_t do_com(uint8_t value) {
    value = ~value;
    reg_6809.cc = (reg_6809.cc && 0xFC) | 0x01;
    set_cc(value, (N_BIT | Z_BIT));
    return value;

}

uint8_t do_neg(uint8_t value) {
    if (value == 0x80) set_cc_bit(V_BIT);
    value = ~value;
    uint16_t carry = (uint16_t)value;
    carry += 1;
    value = (carry & 0x00FF);

    clr_carry();
    if (carry & 0xFF00) set_carry();
    set_cc(value, (N_BIT | Z_BIT));

    return value;
}

void do_clra() {
    reg_6809.a = 0;
    reg_6809.cc = (reg_6809.cc & 0xF0) | 0x04;
}

void do_clrb() {
    reg_6809.b = 0;
    reg_6809.cc = (reg_6809.cc & 0xF0) | 0x04;
}