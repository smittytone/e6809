
#import "CPU.h"

@implementation MC6809


@synthesize regA;
@synthesize regB;
@synthesize regCC;
@synthesize regDP;
@synthesize regHSP;
@synthesize regUSP;
@synthesize regX;
@synthesize regY;
@synthesize regPC;



- (id)init
{
    self = [super init];
    
    if (self)
    {
        // Extra init code here
        
        regPC = 0;
        regHSP = 0;
        regUSP = 0;
        regCC = 0;
        
		[self configMemory:65535];
   	}
    
    return self;
}



- (void)configMemory:(NSUInteger)memSizeInBytes
{
	// Impose a minimum RAM of 1KB
    
    if (memSizeInBytes == 0) memSizeInBytes = 1024;
	if (memSizeInBytes > 65535) memSizeInBytes = 65535;
	
	topAddress = memSizeInBytes;
	
	ram = [[RAM alloc] init];
	[ram bootRam:topAddress];
	
	// Load in Standard Low RAM memory
	// "$15E-1AF are three-byte subroutines used by Basic. By default they return immediately with an RTS
	
	for (NSUInteger i = kRamBasicSubroutineVectorsStart ; i < (kRamBasicSubroutineVectorsEnd + 1) ; i++)
	{
		[ram poke:i :opcode_RTS];
	}
}



#pragma mark - Memory access


- (NSUInteger)addressFromNextTwoBytes
{
    // Reads the bytes at regPC and regPC + 1 and returns them as a 16-bit address
    // NOTE 'loadFromRAM:' auto-increments regPC and handles rollover

    NSUInteger msb = [self loadFromRam];
    NSUInteger lsb = [self loadFromRam];
    return (msb << 8) + lsb;
}


- (NSUInteger)addressFromDPR
{
    return [self addressFromDPR:0];
}


- (NSUInteger)addressFromDPR:(NSInteger)offset
{
    // Returns the address composed from regDP (MSB) and the byte at
    // regPC (LSB) plus any supplied offset
    // Does not increment regPC
    // TODO Should we increment regPC?
    return (regDP * 256) + [ram peek:(regPC + offset)];
}



- (NSUInteger)contentsOfMemory:(NSUInteger)address
{
    // Returns the byte at the specified address

    return [ram peek:address];
}



- (void)setContentsOfMemory:(NSUInteger)address :(NSUInteger)value
{
	[ram poke:address :value];
}



- (NSUInteger)loadFromRam
{
    // Return the byte at the address stored in regPC and
    // auto-increment regPC aftewards, handling rollover
    
    NSUInteger load = [self contentsOfMemory:regPC];
    regPC++;
    if (regPC > ram.topAddress) regPC = regPC & ram.topAddress;
    return load;
}



- (void)incrementPC:(NSUInteger)amount
{
    regPC += amount;
    if (regPC > ram.topAddress) regPC = regPC & ram.topAddress;
}



- (NSInteger)checkRange:(NSInteger)address
{
    // Handle RAM address rollover

    if (address < 0) return ram.topAddress - address;
    if (address > ram.topAddress) return address & ram.topAddress;
    return address & ram.topAddress;
}


#pragma mark - Utilities


- (void)clearBits
{
    // Clear all the values in the 'bit' array

    for (NSUInteger i = 0 ; i < 16 ; i++) bit[i] = 0;
}


- (void)decimalToBits:(NSUInteger)value
{
    // Convert the 8-bit byte 'value' into its binary equivalent stored as individual bits
    // in the array 'bit'
    // NOTE 'bit' can hold a 16-bit value, but we only use eight of the elements here

    [self clearBits];

    if (value & 0x80) bit[7] = 1;
    if (value & 0x40) bit[6] = 1;
    if (value & 0x20) bit[5] = 1;
    if (value & 0x10) bit[4] = 1;
    if (value & 0x08) bit[3] = 1;
    if (value & 0x04) bit[2] = 1;
    if (value & 0x02) bit[1] = 1;
    if (value & 0x01) bit[0] = 1;

    // At this point the machine-wide array bit[] contains an eight-bit binary representation
    // of the input decimal value, including the sign bit
}


- (void)decimal16ToBits:(NSUInteger)value
{
    // Convert the 16-bit byte 'value' into its binary equivalent stored as individual bits

    [self clearBits];

    // Convert the MSB

    [self decimalToBits:((value >> 8) & 0xFF)];

    // Shift the bits left

    for (NSUInteger i = 0 ; i < 8 ; i++) bit[i + 8] = bit[i];

    // Convert the LSB

    [self decimalToBits:(value & 0xFF)];
}


- (NSUInteger)bitsToDecimal
{
    // Reads the first eight bits of the 'bit' array and returns them as a decimal value

    NSUInteger a = (bit[3] * 0x08) + (bit[2] * 0x04) + (bit[1] * 0x02) + bit[0];
    a += (bit[4] * 0x10) + (bit[5] * 0x20) + (bit[6] * 0x40) + (bit[7] * 0x80);
    return a;
}


- (NSUInteger)bits16ToDecimal
{
    // Reads the full 16 bits of the 'bit' array and returns them as a decimal value

    NSUInteger a = bit[0];
    for (NSUInteger i = 1 ; i < 16 ; i++) a += (((NSUInteger)pow(2,i)) * bit[i]);
    return a;
}


- (BOOL)bitSet:(NSUInteger)value :(NSUInteger)bit
{
    // Set bit 'bit' of the byte or word 'value'
    // Doesn't yet check the value of bit

    return (((value >> bit) & 1) == 1);
}


- (NSUInteger)setBit:(NSUInteger)value :(NSUInteger)bit
{
    return (value | (1 << bit));
}


- (NSUInteger)clrBit:(NSUInteger)value :(NSUInteger)bit
{
    return (value & ~(1 << bit));
}


- (void)setCCV
{
    regCC = [self setBit:regCC :kCC_v];
}


- (void)clrCCV
{
    regCC = [self clrBit:regCC :kCC_v];
}


- (void)setCCZ
{
    regCC = [self setBit:regCC :kCC_z];
}


- (void)clrCCZ
{
    regCC = [self clrBit:regCC :kCC_z];
}


- (void)setCCN
{
    regCC = [self setBit:regCC :kCC_n];
}


- (void)clrCCN
{
    regCC = [self clrBit:regCC :kCC_n];
}


- (void)setCCC
{
    regCC = [self setBit:regCC :kCC_c];
}


- (void)clrCCC
{
    regCC = [self clrBit:regCC :kCC_c];
}


- (NSInteger)unsign8ToSign8:(NSInteger)value
{
    // Set the signed value to the first seven bits of the unsigned value
    NSInteger a = value & 0x7F;

    // If the sign bit is set, multiply the signed value by -1
    if ([self bitSet:value :7]) a = a * -1;

    return a;
}


- (NSUInteger)alu:(NSUInteger)value1 :(NSUInteger)value2 :(BOOL)useCarry
{
    // Simulates addition of two unsigned 8-bit values in a binary ALU
    // Checks for half-carry, carry and overflow (CC bits h, c, v)

    NSUInteger binary1[8], binary2[8], answer[8];

    BOOL bitCarry = NO;
    BOOL bit6Carry = NO;

    [self clrCCV];

    [self decimalToBits:(value1 & 0xFF)];
    for (NSUInteger i = 0 ; i < 8 ; i++) binary1[i] = bit[i];

    [self decimalToBits:(value2 & 0xFF)];
    for (NSUInteger i = 0 ; i < 8 ; i++) binary2[i] = bit[i];

	if (useCarry) bitCarry = [self bitSet:regCC :kCC_c];

    for (NSUInteger i = 0 ; i < 8 ; i++)
    {
        if (binary1[i] == binary2[i])
        {
            // Both digits are the same, ie. 1 and 1, or 0 and 0,
            // so added value is 0 + a carry to the next digit

            answer[i] = bitCarry ? 1 : 0;
            bitCarry = binary1[i] == 1;
        }
        else
        {
            // Both digits are different, ie. 1 and 0, or 0 and 1, so the
            // added value is 1 plus any carry from the previous addition
            // 1 + 1 -> 0 + carry 1, or 0 + 1 -> 1

            answer[i] = bitCarry ? 0 : 1;
        }

		// Check for half carry, ie. result of bit 3 add is a carry into bit 4

        if (i == 3 && bitCarry == YES) regCC = [self setBit:regCC :kCC_h];

        // Record bit 6 carry for carry check
        
		if (i == 6) bit6Carry = bitCarry;
    }

    // Preserve the final carry value in the CC register's c bit

    regCC = bitCarry ? [self setBit:regCC :kCC_c] : [self clrBit:regCC :kCC_c];
    
    // Check for an overflow:
    // 1. Sum of two positive numbers yields sign bit set
    // 2. Sum of two negative numbers yields sign bit unset

	if (binary1[7] == 1 && binary2[7] == 1 && answer[7] == 0) [self setBit:regCC :kCC_v];
    if (binary1[7] == 0 && binary2[7] == 0 && answer[7] == 1) [self setBit:regCC :kCC_v];

    // Copy answer into bits[] array for conversion to decimal

	for (NSUInteger i = 0 ; i < 8 ; i++) bit[i] = answer[i];

    // Return the answer (condition codes already set elsewhere)

	return [self bitsToDecimal];
}



- (NSUInteger)alu16:(NSUInteger)value1 :(NSUInteger)value2 :(BOOL)useCarry
{
    // Add the LSBs
    
    NSUInteger lsb1 = value1 & 0xFF;
    NSUInteger lsb2 = value2 & 0xFF;
    NSUInteger total = [self alu:lsb1 :lsb2 :useCarry];
    
    // Now add the MSBs, using the carry (if any) from the LSB calculation
    
    NSUInteger msb1 = (value1 >> 8) & 0xFF;
    NSUInteger msb2 = (value2 >> 8) & 0xFF;
    NSUInteger subtotal = [self alu:msb1 :msb2 :YES];
    
    return ((subtotal << 8) + total) & 0xFFFF;
}



#pragma mark - Process Instructions

