
// Memory Address Constants

#define kTextScreenAddress                  1024
#define kGraphicsScreenAddress              1536
#define k32BasicRomAddress                  32768
#define k32CartridgeRomAddress              49152
#define kParallelIOAddress0                 65280
#define kParallelIOAddress1                 65312
#define kSamChipRegisterAddress             65472
#define kResetVectorsAddress                65520
#define kRamBasicSubroutineVectorsStart     350
#define kRamBasicSubroutineVectorsEnd       431

#define kRamRestartCode                     113

// Opcodes

#define opcode_NEG_direct                   0x00 //
#define opcode_COM_direct                   0x03 //
#define opcode_LSR_direct                   0x04 //
#define opcode_ROR_direct                   0x06 //
#define opcode_ASR_direct                   0x07 //
#define opcode_ASL_direct                   0x08 //
#define opcode_ROL_direct                   0x09 //
#define opcode_DEC_direct                   0x0A //
#define opcode_INC_direct                   0x0C //
#define opcode_TST_direct                   0x0D //

#define opcode_JMP_direct                   0x0E // UNTESTED
#define opcode_CLR_direct                   0x0F //

#define opcode_extended_set_1               0x10 //
#define opcode_extended_set_2               0x11 //

#define opcode_NOP                          0x12 //
#define opcode_SYNC                         0x13
#define opcode_LBRA_rel                     0x16 //
#define opcode_LBSR_rel                     0x17 //
#define opcode_DAA                          0x19 // BUG
#define opcode_ORCC_immed               	0x1A //
#define opcode_ANDCC_immed              	0x1C //
#define opcode_SEX                          0x1D // UNTESTED
#define opcode_EXG_immed                	0x1E //
#define opcode_TFR_immed                	0x1F //

#define opcode_BRA_rel                      0x20 // UNTESTED
#define opcode_BRN_rel                      0x21 // UNTESTED
#define opcode_BHI_rel                      0x22 // UNTESTED
#define opcode_BLS_rel                      0x23 // UNTESTED
#define opcode_BHS_rel                      0x24 // UNTESTED
#define opcode_BLO_rel                      0x25 // UNTESTED
#define opcode_BNE_rel                      0x26 // UNTESTED
#define opcode_BEQ_rel                      0x27 // UNTESTED
#define opcode_BVC_rel                      0x28 // UNTESTED
#define opcode_BVS_rel                      0x29 // UNTESTED
#define opcode_BPL_rel                      0x2A // UNTESTED
#define opcode_BMI_rel                      0x2B // UNTESTED
#define opcode_BGE_rel                      0x2C // UNTESTED
#define opcode_BLT_rel                      0x2D // UNTESTED
#define opcode_BGT_rel                      0x2E // UNTESTED
#define opcode_BLE_rel                      0x2F // UNTESTED

#define opcode_LEAX_indexed                 0x30 //
#define opcode_LEAY_indexed                 0x31 //
#define opcode_LEAS_indexed                 0x32 //
#define opcode_LEAU_indexed                 0x33 //

#define opcode_PSHS_immed               	0x34 //
#define opcode_PULS_immed               	0x35 //
#define opcode_PSHU_immed               	0x36 //
#define opcode_PULU_immed               	0x37 //

#define opcode_RTS                          0x39 //
#define opcode_ABX                          0x3A //
#define opcode_RTI                          0x3B //
#define opcode_CWAI_immed               	0x3C //
#define opcode_MUL                          0x3D //
#define opcode_SWI                          0x3F // UNTESTED

#define opcode_NEGA                         0x40 // Note: Immediate addressing
#define opcode_COMA                         0x43 //
#define opcode_LSRA                         0x44 //
#define opcode_RORA                         0x46 //
#define opcode_ASRA                         0x47 //
#define opcode_ASLA                         0x48 //
#define opcode_ROLA                         0x49 //
#define opcode_DECA                         0x4A //
#define opcode_INCA                         0x4C //
#define opcode_TSTA                         0x4D //
#define opcode_CLRA                         0x4F //

#define opcode_NEGB                         0x50 // Note: Immediate addressing
#define opcode_COMB                         0x53 //
#define opcode_LSRB                         0x54 //
#define opcode_RORB                         0x56 //
#define opcode_ASRB                         0x57 //
#define opcode_ASLB                         0x58 //
#define opcode_ROLB                         0x59 //
#define opcode_DECB                         0x5A //
#define opcode_INCB                         0x5C //
#define opcode_TSTB                         0x5D //
#define opcode_CLRB                         0x5F //

#define opcode_NEG_indexed                  0x60 //
#define opcode_COM_indexed                  0x63 //
#define opcode_LSR_indexed                  0x64 //
#define opcode_ROR_indexed                  0x66 //
#define opcode_ASR_indexed                  0x67 //
#define opcode_ASL_indexed                  0x68 //
#define opcode_ROL_indexed                  0x69 //
#define opcode_DEC_indexed                  0x6A //
#define opcode_INC_indexed                  0x6C //
#define opcode_TST_indexed                  0x6D //
#define opcode_JMP_indexed                  0x6E //
#define opcode_CLR_indexed                  0x6F //

