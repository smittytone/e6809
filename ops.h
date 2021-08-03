/*
 * e6809 for Raspberry Pi Pico
 *
 * @version     1.0.0
 * @author      smittytone
 * @copyright   2021
 * @licence     MIT
 *
 */
#ifndef _OPS_HEADER_
#define _OPS_HEADER_

#define NOP                 0x12
#define SYNC                0x13
#define DAA                 0x19
#define ORCC                0x1A
#define ANDCC               0x1C
#define SEX                 0x1D
#define EXG                 0x1E
#define TFR                 0x1F

#define BRA                 0x20
#define BSR                 0x21

#define NEGA                0x40
#define COMA                0x43
#define CLRA                0x4F

#define NEGB                0x50
#define COMB                0x53
#define CLRB                0x5F

#endif // _OPS_HEADER_