- (void)processNextInstruction  
{
	// At this point the PC register should be pointing to the instruction to process,
	// ie. previous operations will have moved PC on sufficiently
	
	NSUInteger i, operand, opcode, address, msb, lsb;

    // Are we awaiting an interrupt? If so check/handle then bail
    
    if (waitForInterruptFlag == YES)
	{
		// Process interrupts

		return;
	}
    
    // Get the memory cell contents
    
	opcode = [self loadFromRam];
    
    // Zero auxilliary variables
    
    operand = 0;
    
    if (opcode == opcode_extended_set_1 || opcode == opcode_extended_set_2)
    {
        // We have an extended instruction prefixed with 0x10 or 0x11
		// Move the PC along one and get the next opcode portion from RAM
        
        opcode = (opcode << 8) + [self loadFromRam];
    }
    
    // Program counter already pointing to the operand at this point
    
    switch (opcode)
    {
		case opcode_NEG_direct:
			address = [self addressFromDPR];
            [self setContentsOfMemory:address :[self negate:[ram peek:address]]];
			[self incrementPC:1];
			break;

		case opcode_COM_direct:
			address = [self addressFromDPR];
			[ram poke:operand :[self complement:[ram peek:operand]]];
			[self incrementPC:1];
			break;

		case opcode_LSR_direct:
			address = [self addressFromDPR];
			[ram poke:operand :[self lShiftRight:[ram peek:operand]]];
			[self incrementPC:1];
			break;

		case opcode_ROR_direct:
			address = [self addressFromDPR];
			[ram poke:operand :[self rotateRight:[ram peek:operand]]];
			[self incrementPC:1];
			break;

		case opcode_ASR_direct:
			address = [self addressFromDPR];
			[ram poke:operand :[self aShiftRight:[ram peek:operand]]];
			[self incrementPC:1];
			break;

		case opcode_ASL_direct:
			address = [self addressFromDPR];
			[ram poke:operand :[self aShiftLeft:[ram peek:operand]]];
			[self incrementPC:1];
			break;

		case opcode_ROL_direct:
			address = [self addressFromDPR];
			[ram poke:operand :[self rotateLeft:[ram peek:operand]]];
			[self incrementPC:1];
			break;

		case opcode_DEC_direct:
			address = [self addressFromDPR];
			[ram poke:operand :[self decrement:[ram peek:operand]]];
			[self incrementPC:1];
			break;

		case opcode_INC_direct:
			address = [self addressFromDPR];
			[ram poke:operand :[self increment:[ram peek:operand]]];
			[self incrementPC:1];
			break;

		case opcode_TST_direct:
			address = [self addressFromDPR];
			[self test:[ram peek:operand]];
			[self incrementPC:1];
			break;

// JMP UNTESTED

		case opcode_JMP_direct:
			regPC = [self addressFromDPR:0];
			break;

		case opcode_CLR_direct:
			operand = [self addressFromDPR:0];
			[ram poke:operand :0];
			[self clear];
			[self incrementPC:1];
			break;

		case opcode_NOP:
			// No OPeration
            // NOTE We may need to add a 2 x CPU cycle delay here
            break;

// SYNC UNTESTED

		case opcode_SYNC:

			/*
			 The processor halts and waits for an interrupt to occur. If the interrupt is masked (disabled) or is shorter than 3 cycles,
             then the processor continues execution with the instruction following the SYNC instruction. If the interrupt is enabled and
             lasts more than 3 cycles, then a normal interrupt sequence begins. The return address pushed onto the stack is that of the
             instruction following the SYNC instruction. This instruction can be used to synchronize the processor with high speed,
             critical events, such as reading data from a disk drive.
			 */

			waitForInterruptFlag = YES;
			break;

		case opcode_LBRA_rel:
			regPC += (short)([ram peek:regPC] * 256 + [ram peek:regPC + 1]);
			break;

		case opcode_LBSR_rel:
			msb = (regPC >> 8) & 0xFF;
			lsb = regPC & 0xFF;
			regHSP--;
			[ram poke:regHSP :lsb];
			regHSP--;
			[ram poke:regHSP :msb];
			break;

// DAA buggy
		case opcode_DAA:
			// Decimal Adjust A
            [self decimalAdjustA];
			break;

		case opcode_ORCC_immed:
			// OR the CC register
            [self orcc:[ram peek:regPC]];
			[self incrementPC:1];
			break;

		case opcode_ANDCC_immed:
			[self andcc:[ram peek:regPC]];
			[self incrementPC:1];
			break;

// SEX UNTESTED
		case opcode_SEX:
			// Sign EXtend B to D
            // Affects n and z

            [self clrCCN];
            [self clrCCZ];
            
			regA = 0x00;

			if ([self bitSet:regB :kSignBit])
			{
				regA = 0xFF;
				[self setCCN];
			}
            
            if (regB == 0) [self setCCZ]; // CHECK
			break;

		case opcode_EXG_immed:
			[self transferDecode:[ram peek:regPC] :YES];
			[self incrementPC:1];
            break;

		case opcode_TFR_immed:
			[self transferDecode:[ram peek:regPC] :NO];
            [self incrementPC:1];
            break;

// Branches untested
            
		case opcode_BRA_rel:
			regPC += (char)[ram peek:regPC];
			break;

		case opcode_BRN_rel:
			[self incrementPC:1];
			break;
			
		case opcode_BHI_rel:
            if (![self bitSet:regCC :kCC_c] && ![self bitSet:regCC :kCC_z]) regPC += (char)[ram peek:regPC];
			break;

		case opcode_BLS_rel:
			if ([self bitSet:regCC :kCC_c] && [self bitSet:regCC :kCC_z]) regPC += (char)[ram peek:regPC];
			break;

		case opcode_BHS_rel:
			if (![self bitSet:regCC :kCC_c]) regPC += (char)[ram peek:regPC];
			break;

		case opcode_BLO_rel:
			if ([self bitSet:regCC :kCC_c]) regPC += (char)[ram peek:regPC];

		case opcode_BNE_rel:
			if (![self bitSet:regCC :kCC_z]) regPC += (char)[ram peek:regPC];
			break;

		case opcode_BEQ_rel:
			if ([self bitSet:regCC :kCC_z]) regPC += (char)[ram peek:regPC];
			break;

		case opcode_BVC_rel:
			if (![self bitSet:regCC :kCC_v]) regPC += (char)[ram peek:regPC];
			break;

		case opcode_BVS_rel:
			if ([self bitSet:regCC :kCC_v]) regPC += (char)[ram peek:regPC];
			break;

		case opcode_BPL_rel:
			if (![self bitSet:regCC :kCC_n]) regPC += (char)[ram peek:regPC];
			break;

		case opcode_BMI_rel:
			if ([self bitSet:regCC :kCC_n]) regPC += (char)[ram peek:regPC];
			break;

		case opcode_BGE_rel:
			if ([self bitSet:regCC :kCC_n] == [self bitSet:regCC :kCC_v]) regPC += (char)[ram peek:regPC];
			break;

		case opcode_BLT_rel:
			if (([self bitSet:regCC :kCC_n] || [self bitSet:regCC :kCC_v]) && [self bitSet:regCC :kCC_n] != [self bitSet:regCC :kCC_v])
                regPC += (char)[ram peek:regPC];
			break;

		case opcode_BGT_rel:
            if (![self bitSet:regCC :kCC_z] && [self bitSet:regCC :kCC_n] == [self bitSet:regCC :kCC_v])
                regPC += (char)[ram peek:regPC];
			break;

		case opcode_BLE_rel:
			if ([self bitSet:regCC :kCC_z])
			{
                regPC += (char)[ram peek:regPC];
			}
			else if (([self bitSet:regCC :kCC_n] || [self bitSet:regCC :kCC_v]) && [self bitSet:regCC :kCC_n] != [self bitSet:regCC :kCC_v])
            {
                regPC += (char)[ram peek:regPC];
			}
			break;

		case opcode_LEAX_indexed:
			// Load Effective Address into X
            // NOTE 'indexedAddressing:' updates PC
            regX = [self indexedAddressing:[ram peek:regPC]];
            [self clrCCZ];
            if (regX == 0) [self setCCZ];
			break;

		case opcode_LEAY_indexed:
            // Load Effective Address into Y
            // NOTE 'indexedAddressing:' updates PC
            regY = [self indexedAddressing:[ram peek:regPC]];
			[self clrCCZ];
			if (regY == 0) [self setCCZ];
			break;

		case opcode_LEAS_indexed:
            // Load Effective Address into S
            // NOTE 'indexedAddressing:' updates PC
            regHSP = [self indexedAddressing:[ram peek:regPC]];
			break;

		case opcode_LEAU_indexed:
            // Load Effective Address into U
            // NOTE 'indexedAddressing:' updates PC
            regUSP = [self indexedAddressing:[ram peek:regPC]];
			break;

		case opcode_PSHS_immed:
			[self push:YES :[ram peek:regPC]];
            [self incrementPC:1];
            break;

		case opcode_PULS_immed:
            // Progress the PC now in case a new value is NOT loaded
            // as we don't want to progress a new counter value
            i = regPC;
            [self incrementPC:1];
            [self pull:YES :[ram peek:i]];
			break;

		case opcode_PSHU_immed:
			[self push:NO :[ram peek:regPC]];
            [self incrementPC:1];
            break;

		case opcode_PULU_immed:
            // Progress the PC now in case a new value is NOT loaded
            // as we don't want to progress a new counter value
            i = regPC;
            [self incrementPC:1];
            [self pull:NO :[ram peek:i]];
			break;

		case opcode_RTS:
			// Return from Subroutine
            regPC = ([ram peek:regHSP] * 256);
            regHSP++;
            if (regHSP > 0xFFFF) regHSP = 0x0000;
            regPC += [ram peek:regHSP];
            regHSP++;
            if (regHSP > 0xFFFF) regHSP = 0x0000;
			break;

		case opcode_ABX:
            // Add Accumulator B into Index Register X
            // Does not affect CC
            regX += regB;
            if (regX > 0xFFFF) regX = (regX & 0xFFFF);
			break;

		case opcode_RTI:
            // Return from Interrupt: pull CC from the hardware stack
            // If e is set, pull all the registers from the hardware stack,
            // otherwise pull the PC register only
            [self pull:YES :kPushPullRegCC];
            if ([self bitSet:regCC :kCC_e]) [self pull:YES :kPushPullRegAll];
			break;

		case opcode_CWAI_immed:
            // Clear CC bits and Wait for Interrupt
            // AND CC with the operand, set e, then push every register,
            // including CC to the hardware stack
			regCC = [self and:regCC with:[ram peek:regPC]];
            regCC = [self setBit:regCC :kCC_e];
			[self push:YES :kPushPullRegEvery];
			waitForInterruptFlag = YES;
			break;

		case opcode_MUL:
			// Multiply A and B, and put the answer MSB into A, LSB into B
            [self multiply];
			break;

		case opcode_SWI:
            // SoftWare Interrupt
            // Set e to 1 then push every register to the hardware stack
            
            regCC = [self setBit:regCC :kCC_e];
            [self push:YES :kPushPullRegEvery];
            
            // Set i and f
            regCC = [self setBit:regCC :kCC_i];
            regCC = [self setBit:regCC :kCC_f];
			waitForInterruptFlag = NO;
            
            // Set the PC to the interrupt vector
			regPC = [ram peek:kInterruptVectorOne] * 256 + [ram peek:kInterruptVectorOne + 1];
			break;
			
		case opcode_NEGA:
			regA = [self negate:regA];
			break;

		case opcode_COMA:
			regA = [self complement:regA];
			break;

		case opcode_LSRA:
			regA = [self lShiftRight:regA];
			break;

		case opcode_RORA:
			regA = [self rotateRight:regA];
			break;

		case opcode_ASRA:
			regA = [self aShiftRight:regA];
			break;

		case opcode_ASLA:
			regA = [self aShiftLeft:regA];
			break;

		case opcode_ROLA:
			regA = [self rotateLeft:regA];
			break;
			
		case opcode_DECA:
			regA = [self decrement:regA];
			break;

		case opcode_INCA:
			regA = [self increment:regA];
			break;

		case opcode_TSTA:
			[self test:regA];
			break;

		case opcode_CLRA:
			regA = 0;
			[self clear];
			break;
			
		case opcode_NEGB:
			regB = [self negate:regB];
			break;

		case opcode_COMB:
			regB = [self complement:regB];
			break;

		case opcode_LSRB:
			regB = [self lShiftRight:regB];
			break;

		case opcode_RORB:
			regB = [self rotateRight:regB];
			break;

		case opcode_ASRB:
			regB = [self aShiftRight:regB];
			break;

		case opcode_ASLB:
			regB = [self aShiftLeft:regB];
			break;
			
		case opcode_ROLB:
			regB = [self rotateLeft:regB];
			break;

		case opcode_DECB:
			regB = [self decrement:regB];
			break;

		case opcode_INCB:
			regB = [self increment:regB];
			break;

		case opcode_TSTB:
			[self test:regB];
			break;

		case opcode_CLRB:
			regB = 0;
			[self clear];
			break;

		case opcode_NEG_indexed:
			address = [self indexedAddressing:[ram peek:regPC]];
			[ram poke:address :[self negate:[ram peek:address]]];
			break;

		case opcode_COM_indexed:
			address = [self indexedAddressing:[ram peek:regPC]];
			[ram poke:address :[self complement:[ram peek:address]]];
			break;

		case opcode_LSR_indexed:
			address = [self indexedAddressing:[ram peek:regPC]];
			[ram poke:address :[self lShiftRight:[ram peek:address]]];
			break;

		case opcode_ROR_indexed:
			address = [self indexedAddressing:[ram peek:regPC]];
			[ram poke:address :[self rotateRight:[ram peek:address]]];
			break;

		case opcode_ASR_indexed:
			address = [self indexedAddressing:[ram peek:regPC]];
			[ram poke:address :[self aShiftRight:[ram peek:address]]];
			break;

		case opcode_ASL_indexed:
			address = [self indexedAddressing:[ram peek:regPC]];
			[ram poke:address :[self aShiftLeft:[ram peek:address]]];
			break;

		case opcode_ROL_indexed:
			address = [self indexedAddressing:[ram peek:regPC]];
			[ram poke:address :[self rotateLeft:[ram peek:address]]];
			break;

		case opcode_DEC_indexed:
			address = [self indexedAddressing:[ram peek:regPC]];
			[ram poke:address :[self decrement:[ram peek:address]]];
			break;

		case opcode_INC_indexed:
			address = [self indexedAddressing:[ram peek:regPC]];
			[ram poke:address :[self increment:[ram peek:address]]];
			break;

		case opcode_TST_indexed:
			i = [self indexedAddressing:[ram peek:regPC]];
			[self test:[ram peek:i]];
			break;

		case opcode_CLR_indexed:
			address = [self indexedAddressing:[ram peek:regPC]];
			[ram poke:address :0];
			[self clear];
			break;
			
		case opcode_NEG_extended:
            address = [self addressFromNextTwoBytes];
			[ram poke:address :[self negate:[ram peek:address]]];
			break;

		case opcode_COM_extended:
            address = [self addressFromNextTwoBytes];
            [ram poke:address :[self complement:[ram peek:address]]];
			break;
			
		case opcode_LSR_extended:
            address = [self addressFromNextTwoBytes];
            [ram poke:address :[self lShiftRight:[ram peek:address]]];
			regPC = regPC + 2;
			break;
			
		case opcode_ROR_extended:
			address = [self addressFromNextTwoBytes];
			[ram poke:address :[self rotateRight:[ram peek:address]]];
			break;
			
		case opcode_ASR_extended:
			address = [self addressFromNextTwoBytes];
			[ram poke:address :[self aShiftRight:[ram peek:address]]];
			break;

		case opcode_ASL_extended:
			address = [self addressFromNextTwoBytes];
			[ram poke:address :[self aShiftLeft:[ram peek:address]]];
			break;

		case opcode_ROL_extended:
			address = [self addressFromNextTwoBytes];
			[ram poke:address :[self rotateLeft:[ram peek:address]]];
			break;

		case opcode_DEC_extended:
			address = [self addressFromNextTwoBytes];
			[ram poke:address :[self decrement:[ram peek:address]]];
			break;

		case opcode_INC_extended:
			address = [self addressFromNextTwoBytes];
			[ram poke:address :[self increment:[ram peek:address]]];
			break;

		case opcode_TST_extended:
			address = [self addressFromNextTwoBytes];
			[self test:[ram peek:address]];
			break;

		case opcode_JMP_extended:
			regPC = [self addressFromNextTwoBytes];
			break;

		case opcode_CLR_extended:
			address = [self addressFromNextTwoBytes];
			[ram poke:address :0];
			[self clear];
			break;

		case opcode_SUBA_immed:
			regA = [self sub:regA :[ram peek:regPC]];
			[self incrementPC:1];
			break;

		case opcode_CMPA_immed:
			[self compare:regA :[ram peek:regPC]];
			[self incrementPC:1];
			break;

		case opcode_SBCA_immed:
			regA = [self subWithCarry:regA :[ram peek:regPC]];
			[self incrementPC:1];
			break;
			
		case opcode_SUBD_immed:
			break;

		case opcode_ANDA_immed:
			regA = [self and:regA with:[ram peek:regPC]];
			[self incrementPC:1];
			break;
			
		case opcode_BITA_immed:
			[self bitTest:regA with:[ram peek:regPC]];
			[self incrementPC:1];
			break;

		case opcode_LDA_immed:
			[self load:[ram peek:regPC] :kRegA];
			[self incrementPC:1];
			break;

		case opcode_EORA_immed:
			regA = [self exclusiveor:regA :[ram peek:regPC]];
			[self incrementPC:1];
			break;

		case opcode_ADCA_immed:
			regA = [self addWithCarry:regA :[ram peek:regPC]];
			[self incrementPC:1];
			break;

		case opcode_ORA_immed:
			regA = [self orr:regA :[ram peek:regPC]];
			[self incrementPC:1];
			break;

		case opcode_ADDA_immed:
			regA = [self add:regA :[ram peek:regPC]];
			[self incrementPC:1];
			break;

		case opcode_CMPX_immed:
            address = [self addressFromNextTwoBytes];
            [self compare16bit:regX :address];
			break;

		case opcode_BSR_rel:
			regHSP = regHSP - 1;
			[ram poke:regHSP :regPC & 0xFF];
			regHSP = regHSP - 1;
			[ram poke:regHSP :(regPC >> 8) & 0xFF];
			regPC = [ram peek:regPC];
			break;

		case opcode_LDX_immed:
			regX = [self addressFromNextTwoBytes];
            [self load16bit:regX];
			break;

		case opcode_SUBA_direct:
			address = [self addressFromDPR:0];
			regA = [self sub:regA :[ram peek:address]];
			[self incrementPC:1];
			break;

		case opcode_CMPA_direct:
			address = [self addressFromDPR:0];
			[self compare:regA :[ram peek:address]];
			[self incrementPC:1];
			break;

		case opcode_SBCA_direct:
			address = [self addressFromDPR:0];
			regA = [self subWithCarry:regA :[ram peek:address]];
			[self incrementPC:1];
			break;

		case opcode_SUBD_direct:
			address = [self addressFromDPR:0];
			[self subd:address];
			[self incrementPC:1];
			break;

		case opcode_ANDA_direct:
			address = [self addressFromDPR:0];
			regA = [self and:regA with:[ram peek:address]];
			[self incrementPC:1];
			break;

		case opcode_BITA_direct:
			address = [self addressFromDPR:0];
			[self bitTest:regA with:[ram peek:address]];
			[self incrementPC:1];
			break;
			
		case opcode_LDA_direct:
			address = [self addressFromDPR:0];
			[self load:[ram peek:address] :kRegA];
			[self incrementPC:1];
			break;

		case opcode_STA_direct:
			address = [self addressFromDPR:0];
			[ram poke:address :regA];
			[self store:regA];
			[self incrementPC:1];
			break;

		case opcode_EORA_direct:
            address = [self addressFromDPR:0];
            regA = [self exclusiveor:regA :[ram peek:address]];
            [self incrementPC:1];
            break;

		case opcode_ADCA_direct:
			address = [self addressFromDPR:0];
			regA = [self addWithCarry:regA :[ram peek:address]];
			[self incrementPC:1];
			break;

		case opcode_ORA_direct:
			address = [self addressFromDPR:0];
			regA = [self orr:regA :[ram peek:address]];
			[self incrementPC:1];
			break;

		case opcode_ADDA_direct:
			address = [self addressFromDPR:0];
			regA = [self add:regA :[ram peek:address]];
			[self incrementPC:1];
			break;
			
		case opcode_CMPX_direct:
			address = [self addressFromDPR:0];
			[self compare16bit:regX :address];
			[self incrementPC:1];
			break;

		case opcode_JSR_direct:
			regHSP = regHSP - 1;
			[ram poke:regHSP :(regPC >> 8) & 0xFF];
			regHSP = regHSP - 1;
			[ram poke:regHSP :regPC & 0xFF];
			regPC = [self addressFromDPR:0];
			break;

		case opcode_LDX_direct:
			address = [self addressFromDPR:0];
			regX = ([ram peek:address] << 8) + [ram peek:address + 1];
			[self load16bit:regX];
			[self incrementPC:1];
			break;

		case opcode_STX_direct:
			address = [self addressFromDPR:0];
			[ram poke:address :(regX >> 8) & 0xFF];
			[ram poke:address + 1 :regX & 0xFF];
			[self store16bit:regX];
			[self incrementPC:1];
			break;

		case opcode_SUBA_indexed:
			break;
			
		case opcode_CMPA_indexed:
            address = [self indexedAddressing:[ram peek:regPC]];
            [self compare:regA :[ram peek:address]];
            break;
			
		case opcode_SBCA_indexed:
			break;
			
		case opcode_SUBD_indexed:
			break;
			
		case opcode_ANDA_indexed:
            address = [self indexedAddressing:[ram peek:regPC]];
            regA = [self and:regA with:[ram peek:address]];
            break;
			
		case opcode_BITA_indexed:
            address = [self indexedAddressing:[ram peek:regPC]];
            [self bitTest:regA with:[ram peek:address]];
            break;
			
		case opcode_LDA_indexed:
			address = [self indexedAddressing:[ram peek:regPC]];
            [self load:[ram peek:address] :kRegA];
            break;
			
		case opcode_STA_indexed:
			break;
			
		case opcode_EORA_indexed:
            address = [self indexedAddressing:[ram peek:regPC]];
            regA = [self exclusiveor:regA :[ram peek:address]];
            break;

		case opcode_ADCA_indexed:
			address = [self indexedAddressing:[ram peek:regPC]];
			regA = [self addWithCarry:regA :[ram peek:address]];
			break;
			
		case opcode_ORA_indexed:
            address = [self indexedAddressing:[ram peek:regPC]];
            regA = [self orr:regA :[ram peek:address]];
            break;

		case opcode_ADDA_indexed:
			address = [self indexedAddressing:[ram peek:regPC]];
            regA = [self add:regA :[ram peek:address]];
			break;

		case opcode_CMPX_indexed:
			break;
			
		case opcode_JSR_indexed:
			break;
			
		case opcode_LDX_indexed:
			break;
			
		case opcode_STX_indexed:
			break;
			
		case opcode_SUBA_extended:
            i = ([ram peek:regPC] * 256) + [ram peek:regPC + 1];
            regA = [self sub:regA :[ram peek:i]];
            regPC = regPC + 2;
            break;
			
		case opcode_CMPA_extended:
            i = ([ram peek:regPC] * 256) + [ram peek:regPC + 1];
            [self compare:regA :[ram peek:i]];
            regPC = regPC + 2;
            break;
			
		case opcode_SBCA_extended:
            i = ([ram peek:regPC] * 256) + [ram peek:regPC + 1];
            regA = [self subWithCarry:regA :[ram peek:i]];
            regPC = regPC + 2;
            break;
			
		case opcode_SUBD_extended:
			break;
			
		case opcode_ANDA_extended:
            i = ([ram peek:regPC] * 256) + [ram peek:regPC + 1];
            regA = [self and:regA with:[ram peek:i]];
            regPC = regPC + 2;
            break;
			
		case opcode_BITA_extended:
            i = ([ram peek:regPC] * 256) + [ram peek:regPC + 1];
            [self bitTest:regA with:[ram peek:i]];
            regPC = regPC + 2;
            break;
			
		case opcode_LDA_extended:
            i = ([ram peek:regPC] * 256) + [ram peek:regPC + 1];
            [self load:[ram peek:i] :kRegA];
            regPC = regPC + 2;
			break;
			
		case opcode_STA_extended:
            i = ([ram peek:regPC] * 256) + [ram peek:regPC + 1];
            [ram poke:i :regA];
            [self store:regA];
            regPC = regPC + 2;
			break;
			
		case opcode_EORA_extended:
            i = ([ram peek:regPC] * 256) + [ram peek:regPC + 1];
            regA = [self exclusiveor:regA :[ram peek:i]];
            regPC = regPC + 2;
            break;
			
		case opcode_ADCA_extended:
            i = ([ram peek:regPC] * 256) + [ram peek:regPC + 1];
            regA = [self addWithCarry:regA :[ram peek:i]];
            regPC = regPC + 2;
            break;
			
		case opcode_ORA_extended:
            i = ([ram peek:regPC] * 256) + [ram peek:regPC + 1];
            regA = [self orr:regA :[ram peek:i]];
            regPC = regPC + 2;
            break;
			
		case opcode_ADDA_extended:
            i = ([ram peek:regPC] * 256) + [ram peek:regPC + 1];
            regA = [self add:regA :[ram peek:i]];
            regPC = regPC + 2;
            break;
			
		case opcode_CMPX_extended:
            i = ([ram peek:regPC] * 256) + [ram peek:regPC + 1];
            [self compare16bit:regX :([ram peek:i] * 256 + [ram peek:i + 1])];
            regPC = regPC + 2;
			break;
			
		case opcode_JSR_extended:
			break;
			
		case opcode_LDX_extended:
            i = ([ram peek:regPC] * 256) + [ram peek:regPC + 1];
            regX = [ram peek:i] * 256 + [ram peek:i + 1];
            [self load16bit:regX];
            regPC = regPC + 2;
            break;
			
		case opcode_STX_extended:
            i = ([ram peek:regPC] * 256) + [ram peek:regPC + 1];
            [ram poke:i :((regX >> 8) & 0xFF)];
            [ram poke:(i + 1) :(regX & 0xFF)];
            [self store16bit:regX];
            regPC = regPC + 2;
            break;

		case opcode_SUBB_immed:
			regB = [self sub:regB :[self loadFromRam]];
			break;

		case opcode_CMPB_immed:
			[self compare:regB :[self loadFromRam]];
			break;

		case opcode_SBCB_immed:
			regB = [self subWithCarry:regB :[ram peek:regPC]];
			regPC++;
			break;
			
		case opcode_ADDD_immed:
            // ADD 16-bit value to D
            i = [self addressFromNextTwoBytes];
            msb = (i >> 8) & 0xFF;
            lsb = i & 0xFF;
            regB = [self add:regB :lsb];
            regA = [self addWithCarry:regA :msb];
			break;

		case opcode_ANDB_immed:
			regB = [self and:regB with:[ram peek:regPC]];
			regPC++;
			break;

		case opcode_BITB_immed:
			[self bitTest:regB with:[ram peek:regPC]];
			regPC++;
			break;

		case opcode_LDB_immed:
			[self load:[ram peek:regPC] :kRegB];
			regPC++;
			break;

		case opcode_EORB_immed:
			regB = [self exclusiveor:regB :[ram peek:regPC]];
			regPC++;
			break;

		case opcode_ADCB_immed:
			regB = [self addWithCarry:regB :[ram peek:regPC]];
			regPC++;
			break;

		case opcode_ORB_immed:
			regB = [self orr:regB :[ram peek:regPC]];
			regPC++;
			break;

		case opcode_ADDB_immed:
			regB = [self add:regB :[ram peek:regPC]];
			regPC++;
			break;
			
		case opcode_LDD_immed:
			regA = [ram peek:regPC];
			regB = [ram peek:regPC + 1];
			[self load16bit:(regA * 256) + regB];
			regPC = regPC + 2;
			break;

		case opcode_LDU_immed:
			regUSP = [ram peek:regPC] * 256 + [ram peek:regPC + 1];
			[self load16bit:regUSP];
			regPC = regPC + 2;
			break;

		case opcode_SUBB_direct:
			i = [self addressFromDPR:0];
			regB = [self sub:regB :[ram peek:i]];
			regPC++;
			break;

		case opcode_CMPB_direct:
			i = [self addressFromDPR:0];
			[self compare:regB :[ram peek:i]];
			regPC++;
			break;

		case opcode_SBCB_direct:
			i = [self addressFromDPR:0];
			regB = [self subWithCarry:regB :[ram peek:i]];
			regPC++;
			break;
			
		case opcode_ADDD_direct:
			break;

		case opcode_ANDB_direct:
			i = [self addressFromDPR:0];
			regB = [self and:regB with:[ram peek:i]];
			regPC++;
			break;

		case opcode_BITB_direct:
			i = [self addressFromDPR:0];
			[self bitTest:regB with:[ram peek:i]];
			regPC++;
			break;

		case opcode_LDB_direct:
			i = [self addressFromDPR:0];
			[self load:[ram peek:i] :kRegB];
			regPC++;
			break;
			
		case opcode_STB_direct:
			i = [self addressFromDPR:0];
			[ram poke:i :regB];
			[self store:regB];
			regPC++;
			break;

		case opcode_EORB_direct:
            i = [self addressFromDPR:0];
            regB = [self exclusiveor:regB :[ram peek:i]];
            regPC++;
            break;

		case opcode_ADCB_direct:
			i = [self addressFromDPR:0];
			regB = [self addWithCarry:regB :[ram peek:i]];
			regPC++;
			break;

		case opcode_ORB_direct:
			i = [self addressFromDPR:0];
			regB = [self orr:regB :[ram peek:i]];
			regPC++;
			break;

		case opcode_ADDB_direct:
			i = [self addressFromDPR:0];
			regB = [self add:regB :[ram peek:i]];
			regPC++;
			break;

		case opcode_LDD_direct:
            i = [self addressFromDPR:0];
            regA = [ram peek:i];
            regB = [ram peek:i + 1];
            [self load16bit:((regA * 256) + regB)];
            regPC++;
            break;

		case opcode_STD_direct:
			i = [self addressFromDPR:0];
			[ram poke:i :regA];
			[ram poke:i + 1 :regB];
			[self store16bit:((regA * 256) + regB)];
			regPC++;
			break;

		case opcode_LDU_direct:
            i = [self addressFromDPR:0];
            regUSP = [ram peek:i] * 256 + [ram peek:i + 1];
            [self load16bit:regUSP];
            regPC++;
            break;

        case opcode_STU_direct:
            i = [self addressFromDPR:0];
            [ram poke:i :((regUSP >> 8) & 0xFF)];
            [ram poke:i + 1 :(regUSP & 0xFF)];
            [self store16bit:regUSP];
            regPC++;
            break;

        case opcode_SUBB_indexed:
            i = [self indexedAddressing:[ram peek:regPC]];
            [self sub:regB :[ram peek:i]];
            regPC++;
            break;

        case opcode_CMPB_indexed:
            i = [self indexedAddressing:[ram peek:regPC]];
            [self compare:regB :[ram peek:i]];
            regPC++;
            break;

        case opcode_SBCB_indexed:
            i = [self indexedAddressing:[ram peek:regPC]];
            [self subWithCarry:regB :[ram peek:i]];
            regPC++;
            break;

        case opcode_ADDD_indexed:
            i = [self indexedAddressing:[ram peek:regPC]];
            unsigned short regD = regA * 256 + regB;
            regD = [self add16bit:regD :([ram peek:i] * 256 + [ram peek:i + 1])];
            regB = regD & 0xFF;
            regA = (regD & 0xFF00) >> 8;
            regPC++;
            break;

        case opcode_ANDB_indexed:
            i = [self indexedAddressing:[ram peek:regPC]];
            regB = [self and:regB with:[ram peek:i]];
            regPC++;
            break;

        case opcode_BITB_indexed:
            i = [self indexedAddressing:[ram peek:regPC]];
            [self bitTest:regB with:[ram peek:i]];
            regPC++;
            break;

        case opcode_LDB_indexed:
            i = [self indexedAddressing:[ram peek:regPC]];
            [self load:[ram peek:i] :0x09];
            regPC++;
            break;

        case opcode_STB_indexed:
            i = [self indexedAddressing:[ram peek:regPC]];
            [ram poke:i :regB];
            [self store:[ram peek:i]];
            regPC++;
            break;

        case opcode_EORB_indexed:
            i = [self indexedAddressing:[ram peek:regPC]];
            regB = [self exclusiveor:[ram peek:i] :regB];
            regPC++;
            break;

        case opcode_ADCB_indexed:
            i = [self indexedAddressing:[ram peek:regPC]];
            regB = [self addWithCarry:[ram peek:i] :regB];
            regPC++;
            break;

        case opcode_ORB_indexed:
            i = [self indexedAddressing:[ram peek:regPC]];
            regB = [self orr:[ram peek:i] :regB];
            regPC++;
            break;

        case opcode_ADDB_indexed:
            i = [self indexedAddressing:[ram peek:regPC]];
            regB = [self add:[ram peek:i] :regB];
            regPC++;
            break;

        case opcode_LDD_indexed:
            i = [self indexedAddressing:[ram peek:regPC]];
            regA = [ram peek:i];
            regB = [ram peek:i + 1];
            [self load16bit:((regA * 256) + regB)];
            regPC++;
            break;

        case opcode_STD_indexed:
            i = [self indexedAddressing:[ram peek:regPC]];
            [ram poke:i :regA];
            [ram poke:i + 1 :regB];
            [self store16bit:((regA * 256) + regB)];
            regPC++;
            break;

        case opcode_LDU_indexed:
            i = [self indexedAddressing:[ram peek:regPC]];
            regUSP = [ram peek:i];
            [self load16bit:regUSP];
            regPC++;
            break;

        case opcode_STU_indexed:
            i = [self indexedAddressing:[ram peek:regPC]];
            [ram poke:i :(regUSP & 0xFF00)];
            [ram poke:i + 1 :(regUSP & 0xFF)];
            [self store16bit:regUSP];
            regPC++;
            break;
            
        case opcode_SUBB_extended:
            i = [ram peek:regPC] * 256 + [ram peek:regPC + 1];
            [self sub:regB :[ram peek:i]];
            regPC = regPC + 2;
            break;

        case opcode_CMPB_extended:
            i = [ram peek:regPC] * 256 + [ram peek:regPC + 1];
            [self compare:regB :[ram peek:i]];
            regPC = regPC + 2;
            break;

        case opcode_SBCB_extended:
            i = [ram peek:regPC] * 256 + [ram peek:regPC + 1];
            [self subWithCarry:regB :[ram peek:i]];
            regPC = regPC + 2;
            break;

        case opcode_ADDD_extended:
            i = [ram peek:regPC] * 256 + [ram peek:regPC + 1];
            unsigned short rD = regA * 256 + regB;
            rD = [self add16bit:rD :([ram peek:i] * 256 + [ram peek:i + 1])];
            regB = rD & 0xFF;
            regA = (rD & 0xFF00) >> 8;
            regPC = regPC + 2;
            break;

        case opcode_ANDB_extended:
            i = [ram peek:regPC] * 256 + [ram peek:regPC + 1];
            regB = [self and:regB with:[ram peek:i]];
            regPC = regPC + 2;
            break;

        case opcode_BITB_extended:
            i = [ram peek:regPC] * 256 + [ram peek:regPC + 1];
            [self bitTest:regB with:[ram peek:i]];
            regPC = regPC + 2;
            break;

        case opcode_LDB_extended:
            i = [ram peek:regPC] * 256 + [ram peek:regPC + 1];
            [self load:[ram peek:i] :0x09];
            regPC = regPC + 2;
            break;

        case opcode_STB_extended:
            i = [ram peek:regPC] * 256 + [ram peek:regPC + 1];
            [ram poke:i :regB];
            [self store:[ram peek:i]];
            regPC = regPC + 2;
            break;

        case opcode_EORB_extended:
            i = [ram peek:regPC] * 256 + [ram peek:regPC + 1];
            regB = [self exclusiveor:[ram peek:i] :regB];
            regPC = regPC + 2;
            break;

        case opcode_ADCB_extended:
            i = [ram peek:regPC] * 256 + [ram peek:regPC + 1];
            regB = [self addWithCarry:[ram peek:i] :regB];
            regPC = regPC + 2;
            break;

        case opcode_ORB_extended:
            i = [ram peek:regPC] * 256 + [ram peek:regPC + 1];
            regB = [self orr:[ram peek:i] :regB];
            regPC = regPC + 2;
            break;

        case opcode_ADDB_extended:
            i = [ram peek:regPC] * 256 + [ram peek:regPC + 1];
            regB = [self add:[ram peek:i] :regB];
            regPC = regPC + 2;
            break;

        case opcode_LDD_extended:
            i = [ram peek:regPC] * 256 + [ram peek:regPC + 1];
            regA = [ram peek:i];
            regB = [ram peek:i + 1];
            [self load16bit:((regA * 256) + regB)];
            regPC = regPC + 2;
            break;

        case opcode_STD_extended:
            i = [ram peek:regPC] * 256 + [ram peek:regPC + 1];
            [ram poke:i :regA];
            [ram poke:i + 1 :regB];
            [self store16bit:((regA * 256) + regB)];
            regPC = regPC + 2;
            break;

        case opcode_LDU_extended:
            i = [ram peek:regPC] * 256 + [ram peek:regPC + 1];
            regUSP = [ram peek:i];
            [self load16bit:regUSP];
            regPC = regPC + 2;
            break;

        case opcode_STU_extended:
            i = [ram peek:regPC] * 256 + [ram peek:regPC + 1];
            [ram poke:i :(regUSP & 0xFF00)];
            [ram poke:i + 1 :(regUSP & 0xFF)];
            [self store16bit:regUSP];
            regPC = regPC + 2;
            break;

        // First Extended Instructions Begin Here
			
		case opcode_LBRN_rel:
            regPC += 2;
			break;
			
		case opcode_LBHI_rel:
            if (![self bitSet:regCC :kCC_c] && ![self bitSet:regCC :kCC_z])
                regPC += (short)([ram peek:regPC] * 256 + [ram peek:(regPC + 1)]);
			break;
			
		case opcode_LBLS_rel:
            if ([self bitSet:regCC :kCC_c] || [self bitSet:regCC :kCC_z])
                regPC += (short)([ram peek:regPC] * 256 + [ram peek:(regPC + 1)]);
			break;
			
		case opcode_LBHS_rel:
            if (![self bitSet:regCC :kCC_c]) regPC += (short)([ram peek:regPC] * 256 + [ram peek:(regPC + 1)]);
			break;
			
		case opcode_LBLO_rel:
            if ([self bitSet:regCC :kCC_c]) regPC +=+ (short)([ram peek:regPC] * 256 + [ram peek:(regPC + 1)]);
			break;
			
		case opcode_LBNE_rel:
            if (![self bitSet:regCC :kCC_z]) regPC += (short)([ram peek:regPC] * 256 + [ram peek:(regPC + 1)]);
			break;
			
		case opcode_LBEQ_rel:
            if ([self bitSet:regCC :kCC_z]) regPC += (short)([ram peek:regPC] * 256 + [ram peek:(regPC + 1)]);
			break;
			
		case opcode_LBVC_rel:
            if (![self bitSet:regCC :kCC_v]) regPC += (short)([ram peek:regPC] * 256 + [ram peek:(regPC + 1)]);
            break;
			
		case opcode_LBVS_rel:
            if ([self bitSet:regCC :kCC_v]) regPC += (short)([ram peek:regPC] * 256 + [ram peek:(regPC + 1)]);
			break;
			
		case opcode_LBPL_rel:
            if (![self bitSet:regCC :kCC_n]) regPC += (short)([ram peek:regPC] * 256 + [ram peek:(regPC + 1)]);
			break;
			
		case opcode_LBMI_rel:
            if ([self bitSet:regCC :kCC_n]) regPC += (short)([ram peek:regPC] * 256 + [ram peek:(regPC + 1)]);
			break;
			
		case opcode_LBGE_rel:
            if ([self bitSet:regCC :kCC_n] == [self bitSet:regCC :kCC_v])
                regPC += (short)([ram peek:regPC] * 256 + [ram peek:(regPC + 1)]);
			break;
			
		case opcode_LBLT_rel:
            if (([self bitSet:regCC :kCC_n] || [self bitSet:regCC :kCC_v]) && ([self bitSet:regCC :kCC_c] != [self bitSet:regCC :kCC_v]))
                regPC += (short)([ram peek:regPC] * 256 + [ram peek:(regPC + 1)]);
			break;
			
		case opcode_LBGT_rel:
            if (![self bitSet:regCC :kCC_z] && [self bitSet:regCC :kCC_n] == [self bitSet:regCC :kCC_v])
                regPC += (short)([ram peek:regPC] * 256 + [ram peek:(regPC + 1)]);
			break;
			
		case opcode_LBLE_rel:
            if ([self bitSet:regCC :kCC_z])
            {
                regPC += (short)([ram peek:regPC] * 256 + [ram peek:(regPC + 1)]);
            }
            else if (([self bitSet:regCC :kCC_n] || [self bitSet:regCC :kCC_v]) && ([self bitSet:regCC :kCC_n] != [self bitSet:regCC :kCC_v]))
            {
                regPC += (short)([ram peek:regPC] * 256 + [ram peek:(regPC + 1)]);
            }
            break;
			
		case opcode_SWI2:
            // SoftWare Interrupt 2
            // Set e to 1 then push every register to the hardware stack
            
            regCC = [self setBit:regCC :kCC_e];
            [self push:YES :kPushPullRegEvery];
            waitForInterruptFlag = NO;
            
            // Set PC to the interrupt vector
            regPC = [ram peek:kInterruptVectorTwo] * 256 + [ram peek:kInterruptVectorTwo + 1];
            break;
			
		case opcode_CMPD_immed:
            [self compare16bit:(regA * 256 + regB) :([ram peek:regPC] * 256 + [ram peek:(regPC + 1)])];
            regPC += 2;
			break;
			
		case opcode_CMPY_immed:
			[self compare16bit:regY :([ram peek:regPC] * 256 + [ram peek:(regPC + 1)])];
            regPC += 2;
            break;
			
		case opcode_LDY_immed:
			regY = [ram peek:regPC] * 256 + [ram peek:regPC + 1];
			regPC += 2;
			[self load16bit:regY];
			break;
			
		case opcode_CMPD_direct:
            i = [self addressFromDPR:0];
            [self compare16bit:(regA * 256 + regB) :([ram peek:i] * 256 + [ram peek:(i + 1)])];
            regPC += 2;
			break;
			
		case opcode_CMPY_direct:
            i = [self addressFromDPR:0];
            [self compare16bit:regY :([ram peek:i] * 256 + [ram peek:(i + 1)])];
            regPC += 2;
            break;
			
		case opcode_LDY_direct:
			i = [self addressFromDPR:0];
			regY = [ram peek:i] * 256 + [ram peek:i + 1];
			[self load16bit:regY];
			regPC++;
			break;
			
		case opcode_STY_direct:
            i = [self addressFromDPR:0];
            [self store16bit:regY];
            [ram poke:i :((regY >> 8) & 0xFF)];
            [ram poke:(i + 1) :(regY & 0xFF)];
            regPC++;
            break;
			
		case opcode_CMPD_indexed:
			break;
			
		case opcode_CMPY_indexed:
			break;
			
		case opcode_LDY_indexed:
			break;
			
		case opcode_STY_indexed:
			break;
			
		case opcode_CMPD_extended:
			break;
			
		case opcode_CMPY_extended:
			break;
			
		case opcode_LDY_extended:
			break;
			
		case opcode_STY_extended:
			break;
			
		case opcode_LDS_immed:
            msb = [self loadFromRam];
            lsb = [self loadFromRam];
            regHSP = (msb << 8) + lsb;
            [self load16bit:regHSP];
            break;
			
		case opcode_LDS_direct:
            operand = [self addressFromDPR:0];
            regHSP = [ram peek:operand] * 256 + [ram peek:operand + 1];
            [self load16bit:regHSP];
            [self incrementPC:1];
            break;
			
		case opcode_STS_direct:
            operand = [self addressFromDPR:0];
            [self store16bit:regHSP];
            [ram poke:operand :((regHSP >> 8) & 0xFF)];
            [ram poke:(operand + 1) :(regHSP & 0xFF)];
            [self incrementPC:1];
            break;
			
		case opcode_LDS_indexed:
			break;
			
		case opcode_STS_indexed:
			break;
			
		case opcode_LDS_extended:
			break;
			
		case opcode_STS_extended:
			break;
			
		// Second Extended Instructions Begin Here
			
		case opcode_SWI3:
            // SoftWare Interrupt 3
            // Set e to 1 then push every register to the hardware stack
            
            regCC = [self setBit:regCC :kCC_e];
            [self push:YES :kPushPullRegEvery];
            waitForInterruptFlag = NO;
            
            // Set the PC to the interrupt vector
            regPC = [ram peek:kInterruptVectorThree] * 256 + [ram peek:kInterruptVectorThree + 1];
            break;
			
		case opcode_CMPU_immed:
            [self compare16bit:regUSP :([ram peek:regPC] * 256 + [ram peek:(regPC + 1)])];
            regPC += 2;
            break;
			
		case opcode_CMPS_immed:
            [self compare16bit:regHSP :([ram peek:regPC] * 256 + [ram peek:(regPC + 1)])];
            regPC += 2;
            break;
			
		case opcode_CMPU_direct:
            i = [self addressFromDPR:0];
            [self compare16bit:regUSP :([ram peek:i] * 256 + [ram peek:(i + 1)])];
            regPC += 2;
            break;
			
		case opcode_CMPS_direct:
            i = [self addressFromDPR:0];
            [self compare16bit:regHSP :([ram peek:i] * 256 + [ram peek:(i + 1)])];
            regPC += 2;
            break;
			
		case opcode_CMPU_indexed:
            [self compare16bit:regUSP :[self indexedAddressing:[ram peek:regPC]]];
            regPC++;
            break;
			
		case opcode_CMPS_indexed:
            [self compare16bit:regHSP :[self indexedAddressing:[ram peek:regPC]]];
            regPC++;
            break;
			
		case opcode_CMPU_extended:
			break;
			
		case opcode_CMPS_extended:
			break;
			
		default:
			break;
    }

    if (regPC > 65535) regPC = regPC - 65536;
}