#define opcode_NEG_extended                 0x70
#define opcode_COM_extended                 0x73
#define opcode_LSR_extended                 0x74
#define opcode_ROR_extended                 0x76
#define opcode_ASR_extended                 0x77
#define opcode_ASL_extended                 0x78
#define opcode_ROL_extended                 0x79
#define opcode_DEC_extended                 0x7A
#define opcode_INC_extended                 0x7C
#define opcode_TST_extended                 0x7D
#define opcode_JMP_extended                 0x7E
#define opcode_CLR_extended                 0x7F

#define opcode_SUBA_immed               	0x80
#define opcode_CMPA_immed               	0x81
#define opcode_SBCA_immed               	0x82
#define opcode_SUBD_immed               	0x83
#define opcode_ANDA_immed              	 	0x84
#define opcode_BITA_immed              	 	0x85
#define opcode_LDA_immed					0x86
#define opcode_EORA_immed               	0x88
#define opcode_ADCA_immed               	0x89
#define opcode_ORA_immed                	0x8A
#define opcode_ADDA_immed               	0x8B
#define opcode_CMPX_immed               	0x8C
#define opcode_BSR_rel                      0x8D
#define opcode_LDX_immed                	0x8E

#define opcode_SUBA_direct                  0x90
#define opcode_CMPA_direct                  0x91
#define opcode_SBCA_direct                  0x92
#define opcode_SUBD_direct                  0x93
#define opcode_ANDA_direct                  0x94
#define opcode_BITA_direct                  0x95
#define opcode_LDA_direct                   0x96
#define opcode_STA_direct                   0x97
#define opcode_EORA_direct                  0x98
#define opcode_ADCA_direct                  0x99
#define opcode_ORA_direct                   0x9A
#define opcode_ADDA_direct                  0x9B
#define opcode_CMPX_direct                  0x9C
#define opcode_JSR_direct                   0x9D
#define opcode_LDX_direct                   0x9E
#define opcode_STX_direct                   0x9F

#define opcode_SUBA_indexed                 0xA0
#define opcode_CMPA_indexed                 0xA1
#define opcode_SBCA_indexed                 0xA2
#define opcode_SUBD_indexed                 0xA3
#define opcode_ANDA_indexed                 0xA4
#define opcode_BITA_indexed                 0xA5
#define opcode_LDA_indexed                  0xA6
#define opcode_STA_indexed                  0xA7
#define opcode_EORA_indexed                 0xA8
#define opcode_ADCA_indexed                 0xA9
#define opcode_ORA_indexed                  0xAA
#define opcode_ADDA_indexed                 0xAB
#define opcode_CMPX_indexed                 0xAC
#define opcode_JSR_indexed                  0xAD
#define opcode_LDX_indexed                  0xAE
#define opcode_STX_indexed                  0xAF

#define opcode_SUBA_extended                0xB0
#define opcode_CMPA_extended                0xB1
#define opcode_SBCA_extended                0xB2
#define opcode_SUBD_extended                0xB3
#define opcode_ANDA_extended                0xB4
#define opcode_BITA_extended                0xB5
#define opcode_LDA_extended                 0xB6
#define opcode_STA_extended                 0xB7
#define opcode_EORA_extended                0xB8
#define opcode_ADCA_extended                0xB9
#define opcode_ORA_extended                 0xBA
#define opcode_ADDA_extended                0xBB
#define opcode_CMPX_extended                0xBC
#define opcode_JSR_extended                 0xBD
#define opcode_LDX_extended                 0xBE
#define opcode_STX_extended                 0xBF

#define opcode_SUBB_immed					0xC0
#define opcode_CMPB_immed               	0xC1
#define opcode_SBCB_immed               	0xC2
#define opcode_ADDD_immed               	0xC3
#define opcode_ANDB_immed               	0xC4
#define opcode_BITB_immed					0xC5
#define opcode_LDB_immed					0xC6
#define opcode_EORB_immed               	0xC8
#define opcode_ADCB_immed               	0xC9
#define opcode_ORB_immed                	0xCA
#define opcode_ADDB_immed               	0xCB
#define opcode_LDD_immed                	0xCC
#define opcode_LDU_immed                	0xCE

#define opcode_SUBB_direct                  0xD0
#define opcode_CMPB_direct                  0xD1
#define opcode_SBCB_direct                  0xD2
#define opcode_ADDD_direct                  0xD3
#define opcode_ANDB_direct                  0xD4
#define opcode_BITB_direct                  0xD5
#define opcode_LDB_direct                   0xD6
#define opcode_STB_direct                   0xD7
#define opcode_EORB_direct                  0xD8
#define opcode_ADCB_direct                  0xD9
#define opcode_ORB_direct                   0xDA
#define opcode_ADDB_direct                  0xDB
#define opcode_LDD_direct                   0xDC
#define opcode_STD_direct                   0xDD
#define opcode_LDU_direct                   0xDE
#define opcode_STU_direct                   0xDF

