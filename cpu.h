/*
 * e6809 for Raspberry Pi Pico
 *
 * @version     1.0.0
 * @author      smittytone
 * @copyright   2021
 * @licence     MIT
 *
 */
#ifndef _CPU_HEADER_
#define _CPU_HEADER_


/*
 * CONSTANTS
 */
#define KB64                    65536
#define BRANCH_MARKER           0x20

#define MODE_UNKNOWN            0
#define MODE_IMMEDIATE          1
#define MODE_DIRECT             2
#define MODE_INDEXED            3
#define MODE_EXTENDED           4
#define MODE_INHERENT           5

#define OPCODE_EXTENDED_1       0x10
#define OPCODE_EXTENDED_2       0x11

#define C_BIT                   0
#define V_BIT                   1
#define Z_BIT                   2
#define N_BIT                   3
#define I_BIT                   4
#define H_BIT                   5
#define F_BIT                   6
#define E_BIT                   7

#define SIGN_BIT_8              7
#define SIGN_BIT_16             15

#define MASK_ZC                 0xFA
#define MASK_NZ                 0xF3
#define MASK_NZC                0xF2
#define MASK_NZV                0xF1
#define MASK_NZVC               0xF0

#define PUSH_PULL_CC_REG        0x01
#define PUSH_PULL_ALL_REGS      0xFE
#define PUSH_PULL_EVERY_REG     0xFF

#define PUSH_TO_HARD_STACK      true

#define INTERRUPT_VECTOR_1      0xFFFA
#define INTERRUPT_VECTOR_2      0xFFF4
#define INTERRUPT_VECTOR_3      0xFFF2


/*
 * STRUCTURES
 */
typedef struct {
    uint8_t  a;
    uint8_t  b;
    uint16_t x;
    uint16_t y;
    uint16_t s;
    uint16_t u;
    uint16_t pc;
    uint8_t  cc;
    uint8_t  dp;
} REG_6809;


/*
 * PROTOTYPES
 */
void        loop();
void        process_next_instruction();
void        do_branch(uint8_t bop, bool is_long);

void     do_op(uint8_t op, bool is_long);
uint8_t  get_mode(uint8_t n);

uint8_t  add_two_8_bit_values(uint8_t a, uint8_t b);
void     set_carry();
void     clr_carry();
bool     is_cc_bit(uint8_t bit);
void     set_cc_bit(uint8_t bit);
void     clr_cc_bit(uint8_t bit);
void     set_cc(uint8_t value, uint8_t field);
uint8_t  get_next_byte();

void     decimal_adjust_a();
uint8_t  do_or(uint8_t value, uint8_t with);
uint8_t  do_and(uint8_t value, uint8_t with);
uint8_t  do_com(uint8_t value);
uint8_t  do_neg(uint8_t value);
void     reg_transfer(bool is_exg);
uint8_t  *nibble_to_reg_8(uint8_t n);
uint16_t *nibble_to_reg_16(uint8_t n);


/*
 * GLOBALS
 */
REG_6809    reg_6809;
uint8_t     mem_6809[KB64];
bool        wait_for_interrupt;


#endif // _CPU_HEADER_