# pragma mark - Operation Helper Methods


- (NSUInteger)add:(NSUInteger)value :(NSUInteger)amount
{
	// Adds two 8-bit values
	// Affects z, n, h, v, c - 'alu:' sets h, v, c

    [self clrCCN];
    [self clrCCZ];

	NSUInteger answer = [self alu:value :amount :NO];
	if (answer == 0) [self setCCZ];
	if ([self bitSet:answer :kSignBit]) [self setCCN];
	return answer;
}



- (NSUInteger)add16bit:(NSUInteger)value :(NSUInteger)amount
{
    // Adds two 16-bit values
    // Affects z, n, h, v, c - 'alu:' sets h, v, c

    [self clrCCN];
    [self clrCCZ];

    NSUInteger lsb = [self alu:(value & 0xFF) :(amount & 0xFF) :NO];
    NSUInteger msb = [self alu:((value >> 8) & 0xFF) :((amount >> 8) & 0xFF) :YES];
    NSUInteger answer = (msb * 0xFF) + lsb;
    if (answer == 0) [self setCCZ];
    if ([self bitSet:answer :15]) [self setCCN];
    return answer;
}



- (NSUInteger)addWithCarry:(NSUInteger)value :(NSUInteger)amount
{
	// Adds two 8-bit values plus CCR c
    // Affects z, n, h, v, c - 'alu:' sets h, v, c

	[self clrCCN];
	[self clrCCZ];

    NSUInteger answer = [self alu:value :amount :YES];
    if (answer == 0) [self setCCZ];
    if ([self bitSet:answer :kSignBit]) [self setCCN];
    return answer;
}