#define opcode_SUBB_indexed                 0xE0
#define opcode_CMPB_indexed                 0xE1
#define opcode_SBCB_indexed                 0xE2
#define opcode_ADDD_indexed                 0xE3
#define opcode_ANDB_indexed                 0xE4
#define opcode_BITB_indexed                 0xE5
#define opcode_LDB_indexed                  0xE6
#define opcode_STB_indexed                  0xE7
#define opcode_EORB_indexed                 0xE8
#define opcode_ADCB_indexed                 0xE9
#define opcode_ORB_indexed                  0xEA
#define opcode_ADDB_indexed                 0xEB
#define opcode_LDD_indexed                  0xEC
#define opcode_STD_indexed                  0xED
#define opcode_LDU_indexed                  0xEE
#define opcode_STU_indexed                  0xEF

#define opcode_SUBB_extended                0xF0
#define opcode_CMPB_extended                0xF1
#define opcode_SBCB_extended                0xF2
#define opcode_ADDD_extended                0xF3
#define opcode_ANDB_extended                0xF4
#define opcode_BITB_extended                0xF5
#define opcode_LDB_extended                 0xF6
#define opcode_STB_extended                 0xF7
#define opcode_EORB_extended                0xF8
#define opcode_ADCB_extended                0xF9
#define opcode_ORB_extended                 0xFA
#define opcode_ADDB_extended                0xFB
#define opcode_LDD_extended                 0xFC
#define opcode_STD_extended                 0xFD
#define opcode_LDU_extended                 0xFE
#define opcode_STU_extended                 0xFF

// Extended opcode start here

#define opcode_LBRN_rel                     0x1021
#define opcode_LBHI_rel                     0x1022
#define opcode_LBLS_rel                     0x1023
#define opcode_LBHS_rel                     0x1024
#define opcode_LBLO_rel                     0x1025
#define opcode_LBNE_rel                     0x1026
#define opcode_LBEQ_rel                     0x1027
#define opcode_LBVC_rel                     0x1028
#define opcode_LBVS_rel                     0x1029
#define opcode_LBPL_rel                     0x102A
#define opcode_LBMI_rel                     0x102B
#define opcode_LBGE_rel                     0x102C
#define opcode_LBLT_rel                     0x102D

#define opcode_LBGT_rel                     0x102E
#define opcode_LBLE_rel                     0x102F
#define opcode_SWI2                         0x103F
#define opcode_CMPD_immed               	0x1083
#define opcode_CMPY_immed               	0x108C
#define opcode_LDY_immed                	0x108E
#define opcode_CMPD_direct                  0x1093
#define opcode_CMPY_direct                  0x109C
#define opcode_LDY_direct                   0x109E
#define opcode_STY_direct                   0x109F
#define opcode_CMPD_indexed                 0x10A3
#define opcode_CMPY_indexed                 0x10AC
#define opcode_LDY_indexed                  0x10AE
#define opcode_STY_indexed                  0x10AF
#define opcode_CMPD_extended                0x10B3
#define opcode_CMPY_extended                0x10BC
#define opcode_LDY_extended                 0x10BE
#define opcode_STY_extended                 0x10BF

#define opcode_LDS_immed                	0x10CE
#define opcode_LDS_direct                   0x10DE
#define opcode_STS_direct                   0x10DF
#define opcode_LDS_indexed                  0x10EE
#define opcode_STS_indexed                  0x10EF
#define opcode_LDS_extended                 0x10FE
#define opcode_STS_extended                 0x10FF

// Second extended opcodes

#define opcode_SWI3                         0x113F
#define opcode_CMPU_immed               	0x1183
#define opcode_CMPS_immed               	0x118C
#define opcode_CMPU_direct                  0x1193
#define opcode_CMPS_direct                  0x119C
#define opcode_CMPU_indexed                 0x11A3
#define opcode_CMPS_indexed                 0x11AC
#define opcode_CMPU_extended                0x11B3
#define opcode_CMPS_extended                0x11BC

// Colours

#define kColourGreen                        0
#define kColourYellow                       1
#define kColourBlue                         2
#define kColourRed                          3
#define kColourWhite                        4
#define kColourCyan                         5
#define kColourMagenta                      6
#define kColourOrange                       7

#define kBootTypeCold                       0
#define kBootTypeWarm                       1

#define kDragon32RomEntry                   32767
#define kDragon64RomEntry                   32767

// Register Values

#define kRegA                               0x08
#define kRegB                               0x09

#define kPushPullRegCC                      0x01
#define kPushPullRegAll                     0xFE
#define kPushPullRegEvery                   0xFF

#define kInterruptVectorOne                 0xFFFA
#define kInterruptVectorTwo                 0xFFF4
#define kInterruptVectorThree               0xFFF2

#define kCC_e                               7
#define kCC_f                               6
#define kCC_h                               5
#define kCC_i                               4
#define kCC_n                               3
#define kCC_z                               2
#define kCC_v                               1
#define kCC_c                               0

#define kSignBit                            7