- (NSUInteger)and:(NSUInteger)value with:(NSUInteger)amount
{
    // ANDs the two supplied values
    // Affects n, z, v - v is always 0

    [self clrCCN];
    [self clrCCZ];
    [self clrCCV];

    NSUInteger answer = value & amount;
    if (answer == 0) [self setCCZ];
    if ([self bitSet:answer :kSignBit]) [self setCCN];
    return (answer & 0xFF);
}



- (void)andcc:(NSUInteger)value
{
    // ANDs the CC register
    // Affects all bits

    regCC = regCC & value;
}



- (NSUInteger)aShiftLeft:(NSUInteger)value
{
    // Arithmetic shift left - equivalent to logic shift left
    // See 'lShiftLeft:'

    return [self lShiftLeft:value];
}



- (NSUInteger)aShiftRight:(NSUInteger)value
{
    // Arithmetic shift right
    // Affects n, z, c
    
    [self clrCCN];
    [self clrCCZ];
    [self clrCCC];
    [self decimalToBits:value];
    
    if (bit[0] == 1) [self setCCC];
    
    bit[0] = bit[1];
    bit[1] = bit[2];
    bit[2] = bit[3];
    bit[3] = bit[4];
    bit[4] = bit[5];
    bit[5] = bit[6];
    bit[6] = bit[7];
    
    value = [self bitsToDecimal];
    if (value == 0) [self setCCZ];
    if ([self bitSet:value :kSignBit]) [self setCCN];
    return value;
}



- (void)bitTest:(NSUInteger)value with:(NSUInteger)amount
{
	// Does not affect the operands, only the CC register, so ignore the result
    // CC register set in 'and:'
    
    [self and:value with:amount];
}



- (void)clear
{
    // Sets n, z, v, c generically for any clear operation, eg. CLRA, CLRB, CLR 0x0000
    // Values are fixed
    
    [self clrCCN];
    [self setCCZ];
    [self clrCCV];
    [self clrCCC];
}



- (void)compare:(NSUInteger)value :(NSUInteger)amount
{
    // Compare two values by subtracting the second from the first. Result is discarded
    // Affects n, z, v, c - c, v set by 'alu:' via 'sub:'
    
    [self clrCCN];
    [self clrCCZ];
    [self clrCCV];
    [self clrCCC];

    NSUInteger answer = [self sub:value :amount];
    if (answer == 0) [self setCCZ];
    if ([self bitSet:answer :kSignBit]) [self setCCN];
}



- (void)compare16bit:(NSUInteger)value :(NSUInteger)amount
{
    // TODO
}



- (NSUInteger)complement:(NSUInteger)value
{
    // One's complement the passed value
    // Affects n, z, v, c - v and c take fixed values (0, 1)
    
    [self decimalToBits:value];
    
    for (NSInteger i = 0 ; i < 8 ; i++) bit[i] = bit[i] == 1 ? 0 : 1;
    
    value = [self bitsToDecimal];

    [self clrCCN];
    [self clrCCZ];
    [self clrCCV];
    [self setCCC];

    if (value == 0) [self setCCZ];
    if ([self bitSet:value :kSignBit]) [self setCCN];
    return value;
}



- (void)decimalAdjustA
{
    unsigned char lsnb, msnb, carry, correction;
    
    carry = [self bitSet:regCC :kCC_c];
    [self decimalToBits:regA];
    
    // Calculate decimal values of Most Significant Nibble and Least Significant Nibble
    
    lsnb = bit[0] + (0x02 * bit[1]) + (0x04 * bit[2]) + (0x08 * bit[3]);
    msnb = (0x10 * bit[4]) + (0x20 * bit[5]) + (0x40 * bit[6]) + (0x80 * bit[7]);
    
    // Set Least Significant Conversion Factor, which will be either 0 or 6
    
    correction = 0;
    if ([self bitSet:regCC :kCC_h] || lsnb > 9) correction = 6;
    
    // Set Most Significant Conversion Factor, which will be either 0 or 6
   
    if ([self bitSet:regCC :kCC_c] || msnb > 9 || (msnb > 8 && lsnb > 9)) correction = 6;

    [self clrCCN];
    [self clrCCZ];
    [self clrCCC];

    regA += correction;

    if (regA > 0xFF)
    {
        regA = regA & 0xFF;
        [self setCCC];
    }

    if (carry) [self setCCC];
    if (regA == 0) [self setCCZ];
    if ([self bitSet:regA :kSignBit]) [self setCCN];
}



- (NSUInteger)decrement:(NSUInteger)value
{
    // Subtract 1 from the operand
    // Affects n, z, v - v set only if original operand was $80, cleared otherwise
    
    [self clrCCN];
    [self clrCCZ];
    [self clrCCV];

    if (value == 0x80) [self setCCV];     // See 'Programming the 6809' p 155
    
    NSInteger answer = value - 1;
    if (answer == 0) [self setCCZ];
    if (answer < 0)
    {
        [self setCCN];
        answer = (256 - answer) & 0xFF;
    }

    return (NSUInteger)answer;
}



- (NSUInteger)increment:(NSUInteger)value
{
    // Add 1 to the operand
    // Affects n, z, v
    
    [self clrCCN];
    [self clrCCZ];
    [self clrCCV];
    
    if (value == 0x7F) [self setCCV];     // See 'Programming the 6809' p 158
    
    NSUInteger answer = (value + 1) & 0xFF;
    if (answer == 0) [self setCCZ];
    if ([self bitSet:answer :kSignBit]) [self setCCN];
    return answer;
}



- (NSUInteger)exclusiveor:(NSUInteger)value :(NSUInteger)amount
{
	// Perform an exclusive OR
    // Affects n, z ,v - v always cleared
    
    [self clrCCN];
    [self clrCCZ];
    [self clrCCV];
    
    NSUInteger answer = value ^ amount;
	if (answer == 0) [self setCCZ];
	if ([self bitSet:answer :kSignBit]) [self setCCN];
	return answer;
}



- (NSUInteger)exchange:(NSUInteger)value :(NSUInteger)regCode
{
    // Returns the value *from* the register identified by regCode
    // after placing the passed value into that register
    
    NSUInteger returnValue = 0x00;
    
    switch (regCode)    // This is the DESTINATION register
    {
        case 0x08:
            // A
            returnValue = regA;
            regA = value;
            break;
            
        case 0x09:
            // B
            returnValue = regB;
            regB = value;
            break;
            
        case 0x0A:
            // CC
            returnValue = regCC;
            regCC = value;
            break;
            
        case 0x0B:
            // DPR
            returnValue = regDP;
            regDP = value;
            break;
            
        default:
            // Incorrect Register - signal error
            // [self halt:@"Incorrect register in 8-bit swap"];
            returnValue = 0xFF;
    }
    
    return returnValue;
}



- (NSUInteger)exchange16bit:(NSUInteger)value :(NSUInteger)regCode
{
    // Returns the value *from* the register identified by regCode
    // after placing the passed value into that register
    
    NSUInteger returnValue = 0x0000;
    
    switch (regCode)  // This is the *destination* register
    {
        case 0x00:
            // D
            returnValue = (regA * 256) + regB;
            regA = (value >> 8) & 0xFF;
            regB = value & 0xFF;
            break;
            
        case 0x01:
            // X
            returnValue = regX;
            regX = value;
            break;
            
        case 0x02:
            // Y
            returnValue = regY;
            regY = value;
            break;
            
        case 0x03:
            // U
            returnValue = regUSP;
            regUSP = value;
            break;
            
        case 0x04:
            // S
            returnValue = regHSP;
            regHSP = value;
            break;
            
        case 0x05:
            // PC
            returnValue = regPC;
            regPC = value;
            break;
            
        default:
            // Incorrect Register - signal error
            // [self halt:@"Incorrect register in 8-bit swap"];
            returnValue = 0xFFFF;
    }
    
    return returnValue;
}



- (void)load:(NSUInteger)value :(NSUInteger)regCode
{
	// Put passed value into an 8-bit register
    // Affects n, z, v - v is always cleared
    
    [self clrCCN];
    [self clrCCZ];
    [self clrCCV];
    
    switch (regCode)
    {
        case 0x08:
            // A
            regA = value;
            break;
            
        case 0x09:
            // B
            regB = value;
            break;
    }

    if (value == 0) [self setCCZ];
	if ([self bitSet:value :kSignBit]) [self setCCN];
}
	


- (void)load16bit:(NSUInteger)value
{
    // Sets CC after a 16-bit load
    // Affects n, z, v - v is always cleared
    
    [self clrCCN];
    [self clrCCZ];
    [self clrCCV];
    
    if (value == 0) [self setCCZ];
	if ([self bitSet:value :15]) [self setCCN];
}



- (void)loadEffective:(NSUInteger)amount :(NSUInteger)regCode
{
    // Put passed value into an 8-bit register
    // Affects z, but only for X and Y registers
    
    switch (regCode)
    {
        case 0x01:
            // X
            regX = amount;
            if (amount == 0) [self setCCZ];
            break;
            
        case 0x02:
            // Y
            regY = amount;
            if (amount == 0) [self setCCZ];
            break;
            
        case 0x03:
            // U
            regUSP = amount;
            break;
            
        case 0x04:
            // S
            regHSP = amount;
            break;
    }
}



- (NSUInteger)lShiftRight:(NSUInteger)value
{
    // Logical shift right
    // Affects n, z, v - n is alwasy cleared
    
    [self clrCCN];
    [self clrCCZ];
    [self clrCCV];
    
    [self decimalToBits:value];
    if (bit[0] == 1) [self setCCC];
    
    bit[0] = bit[1];
    bit[1] = bit[2];
    bit[2] = bit[3];
    bit[3] = bit[4];
    bit[4] = bit[5];
    bit[5] = bit[6];
    bit[6] = bit[7];
    bit[7] = 0;
    
    value = [self bitsToDecimal];
    if (value == 0) [self setCCZ];
    return value;
}



- (NSUInteger)lShiftLeft:(NSUInteger)value
{
    // Logical shift left
    // Affects n, z, v, c
    
    [self clrCCN];
    [self clrCCZ];
    [self clrCCV];
    [self clrCCC];
    
    [self decimalToBits:value];
    
    if (bit[7] == 1) [self setCCC];
    if (bit[7] != bit[6]) [self setCCV];
    
    bit[7] = bit[6];
    bit[6] = bit[5];
    bit[5] = bit[4];
    bit[4] = bit[3];
    bit[3] = bit[2];
    bit[2] = bit[1];
    bit[1] = bit[0];
    bit[0] = 0;
    
    value = [self bitsToDecimal];
    if (value == 0) [self setCCZ];
    if ([self bitSet:value :kSignBit]) [self setCCN];
    return value;
}



- (void)multiply
{
    // Multiple A and B and put the result in D (ie. A and B)
    // Affects z, c
    
    [self clrCCZ];
    [self clrCCC];
    
    NSUInteger d = regA * regB;
    if ([self bitSet:d :7]) [self setCCC];
    if (d == 0) [self setCCZ];
    
    regA = (d >> 8) & 0xFF;
    regB = d & 0xFF;
}



- (NSUInteger)negate:(NSUInteger)value
{
    // Returns 2's complement of 8-bit value
    // Affects CCR z, n, v ('alu:' sets CCR c, v, h)
    // If value is 0x80, v is set

    [self decimalToBits:value];
    
    // Flip value's bits to make the 1s complenent
    
    for (NSUInteger i = 0 ; i < 8 ; i++) bit[i] = bit[i] == 1 ? 0 : 1;
    
    // Add 1 to the bits to get the 2s complement
    
    NSUInteger answer = [self alu:[self bitsToDecimal] :1 :NO];
    
    [self clrCCN];
    if ([self bitSet:answer :kSignBit]) [self setCCN];
    
    [self clrCCZ];
    if (answer == 0) [self setCCZ];
	
	[self clrCCV];
	if (value == 0x80) [self setCCV];

	return answer;
}



- (NSUInteger)orr:(NSUInteger)value :(NSUInteger)amount
{
    // OR
    // Affects n, z, v - v is always cleared
    
    [self clrCCN];
    [self clrCCZ];
    [self clrCCV];
    
    NSUInteger answer = value | amount;
    if (answer == 0) [self setCCZ];
    if ([self bitSet:answer :kSignBit]) [self setCCN];
    return answer;
}



- (void)orcc:(NSUInteger)value
{
    // OR the contents of CC with the supplied 8-bit value
    // Affects all CC bits
    
    regCC = (regCC | value) & 0xFF;
}



- (void)push:(BOOL)toHardwareStack :(NSUInteger)postbyte
{
    // Push the specified registers (in 'postByte') to the hardware or user stack
    // 'toHardwareStack' should be YES for S, NO for U
    // See 'Programming the 6809' p.171-2
    
    NSUInteger source = regHSP;
    NSUInteger dest = regUSP;
    
    if (toHardwareStack)
    {
        source = regUSP;
        dest = regHSP;
    }
    
    [self decimalToBits:postbyte];
    
    if (bit[7] == 1)
    {
        // Push PC
        dest--;
        if (dest == -1) dest = 0xFFFF;
        [ram poke:dest :regPC & 0xFF];
        dest--;
        if (dest == -1) dest = 0xFFFF;
        [ram poke:dest :(regPC >> 8) & 0xFF];
    }
    
    if (bit[6] == 1)
    {
        // Push U/S
        dest--;
        if (dest == -1) dest = 0xFFFF;
        [ram poke:dest :source & 0xFF];
        dest--;
        if (dest == -1) dest = 0xFFFF;
        [ram poke:dest :(source >> 8) & 0xFF];
    }
    
    if (bit[5] == 1)
    {
        // Push Y
        dest--;
        if (dest == -1) dest = 0xFFFF;
        [ram poke:dest :regY & 0xFF];
        dest--;
        if (dest == -1) dest = 0xFFFF;
        [ram poke:dest :(regY >> 8) & 0xFF];
    }
    
    if (bit[4] == 1)
    {
        // Push X
        dest--;
        if (dest == -1) dest = 0xFFFF;
        [ram poke:dest :regX & 0xFF];
        dest--;
        if (dest == -1) dest = 0xFFFF;
        [ram poke:dest :(regX >> 8) & 0xFF];
    }
    
    if (bit[3] == 1)
    {
        // Push DP
        dest--;
        if (dest == -1) dest = 0xFFFF;
        [ram poke:dest :regDP];
    }
    
    if (bit[2] == 1)
    {
        // Push B
        dest--;
        if (dest == -1) dest = 0xFFFF;
        [ram poke:dest :regB];
    }
    
    if (bit[1] == 1)
    {
        // Push A
        dest--;
        if (dest == -1) dest = 0xFFFF;
        [ram poke:dest :regA];
    }
    
    if (bit[0] == 1)
    {
        // Push CC
        dest--;
        if (dest == -1) dest = 0xFFFF;
        [ram poke:dest :regCC];
    }
    
    if (toHardwareStack)
    {
        regHSP = dest;
    }
    else
    {
        regUSP = dest;
    }
}



- (void)pull:(BOOL)fromHardwareStack :(NSUInteger)postbyte
{
    // Pull the specified registers (in 'postByte') from the hardware or user stack
    // 'fromHardwareStack' should be YES for S, NO for U
    // See 'Programming the 6809' p.173-4
    
    NSUInteger source = regUSP;
    NSUInteger dest = regHSP;
    
    if (fromHardwareStack)
    {
        source = regHSP;
        dest = regUSP;
    }
    
    [self decimalToBits:postbyte];
    
    if (bit[0] == 1)
    {
        // Pull CC
		regCC = [ram peek:source];
        source++;
        if (source > 0xFFFF) source = 0x00;
    }
    
    if (bit[1] == 1)
    {
        // Pull A
        regA = [ram peek:source];
        source++;
        if (source > 0xFFFF) source = 0x00;
    }
    
    if (bit[2] == 1)
    {
        // Pull B
        regB = [ram peek:source];
        source++;
        if (source > 0xFFFF) source = 0x00;
    }
    
    if (bit[3] == 1)
    {
        // Pull DP
        regDP = [ram peek:source];
        source++;
        if (source > 0xFFFF) source = 0x00;
    }
    
    if (bit[4] == 1)
    {
        // Pull X
        regX = [ram peek:source] * 256;
        source++;
        if (source > 0xFFFF) source = 0x00;
        regX = regX + [ram peek:source];
        source++;
        if (source > 0xFFFF) source = 0x00;
    }
    
    if (bit[5] == 1)
    {
        // Pull Y
        regY = [ram peek:source] * 256;
        source++;
        if (source > 0xFFFF) source = 0x00;
        regY = regY + [ram peek:source];
        source++;
        if (source > 0xFFFF) source = 0x00;
    }
    
    if (bit[6] == 1)
    {
        // Pull S or U
        dest = [ram peek:source] * 256;
        source++;
        if (source > 0xFFFF) source = 0x00;
        dest = dest + [ram peek:source];
        source++;
        if (source > 0xFFFF) source = 0x00;
    }
    
    if (bit[7] == 1)
    {
        // Pull PC
        regPC = [ram peek:source] * 256;
        source++;
        if (source > 0xFFFF) source = 0x00;
        regPC = regPC + [ram peek:source];
        source++;
        if (source > 0xFFFF) source = 0x00;
    }
    
    if (fromHardwareStack)
    {
        regHSP = source;
        regUSP = dest;
    }
    else
    {
        regUSP = source;
        regHSP = dest;
    }
}



- (NSUInteger)rotateLeft:(NSUInteger)value
{
    // Rotate left
    // Affects n, z, v, c
    
    NSUInteger carry = [self bitSet:regCC :kCC_c] ? 1 : 0;
    
    [self decimalToBits:value];
    
    // c becomes bit 7 of original operand
    [self clrCCC];
    if (bit[7] == 1) [self setCCC];
    
    // n is bit 7 XOR bit 6 of value
    [self clrCCV];
    if (bit[7] != bit[6]) [self setCCV];
    
    bit[7] = bit[6];
    bit[6] = bit[5];
    bit[5] = bit[4];
    bit[4] = bit[3];
    bit[3] = bit[2];
    bit[2] = bit[1];
    bit[1] = bit[0];
    bit[0] = carry;
    
    [self clrCCN];
    if (bit[7] == 1) [self setCCN];
    
    [self clrCCZ];
    value = [self bitsToDecimal];
    if (value == 0) [self setCCZ];
    return value;
}



- (NSUInteger)rotateRight:(NSUInteger)value
{
	// Rotate right
	// Affects n, z, c
	
    NSUInteger carry = [self bitSet:regCC :kCC_c] ? 1 : 0;
    
    [self decimalToBits:value];
	
    // c is bit 0 of original operand
    [self clrCCC];
    if (bit[0] == 1) [self setCCC];
    
    bit[0] = bit[1];
    bit[1] = bit[2];
    bit[2] = bit[3];
    bit[3] = bit[4];
    bit[4] = bit[5];
    bit[5] = bit[6];
    bit[6] = bit[7];
    bit[7] = carry;
    
    [self clrCCN];
    if (bit[7] == 1) [self setCCN];
    
    [self clrCCZ];
    value = [self bitsToDecimal];
    if (value == 0) [self setCCZ];
    return value;   
}



- (void)store:(NSUInteger)value
{
	// Sets CC after an 8-bit store
	// Affects n, z, v - v is always cleared
	
    [self clrCCN];
    [self clrCCZ];
    [self clrCCV];
    
	if (value == 0) [self setCCZ];
	if ([self bitSet:value :kSignBit]) [self setCCN];
}



- (void)store16bit:(NSUInteger)value
{
	// Sets CC after a 16-bit store
	// Affects n, z, v - v is always cleared
	
    [self clrCCN];
    [self clrCCZ];
    [self clrCCV];
	
	if (value == 0) [self setCCZ];
	if ([self bitSet:value :15]) [self setCCN];
}



- (NSUInteger)sub:(NSUInteger)value :(NSUInteger)amount
{
    // Subtract 'amount' from 'value' by adding 'value' to -'amount'
    // Affects n, z, v, c - v, c are set by 'alu:'

    [self clrCCN];
    [self clrCCV];
    [self clrCCZ];
	
	NSUInteger comp = [self negate:amount];
	NSUInteger answer = [self alu:value :comp :NO];
	
	if (answer == 0) [self setCCZ];
	if ([self bitSet:value :kSignBit]) [self setCCN];
	
    // c represents a borrow and is set to the complement of the carry
    // of the internal binary addition
    
    if ([self bitSet:regCC :kCC_c])
	{
        [self clrCCC];
	}
	else
	{
		[self setCCC];
	}

	return answer;
}


- (NSUInteger)subWithCarry:(NSUInteger)value :(NSUInteger)amount
{
    // Subtract with Carry (borrow)
    // Affects n, z, v, c - v and c set by 'alu:'
    
    [self clrCCN];
    [self clrCCZ];
    [self clrCCV];
    
	NSUInteger comp = [self negate:amount];
	NSUInteger answer = [self alu:value :comp :YES];
	
	if (answer == 0) [self setCCZ];
	if ([self bitSet:answer :kSignBit]) [self setCCN];
	
	if ([self bitSet:regCC :kCC_c])
	{
		[self clrCCC];
	}
	else
	{
		[self setCCC];
	}

    return answer;
}



- (NSUInteger)sub16bit:(NSUInteger)value :(NSUInteger)amount
{
    // Subtract amount from value
    // Affects n, z, v, c - v, c are set by 'alu:'
    
    [self clrCCN];
    [self clrCCV];
    [self clrCCZ];
    
    NSUInteger comp = [self negate:amount] & 0xFFFF;
    NSUInteger answer = [self alu:value :comp :YES];
	
    if (answer == 0) [self setCCZ];
    if ([self bitSet:answer :15]) [self setCCN];

    return answer;
}



- (void)subd:(NSUInteger)amount
{
    // Subtract amount from D
    // Affects n, z, v, c - all are set by 'sub16bit:'
    
    [self clrCCN];
    [self clrCCV];
    [self clrCCZ];
	[self clrCCC];

	NSUInteger d = (regA * 256) + regB;
    d = [self sub16bit:d :amount];
	regA = (d >> 8) & 0xFF;
	regB = d & 0xFF;
}



- (void)test:(NSUInteger)value
{
	// Tests value for zero or negative
	// Affects n, z, v - v is alwasy cleared
	
    [self clrCCN];
    [self clrCCZ];
    [self clrCCV];
    
	if (value == 0) [self setCCZ];
	if ([self bitSet:value :kSignBit]) [self setCCN];
}



- (void)transferDecode:(NSUInteger)regCode :(BOOL)swapOp
{
    // regCode contains an 8-bit number that identifies the two registers to be swapped
    // first four bits give source, second four bits give destination. So convert regCode
    // into an array of eight ints representing each bit
    
    NSUInteger sourceReg, destReg, d;
    
    [self decimalToBits:regCode];
    
    // Get value of first four bits - the source
    
    sourceReg = (bit[7] * 8) + (bit[6] * 4) + (bit[5] * 2) + bit[4];
    
    // Get value of second four bits - the destination
    
    destReg = (bit[3] * 8) + (bit[2] * 4) + (bit[1] * 2) + bit[0];
    
    switch (sourceReg)
    {
        case 0x00:

			// D - can only swap to X, Y, U, S, PC

			if (destReg > 0x05) return;

            d = [self exchange16bit:((regA * 256) + regB) :destReg];
            
            if (swapOp)
            {
                regA = (d >> 8) & 0xFF;
                regB = d & 0xFF;
            }
            
            break;
            
        case 0x01:

			// X - can only swap to D, Y, U, S, PC

			if (destReg > 0x05) return;

			if (swapOp)
            {
                regX = [self exchange16bit:regX :destReg];
            }
            else 
            {
                [self exchange16bit:regX :destReg];
            }
            
            break;

        case 0x02:

			// Y - can only swap to D, X, U, S, PC

			if (destReg > 0x05) return;

			if (swapOp)
            {
                regY = [self exchange16bit:regY :destReg];
            }
            else 
            {
                [self exchange16bit:regY :destReg];
            }
            
            break;

        case 0x03:

			// U - can only swap to D, X, Y, S, PC

			if (destReg > 0x05) return;

			if (swapOp)
            {
                regUSP = [self exchange16bit:regUSP :destReg];
            }
            else 
            {
                [self exchange16bit:regUSP :destReg];
            }
            
            break;

        case 0x04:

			// S - can only swap to D, X, Y, U, PC

			if (destReg > 0x05) return;

			if (swapOp)
            {
                regHSP = [self exchange16bit:regHSP :destReg];
            }
            else 
            {
                [self exchange16bit:regHSP :destReg];
            }
            
            break;
            
        case 0x05:

			// PC - can only swap to D, X, Y, U, S

			if (destReg > 0x05) return;

			if (swapOp)
            {
                regPC = [self exchange16bit:regPC :destReg];
            }
            else 
            {
                [self exchange16bit:regPC :destReg];
            }
            
            break;

        case 0x08:

			// A - can only swap to B, CC, DP

			if (destReg < 0x08) return;

			if (swapOp)
            {
                regA = [self exchange:regA :destReg];
            }
            else 
            {
                [self exchange:regA :destReg];
            }
            
            break;
            
        case 0x09:

			// B - can only swap to A, CC, DP

			if (destReg < 0x08) return;

			if (swapOp)
            {
                regB = [self exchange:regB :destReg];
            }
            else 
            {
                [self exchange:regB :destReg];
            }
            
            break;
            
        case 0x0A:

			// CC - can only swap to A, B, DP

			if (destReg < 0x08) return;

			if (swapOp)
            {
                regCC = [self exchange:regCC :destReg];
            }
            else 
            {
                [self exchange:regCC :destReg];
            }

            break;
            
        case 0x0B:

			// DPR - can only swap to A, B, CC

			if (destReg < 0x08) return;

			if (swapOp)
            {
                regDP = [self exchange:regDP :destReg];
            }
            else 
            {
                [self exchange:regDP :destReg];
            }
    }
}



#pragma mark - Indexed Addressing Methods


- (NSUInteger)indexedAddressing:(NSUInteger)postByte
{
    // This method increases the PC

    NSUInteger sourceReg, opcode, value, msb, lsb;
    NSInteger address;
    
    // Decode the indexed addressing postbyte and use, in conjunction where appropriate, with the two
    // offset bytes, to calculate the Effective Address, which is returned.
    
    // For indirect addressing modes, we use the initial EA to look up the final EA from memory and 
    // return the final EA

    [self decimalToBits:postByte];

    // Point to the next byte, whatever it is

    [self incrementPC:1];

    // Get bits 5 and 6 to calculate source register
    // x = 0 ; y = 1 ; u = 2 ; s = 3
    
    sourceReg = (bit[6] * 2) + bit[5];
    
    // Set 'returnValue' to the contents of the source register
    
    address = [self registerValue:sourceReg];
    
    // Process the opcode encoded in the first five bits of the postbyte
    
    opcode = bit[0] + (0x02 * bit[1]) + (0x04 * bit[2]) + (0x08 * bit[3]) + (0x0A * bit[4]);
    
    if (bit[7] == 0)
    {
        // 5-bit non-indirect offset
        
        NSUInteger offset = opcode + (bit[5] * 0x20);
        address += [self bitSet:offset :5] ? offset - 16 : offset;
        address = [self checkRange:address];
    }
    else 
    {
        // All other opcodes have bit 7 set to 1
        
        switch (opcode)
        {
            case 0:
                // Auto-increment (,R+)
                [self incrementRegister:sourceReg :1];
                break;
                
            case 1:
                // Auto-increment (,R++)
                [self incrementRegister:sourceReg :2];
                break;
                
            case 2:
                // Auto-decrement (,-R)
                [self incrementRegister:sourceReg :-1];
                
                // Reacquire 'address' as decrement must here be performed FIRST
                address = [self registerValue:sourceReg];
                break;
                
            case 3:
                // Auto-decrement (,--R)
                [self incrementRegister:sourceReg :-2];
                
                // Reacquire 'address' as decrement must here be performed FIRST
                address = [self registerValue:sourceReg];
                break;
                
            case 4:
                // No offset (,R)
                break;
                
            case 5:
                // Accumulator offset B
                address += [self bitSet:regB :kSignBit] ? regB - 256 : regB;
                address = [self checkRange:address];
                break;
                
            case 6:
                // Accumulator offset A
                address += [self bitSet:regA :kSignBit] ? regA - 256 : regA;
                address = [self checkRange:address];
                break;
                
            case 8:
                // 8-bit constant offset
                value = [self loadFromRam];
                address += [self bitSet:value :kSignBit] ? value - 256 : value;
                address = [self checkRange:address];
                break;
                
            case 9:
                // 16-bit constant offset
                msb = [self loadFromRam];
                lsb = [self loadFromRam];
                value = (msb << 8) + lsb;
                address += [self bitSet:value :15] ? value - 65536 : value;
                address = [self checkRange:address];
                break;
                
            case 11:
                // Accumulator offset D
                value = (regA << 8) + regB;
                address += [self bitSet:value :15] ? value - 65536 : value;
                address = [self checkRange:address];
                break;
                
            case 12:
                // PC relative 8-bit offset
                value = [self loadFromRam];
                address = regPC + ([self bitSet:value :kSignBit] ? value - 256 : value);
                address = [self checkRange:address];
                break;
                
            case 13:
                // regPC relative 16-bit offset
                msb = [self loadFromRam];
                lsb = [self loadFromRam];
                value = (msb << 8) + lsb;
                address = regPC + ([self bitSet:value :15] ? value - 65536 : value);
                address = [self checkRange:address];
                break;
                
                // From here on, the addressing is indirect: the Effective Address is a handle not a pointer
                // So we need to add a second memoryMap[] lookup to return the ultimate address we are pointing to
                
            case 17:
                // Indirect auto-increment + 2
                [self incrementRegister:sourceReg :2];
                address = [ram peek:address];
                break;
                
            case 19:
                // Indirect auto-decrement - 2
                [self incrementRegister:sourceReg :-2];
                
                // Reacquire returnValue as decrement is performed FIRST
                address = [self registerValue:sourceReg];
                address = [ram peek:address];
                break;
                
            case 20:
                // Indirect constant zero offset
                address = [ram peek:address];
                break;
                
            case 21:
                // Indirect Accumulator offset B
                address += ([self bitSet:regB :kSignBit] ? regB - 256 : regB);
                address = [ram peek:address];
                break;
                
            case 22:
                // Indirect Accumulator offset A
                address += ([self bitSet:regA :kSignBit] ? regA - 256 : regA);
                address = [ram peek:address];
                break;
                
            case 24:
                // Indirect constant 8-bit offset
                value = [self loadFromRam];
                address += [self bitSet:value :kSignBit] ? value - 256 : value;
                address = [self checkRange:address];
                address = [ram peek:address];
                break;
                
            case 25:
                // Indirect constant 16-bit offset
                msb = [self loadFromRam];
                lsb = [self loadFromRam];
                value = (msb << 8) + lsb;
                address += [self bitSet:value :15] ? value - 65536 : value;
                address = [self checkRange:address];
                address = [ram peek:address];
                break;
                
            case 27:
                // Indirect Accumulator offset D
                value = (regA << 8) + regB;
                address += [self bitSet:value :15] ? value - 65536 : value;
                address = [self checkRange:address];
                address = [ram peek:address];
                break;
                
            case 28:
                // Indirect regPC relative 8-bit offset
                value = [self loadFromRam];
                address = regPC + ([self bitSet:value :kSignBit] ? value - 256 : value);
                address = [self checkRange:address];
                address = [ram peek:address];
                break;
                
            case 29:
                // Indirect regPC relative 16-bit offset
                msb = [self loadFromRam];
                lsb = [self loadFromRam];
                value = (msb << 8) + lsb;
                address = regPC + ([self bitSet:value :15] ? value - 65536 : value);
                address = [self checkRange:address];
                address = [ram peek:address];
                break;
                
            case 31:
                // Extended indirect
                msb = [self loadFromRam];
                lsb = [self loadFromRam];
                address = (msb << 8) + lsb;
                address = [ram peek:address];
                break;
        }
    }
    
    return (NSUInteger)address;
}



- (NSUInteger)registerValue:(NSUInteger)sourceRegister
{
    // Return the value of the appropriate index register
    // x = 0 ; y = 1 ; u = 2 ; s = 3
    
    if (sourceRegister == 0) return regX;
    if (sourceRegister == 1) return regY;
    if (sourceRegister == 2) return regUSP;
    return regHSP;
}



- (void)incrementRegister:(NSUInteger)sourceRegister :(NSUInteger)value
{
    // Calculate the offset from the Two's Complement of the 8- or 16-bit value
    
    if (sourceRegister == 0) regX += value;
    if (sourceRegister == 1) regY += value;
    if (sourceRegister == 2) regUSP += value;
    if (sourceRegister == 3) regHSP += value;
}


#pragma mark - Extended Addressing




#pragma mark - Test routines



@end
