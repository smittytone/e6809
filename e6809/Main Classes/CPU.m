
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


- (NSUInteger)fromRam:(NSUInteger)address
{
    // Returns the byte at the specified address

    return [ram peek:address] & 0xFF;
}



- (void)toRam:(NSUInteger)address :(NSUInteger)value
{
	[ram poke:(address & 0xFFFF) :(value & 0xFF)];
}



- (NSUInteger)loadFromRam
{
    // Return the byte at the address stored in regPC and
    // auto-increment regPC aftewards, handling rollover

    NSUInteger load = [self fromRam:(NSUInteger)regPC];
    regPC++;
    return load;
}



- (void)incrementPC:(NSInteger)amount
{
    unsigned short address = (unsigned short)regPC;
    address += (short)amount;
    regPC = (NSUInteger)address;
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



#pragma mark - Process Instructions


- (void)processNextInstruction
{
    // Are we awaiting an interrupt? If so check/handle then bail

    if (waitForInterruptFlag == YES)
    {
        // Process interrupts

        return;
    }

    // Get the memory cell contents

    NSUInteger opcode = [self loadFromRam];
    NSUInteger extendedOpcode = 0;
    NSUInteger addressMode = 0;

    // Check for extend opcodes: prefix 0x10 or 0x11

    if (opcode == opcode_extended_set_1 || opcode == opcode_extended_set_2)
    {
        // Set the flag and load the next byte - the actual opcode

        extendedOpcode = opcode;
        opcode = [self loadFromRam];
    }

    // Process all ops but NOT

    if (opcode != 0x12)
    {
        NSUInteger opcodeHi = (opcode & 0xF0) >> 4;
        NSUInteger opcodeLo = opcode & 0x0F;

        if (opcodeHi == 0x01)
        {
            // These ops have only one, specific address mode each

            if (opcodeLo == 0x03) [self sync];
            if (opcodeLo == 0x06) [self doBranch:opcode_BRA_rel :YES];  // Set correct op for LBRA handling
            if (opcodeLo == 0x07) [self doBranch:opcode_BSR_rel :YES];  // Set correct op for LBSR handling
            if (opcodeLo == 0x09) [self daa];
            if (opcodeLo == 0x0A) [self  orcc:[self loadFromRam]];
            if (opcodeLo == 0x0C) [self andcc:[self loadFromRam]];
            if (opcodeLo == 0x0D) [self sex];
            if (opcodeLo == 0x0E) [self transferDecode:[self loadFromRam] :YES];
            if (opcodeLo == 0x0F) [self transferDecode:[self loadFromRam]  :NO];
            return;
        }

        if (opcodeHi == 0x02)
        {
            // All 0x2x operations are branch ops

            [self doBranch:opcode :(extendedOpcode != 0)];
            return;
        }

        if (opcodeHi == 0x03)
        {
            // These ops have only one, specific address mode each

            if (opcodeLo  < 0x04) [self lea:opcode];
            if (opcodeLo == 0x04) [self push:YES :[self loadFromRam]];
            if (opcodeLo == 0x06) [self push:NO  :[self loadFromRam]];
            if (opcodeLo == 0x05) [self pull:YES :[self loadFromRam]];
            if (opcodeLo == 0x07) [self pull:NO  :[self loadFromRam]];
            if (opcodeLo == 0x0A) [self rts];
            if (opcodeLo == 0x0A) [self abx];
            if (opcodeLo == 0x0B) [self rti];
            if (opcodeLo == 0x0C) [self cwai];
            if (opcodeLo == 0x0D) [self mul];

            if (opcodeLo == 0x0F)
            {
                // This value of 'opcodeLo' takes in all varieties of SWI

                if (extendedOpcode == 0x00) [self swi];
                if (extendedOpcode == 0x10) [self swi2];
                if (extendedOpcode == 0x11) [self swi3];
            }

            return;
        }

        // Set the address mode as far as we can

        if (opcodeHi == 0x08 || opcodeHi == 0x0C) addressMode = kAddressModeImmediate;
        if (opcodeHi == 0x00 || opcodeHi == 0x09 || opcodeHi == 0x0D) addressMode = kAddressModeDirect;
        if (opcodeHi == 0x06 || opcodeHi == 0x0A || opcodeHi == 0x0E) addressMode = kAddressModeIndexed;
        if (opcodeHi == 0x07 || opcodeHi == 0x0B || opcodeHi == 0x0F) addressMode = kAddressModeExtended;
        if (opcodeHi == 0x04 || opcodeHi == 0x05) addressMode = kAddressModeInherent;

        // Jump to specific ops or groups of ops

        if (opcodeLo == 0x00 && opcodeHi > 0x07) [self sub:opcode :addressMode];
        if (opcodeLo == 0x00) [self neg:opcode :addressMode];

        if (opcodeLo == 0x01) [self cmp:opcode :addressMode];

        if (opcodeLo == 0x02) [self sbc:opcode :addressMode];

        if (opcodeLo == 0x03 && opcodeHi > 0x0B) [self add16:opcode :addressMode];
        if (opcodeLo == 0x03 && opcodeHi > 0x07) [self sub16:opcode :addressMode];
        if (opcodeLo == 0x03) [self com:opcode :addressMode];

        if (opcodeLo == 0x04 && opcodeHi < 0x08) [self lsr:opcode :addressMode];
        if (opcodeLo == 0x04) [self and:opcode :addressMode];

        if (opcodeLo == 0x05) [self bit:opcode :addressMode];

        if (opcodeLo == 0x06 && opcodeHi < 0x08) [self ror:opcode :addressMode];
        if (opcodeLo == 0x06) [self ld:opcode :addressMode];

        if (opcodeLo == 0x07 && opcodeHi < 0x08) [self asr:opcode :addressMode];
        if (opcodeLo == 0x07) [self st:opcode :addressMode];

        if (opcodeLo == 0x08 && opcodeHi < 0x08) [self asl:opcode :addressMode];
        if (opcodeLo == 0x08) [self eor:opcode: addressMode];

        if (opcodeLo == 0x09 && opcodeHi < 0x08) [self rol:opcode :addressMode];
        if (opcodeLo == 0x09) [self adc:opcode: addressMode];

        if (opcodeLo == 0x0A && opcodeHi < 0x08) [self dec:opcode :addressMode];
        if (opcodeLo == 0x0A) [self orr:opcode :addressMode];

        if (opcodeLo == 0x0B) [self add:opcode :addressMode];

        if (opcodeLo == 0x0C && opcodeHi < 0x08) [self inc:opcode :addressMode];
        if (opcodeLo == 0x0C && opcodeHi > 0x0C) [self ld16:opcode :addressMode :extendedOpcode];   // LDD
        if (opcodeLo == 0x0C) [self cmp16:opcode :addressMode :extendedOpcode];

        if (opcodeLo == 0x0D && opcodeHi < 0x08) [self tst:opcode :addressMode];
        if (opcodeLo == 0x0D && opcodeHi > 0x0C) [self st16:opcode :addressMode :extendedOpcode];   // STD
        if (opcodeLo == 0x0D) [self jsr:addressMode];

        if (opcodeLo == 0x0E && opcodeHi < 0x08) [self jmp:addressMode];
        if (opcodeLo == 0x0E && opcodeHi > 0x07) [self ld16:opcode :addressMode :extendedOpcode];

        if (opcodeLo == 0x0F && opcodeHi > 0x08) [self st16:opcode :addressMode :extendedOpcode];
        if (opcodeLo == 0x0F) [self clr:opcode :addressMode];
    }
}



- (void)doBranch:(NSUInteger)op :(BOOL)isLong
{
    // Handler for multiple branch types

    short offset = isLong ? [self addressFromNextTwoBytes]: [self loadFromRam];
    BOOL branch = NO;

    if (op == opcode_BRA_rel) branch = YES;

    if (op == opcode_BEQ_rel && [self bitSet:regCC :kCC_z]) branch = YES;
    if (op == opcode_BNE_rel && ![self bitSet:regCC :kCC_z]) branch = YES;

    if (op == opcode_BMI_rel && [self bitSet:regCC :kCC_n]) branch = YES;
    if (op == opcode_BPL_rel && ![self bitSet:regCC :kCC_n]) branch = YES;

    if (op == opcode_BVS_rel && [self bitSet:regCC :kCC_v]) branch = YES;
    if (op == opcode_BVC_rel && ![self bitSet:regCC :kCC_v]) branch = YES;

    if (op == opcode_BLO_rel && [self bitSet:regCC :kCC_c]) branch = YES;   // Also BCS
    if (op == opcode_BHS_rel && ![self bitSet:regCC :kCC_c]) branch = YES;  // Also BCC

    if (op == opcode_BGE_rel && ([self bitSet:regCC :kCC_n] == [self bitSet:regCC :kCC_v])) branch = YES;
    if (op == opcode_BGT_rel && ![self bitSet:regCC :kCC_z] && [self bitSet:regCC :kCC_n] == [self bitSet:regCC :kCC_v]) branch = YES;
    if (op == opcode_BHI_rel && ![self bitSet:regCC :kCC_c] && ![self bitSet:regCC :kCC_z]) branch = YES;
    if (op == opcode_BLE_rel && ([self bitSet:regCC :kCC_z] || (([self bitSet:regCC :kCC_n] || [self bitSet:regCC :kCC_v]) && [self bitSet:regCC :kCC_n] != [self bitSet:regCC :kCC_v]))) branch = YES;
    if (op == opcode_BLS_rel && [self bitSet:regCC :kCC_c] && [self bitSet:regCC :kCC_z]) branch = YES;
    if (op == opcode_BLT_rel && (([self bitSet:regCC :kCC_n] || [self bitSet:regCC :kCC_v]) && [self bitSet:regCC :kCC_n] != [self bitSet:regCC :kCC_v])) branch = YES;

    if (op == opcode_BSR_rel)
    {
        // Branch to Subroutine: push PC to hardware stack (S) first

        branch = YES;
        regHSP--;
        [self toRam:regHSP :(regPC & 0xFF)];
        regHSP--;
        [self toRam:regHSP :((regPC >> 8) & 0xFF)];
    }

    if (branch) regPC += offset;
}



# pragma mark - Specific Operation Methods

- (void)abx
{
    // ABX: B + X -> X (unsigned)

    regX += regB;
    if (regX > 0xFFFF) regX = (regX & 0xFFFF);
}



- (void)adc:(NSUInteger)op :(NSUInteger)mode
{
    // ADC: A + M + C -> A,
    //      B + M + C -> A

    NSUInteger address = [self addressFromMode:mode];

    if (op < 0xC9)
    {
        regA = [self addWithCarry:regA :[self fromRam:address]];
    }
    else
    {
        regB = [self addWithCarry:regB :[self fromRam:address]];
    }
}



- (void)add:(NSUInteger)op :(NSUInteger)mode
{
    // ADD: A + M -> A
    //      B + M -> N

    NSUInteger address = [self addressFromMode:mode];

    if (op < 0xCB)
    {
        regA = [self doAdd:regA :[self fromRam:address]];
    }
    else
    {
        regB = [self doAdd:regB :[self fromRam:address]];
    }
}



- (void)add16:(NSUInteger)op :(NSUInteger)mode
{
    // ADDD: D + M:M + 1 -> D

    [self clrCCN];
    [self clrCCZ];

    // Cast 'address' as unsigned short so the + 1 correctly rolls over, if necessary

    unsigned short address = (unsigned short)[self addressFromMode:mode];
    NSUInteger msb = [self fromRam:address];
    NSUInteger lsb = [self fromRam:address + 1];

    // Add the two LSBs (M + 1, B) to set the carry,
    // then add the two MSBs (M, A) with the carry

    lsb = [self alu:regB :lsb :NO];
    msb = [self alu:regA :msb :YES];

    // Convert the bytes back to a 16-bit value and set the CC

    NSUInteger answer = (msb << 8) + lsb;
    if (answer == 0) [self setCCZ];
    if ([self bitSet:answer :15]) [self setCCN];

    // Set D's component registers

    regA = (answer >> 8) & 0xFF;
    regB = answer & 0xFF;
}



- (void)and:(NSUInteger)op :(NSUInteger)mode
{
    // AND: A & M -> A
    //      B & M -> N

    NSUInteger address = [self addressFromMode:mode];

    if (op < 0xC4)
    {
        regA = [self doAnd:regA :[self fromRam:address]];
    }
    else
    {
        regB = [self doAnd:regB :[self fromRam:address]];
    }
}



- (void)andcc:(NSUInteger)value
{
    // AND CC: CC & M -> CC

    regCC = regCC & value;
}



- (void)asl:(NSUInteger)op :(NSUInteger)mode
{
    // ASL: arithmetic shift left A -> A,
    //      arithmetic shift left B -> B,
    //      arithmetic shift left M -> M
    // This is is the same as LSL

    if (mode == kAddressModeInherent)
    {
        if (op == 0x48)
        {
            regA = [self logicShiftLeft:regA];
        }
        else
        {
            regB = [self logicShiftLeft:regB];
        }
    }
    else
    {
        NSUInteger address = [self addressFromMode:mode];
        [self toRam:address :[self logicShiftLeft:[self fromRam:address]]];
    }
}



- (void)asr:(NSUInteger)op :(NSUInteger)mode
{
    // ASR: arithmetic shift right A -> A,
    //      arithmetic shift right B -> B,
    //      arithmetic shift right M -> M

    if (mode == kAddressModeInherent)
    {
        if (op == 0x47)
        {
            regA = [self arithmeticShiftRight:regA];
        }
        else
        {
            regB = [self arithmeticShiftRight:regB];
        }
    }
    else
    {
        NSUInteger address = [self addressFromMode:mode];
        [self toRam:address :[self arithmeticShiftRight:[self fromRam:address]]];
    }
}



- (void)bit:(NSUInteger)op :(NSUInteger)mode
{
    // BIT: Bit test A (A & M)
    //      Bit test B (B & M)
    // Does not affect the operands, only the CC register

    NSUInteger address = [self addressFromMode:mode];
    [self doAnd:(op < 0xC5 ? regA : regB) :[self fromRam:address]];
}



- (void)clr:(NSUInteger)op :(NSUInteger)mode
{
    // CLR: 0 -> A
    //      0 -> B
    //      0 -> M

    if (mode == kAddressModeInherent)
    {
        if (op == 0x4F) { regA = 0; } else { regB = 0; }
    }
    else
    {
        NSUInteger address = [self addressFromMode:mode];
        [self toRam:address :0];
    }

    [self setCCAfterClear];
}



- (void)cmp:(NSUInteger)op :(NSUInteger)mode
{
    // CMP: Compare M to A
    //      Compare M to B

    NSUInteger address = [self addressFromMode:mode];
    [self compare:(op < 0xC1 ? regA : regB) :[self fromRam:address]];
}



- (void)cmp16:(NSUInteger)op :(NSUInteger)mode :(NSUInteger)exOp
{

}



- (void)com:(NSUInteger)op :(NSUInteger)mode
{
    // COM: !A -> A,
    //      !B -> B,
    //      !M -> A

    if (mode == kAddressModeInherent)
    {
        if (op == 0x43)
        {
            regA = [self complement:regA];
        }
        else
        {
            regB = [self complement:regB];
        }
    }
    else
    {
        NSUInteger address = [self addressFromMode:mode];
        [self toRam:address :[self complement:[self fromRam:address]]];
    }
}



- (void)cwai
{
    // CWAI
    // Clear CC bits and Wait for Interrupt
    // AND CC with the operand, set e, then push every register,
    // including CC to the hardware stack

    regCC = [self doAnd:regCC :[ram peek:regPC]];
    regCC = [self setBit:regCC :kCC_e];
    [self push:YES :kPushPullRegEvery];
    waitForInterruptFlag = YES;
}



- (void)daa
{
    // DAA: Decimal Adjust A

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



- (void)dec:(NSUInteger)op :(NSUInteger)mode
{
    // DEC: A - 1 -> A,
    //      B - 1 -> B,
    //      M - 1 -> M

    if (mode == kAddressModeInherent)
    {
        if (op == 0x4A)
        {
            regA = [self decrement:regA];
        }
        else
        {
            regB = [self decrement:regB];
        }
    }
    else
    {
        NSUInteger address = [self addressFromMode:mode];
        [self toRam:address :[self decrement:[self fromRam:address]]];
    }
}



- (void)eor:(NSUInteger)op :(NSUInteger)mode
{
    // EOR: A ^ M -> A
    //      B ^ M -> B

    NSUInteger address = [self addressFromMode:mode];

    if (op < 0xC8)
    {
        regA = [self doXOr:regA :[self fromRam:address]];
    }
    else
    {
        regB = [self doXOr:regB :[self fromRam:address]];
    }
}



- (void)inc:(NSUInteger)op :(NSUInteger)mode
{
    // INC: A + 1 -> A,
    //      B + 1 -> B,
    //      M + 1 -> M

    if (mode == kAddressModeInherent)
    {
        if (op == 0x43)
        {
            regA = [self increment:regA];
        }
        else
        {
            regB = [self increment:regB];
        }
    }
    else
    {
        NSUInteger address = [self addressFromMode:mode];
        [self toRam:address :[self increment:[self fromRam:address]]];
    }
}



- (void)jmp:(NSUInteger)mode
{
    // JMP: M -> PC

    regPC = [self addressFromMode:mode];
}



- (void)jsr:(NSUInteger)mode
{
    // JSR: S = S - 1;
    //      PC LSB to stack;
    //      S = S- 1;
    //      PC MSB to stack;
    //      M -> PC

    regHSP--;
    [self toRam:regHSP :(regPC & 0xFF)];
    regHSP--;
    [self toRam:regHSP :((regPC >> 8) & 0xFF)];
    regPC = [self addressFromMode:mode];
}



- (void)ld:(NSUInteger)op :(NSUInteger)mode
{
    // LD: M -> A,
    //     M -> B

    NSUInteger address = [self addressFromMode:mode];

    if (op < 0xC6)
    {
        regA = [self fromRam:address];
        [self setCCAfterLoad:regA];
    }
    else
    {
        regB = [self fromRam:address];
        [self setCCAfterLoad:regB];
    }
}



- (void)ld16:(NSUInteger)op :(NSUInteger)mode :(NSUInteger)exOp
{

}



- (void)lea:(NSUInteger)op
{
    // LEA: EA -> S,
    //      EA -> U,
    //      EA -> X,
    //      EA -> Y

    [self loadEffective:[self indexedAddressing:[self loadFromRam]] :(op & 0x03)];
}



- (void)lsr:(NSUInteger)op :(NSUInteger)mode
{
    // LSR: logic shift right A -> A,
    //      logic shift right B -> B,
    //      logic shift right M -> M

    if (mode == kAddressModeInherent)
    {
        if (op == 0x44)
        {
            regA = [self logicShiftRight:regA];
        }
        else
        {
            regB = [self logicShiftRight:regB];
        }
    }
    else
    {
        NSUInteger address = [self addressFromMode:mode];
        [self toRam:address :[self logicShiftRight:[self fromRam:address]]];
    }
}



- (void)mul
{
    // MUL: A x B -> D (unsigned)

    [self clrCCZ];
    [self clrCCC];

    NSUInteger d = regA * regB;
    if ([self bitSet:d :7]) [self setCCC];
    if (d == 0) [self setCCZ];

    regA = (d >> 8) & 0xFF;
    regB = d & 0xFF;
}



- (void)neg:(NSUInteger)op :(NSUInteger)mode
{
    // NEG: !R + 1 -> R, !M + 1 -> M

    if (mode == kAddressModeInherent)
    {
        if (op == 0x40) { regA = [self negate:regA]; } else { regB = [self negate:regB]; }
    }
    else
    {
        NSUInteger address = [self addressFromMode:mode];
        [self toRam:address :[self negate:[self fromRam:address]]];
    }
}



- (void)orr:(NSUInteger)op :(NSUInteger)mode
{
    // OR: A | M -> A
    //     B | M -> B

    NSUInteger address = [self addressFromMode:mode];

    if (op < 0xCA)
    {
        regA = [self doOr:regA :[self fromRam:address]];
    }
    else
    {
        regB = [self doOr:regB :[self fromRam:address]];
    }
}



- (void)orcc:(NSUInteger)value
{
    // OR CC: CC | M -> CC

    regCC = (regCC | value) & 0xFF;
}



- (void)rol:(NSUInteger)op :(NSUInteger)mode
{
    // ROL: rotate left A -> A,
    //      rotate left B -> B,
    //      rotate left M -> M

    if (mode == kAddressModeInherent)
    {
        if (op == 0x49)
        {
            regA = [self rotateLeft:regA];
        }
        else
        {
            regB = [self rotateLeft:regB];
        }
    }
    else
    {
        NSUInteger address = [self addressFromMode:mode];
        [self toRam:address :[self rotateLeft:[self fromRam:address]]];
    }
}



- (void)ror:(NSUInteger)op :(NSUInteger)mode
{
    // ROR: rotate right A -> A,
    //      rotate right B -> B,
    //      rotate right M -> M

    if (mode == kAddressModeInherent)
    {
        if (op == 0x46)
        {
            regA = [self rotateRight:regA];
        }
        else
        {
            regB = [self rotateRight:regB];
        }
    }
    else
    {
        NSUInteger address = [self addressFromMode:mode];
        [self toRam:address :[self rotateRight:[self fromRam:address]]];
    }
}



- (void)rti
{
    // RTI
    // Pull CC from the hardware stack; if e is set, pull all the registers from the hardware stack,
    // otherwise pull the PC register only

    [self pull:YES :kPushPullRegCC];
    if ([self bitSet:regCC :kCC_e]) [self pull:YES :kPushPullRegAll];
}



- (void)rts
{
    // RTS
    // Pull the PC from the hardware stack

    regPC = ([ram peek:regHSP] * 256);
    regHSP++;
    if (regHSP > 0xFFFF) regHSP = 0x0000;
    regPC += [ram peek:regHSP];
    regHSP++;
    if (regHSP > 0xFFFF) regHSP = 0x0000;

}



- (void)sbc:(NSUInteger)op :(NSUInteger)mode
{
    // SBC: A - M - C -> A,
    //      B - M - C -> A

    NSUInteger address = [self addressFromMode:mode];

    if (op < 0xC2)
    {
        regA = [self subWithCarry:regA :[self fromRam:address]];
    }
    else
    {
        regB = [self subWithCarry:regB :[self fromRam:address]];
    }
}



- (void)sex
{
    // SEX: sign-extend B into A
    // Affects n, z

    [self clrCCN];
    [self clrCCZ];

    regA = 0x00;

    if ([self bitSet:regB :kSignBit])
    {
        regA = 0xFF;
        [self setCCN];
    }

    if (regB == 0) [self setCCZ]; // ***CHECK***
}



- (void)st:(NSUInteger)op :(NSUInteger)mode
{
    // ST: A -> M,
    //     B -> M

    NSUInteger address = [self addressFromMode:mode];

    if (op < 0xD7)
    {
        [self toRam:address :regA];
        [self setCCAfterStore:regA];
    }
    else
    {
        [self toRam:address :regB];
        [self setCCAfterStore:regB];
    }
}



- (void)st16:(NSUInteger)op :(NSUInteger)mode :(NSUInteger)exOp
{

}



- (void)sub:(NSUInteger)op :(NSUInteger)mode
{
    // SUB: A - M -> A,
    //      B - M -> B

    NSUInteger address = [self addressFromMode:mode];

    if (op == 0x82)
    {
        regA = [self subtract:regA :[self fromRam:address]];
    }
    else
    {
        regB = [self subtract:regB :[self fromRam:address]];
    }
}



- (void)sub16:(NSUInteger)op :(NSUInteger)mode
{
    // SUBD: D - M:M + 1 -> D

    [self clrCCN];
    [self clrCCZ];
    [self clrCCV];

    // Cast 'address' as unsigned short so the + 1 correctly rolls over, if necessary

    unsigned short address = (unsigned short)[self addressFromMode:mode];

    // Complement the value at M:M + 1

    NSUInteger msb = [self negate:[self fromRam:address]];
    NSUInteger lsb = [self negate:[self fromRam:address + 1]];

    // Add 1 to form the 2's complement

    lsb = [self alu:lsb :1 :NO];
    msb = [self alu:msb :0 :YES];

    // Add D - A and B

    lsb = [self alu:regB :lsb :NO];
    msb = [self alu:regA :msb :YES];

    // Convert the bytes back to a 16-bit value and set the CC

    NSUInteger answer = (msb << 8) + lsb;
    if (answer == 0) [self setCCZ];
    if ([self bitSet:answer :15]) [self setCCN];

    // Set D's component registers

    regA = (answer >> 8) & 0xFF;
    regB = answer & 0xFF;
}



- (void)swi
{
    // SWI: SoftWare Interrupt
    // Set e to 1 then push every register to the hardware stack

    regCC = [self setBit:regCC :kCC_e];
    [self push:YES :kPushPullRegEvery];

    // Set i and f

    regCC = [self setBit:regCC :kCC_i];
    regCC = [self setBit:regCC :kCC_f];
    waitForInterruptFlag = NO;

    // Set the PC to the interrupt vector

    regPC = ([ram peek:kInterruptVectorOne] << 8) + [ram peek:kInterruptVectorOne + 1];
}



- (void)swi2
{
    // SWI2: SoftWare Interrupt 2
    // Set e to 1 then push every register to the hardware stack

    regCC = [self setBit:regCC :kCC_e];
    [self push:YES :kPushPullRegEvery];
    waitForInterruptFlag = NO;

    // Set PC to the interrupt vector
    regPC = [ram peek:kInterruptVectorTwo] * 256 + [ram peek:kInterruptVectorTwo + 1];
}



- (void)swi3
{
    // SWI3: SoftWare Interrupt 3
    // Set e to 1 then push every register to the hardware stack

    regCC = [self setBit:regCC :kCC_e];
    [self push:YES :kPushPullRegEvery];
    waitForInterruptFlag = NO;

    // Set the PC to the interrupt vector
    regPC = [ram peek:kInterruptVectorThree] * 256 + [ram peek:kInterruptVectorThree + 1];
}



- (void)sync
{
    // SYNC

    waitForInterruptFlag = YES;
}



- (void)tst:(NSUInteger)op :(NSUInteger)mode
{
    // TST: test A,
    //      test B,
    //      test M

    if (mode == kAddressModeInherent)
    {
        if (op == 0x4D)
        {
            [self test:regA];
        }
        else
        {
            [self test:regB];
        }
    }
    else
    {
        NSUInteger address = [self addressFromMode:mode];
        [self test:[self fromRam:address]];
    }
}



# pragma mark - Specific Operation Helper Methods


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



- (NSUInteger)doAdd:(NSUInteger)value :(NSUInteger)amount
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



- (NSUInteger)doAnd:(NSUInteger)value :(NSUInteger)amount
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



- (NSUInteger)doOr:(NSUInteger)value :(NSUInteger)amount
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



- (NSUInteger)doXOr:(NSUInteger)value :(NSUInteger)amount
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



- (NSUInteger)arithmeticShiftRight:(NSUInteger)value
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



- (NSUInteger)logicShiftRight:(NSUInteger)value
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



- (NSUInteger)logicShiftLeft:(NSUInteger)value
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



- (void)setCCAfterClear
{
    // Sets n, z, v, c generically for any clear operation, eg. CLRA, CLRB, CLR 0x0000
    // Values are fixed

    [self clrCCN];
    [self setCCZ];
    [self clrCCV];
    [self clrCCC];
}



- (void)setCCAfterLoad:(NSUInteger)value
{
    // Sets the CC after an 8-bit load
    // Affects n, z, v - v is always cleared

    [self clrCCN];
    [self clrCCZ];
    [self clrCCV];

    if (value == 0) [self setCCZ];
    if ([self bitSet:value :kSignBit]) [self setCCN];
}



- (void)setCCAfterStore:(NSUInteger)value
{
    // Sets CC after an 8-bit store
    // Affects n, z, v - v is always cleared

    [self clrCCN];
    [self clrCCZ];
    [self clrCCV];

    if (value == 0) [self setCCZ];
    if ([self bitSet:value :kSignBit]) [self setCCN];
}



- (void)compare:(NSUInteger)value :(NSUInteger)amount
{
    // Compare two values by subtracting the second from the first. Result is discarded
    // Affects n, z, v, c - c, v set by 'alu:' via 'sub:'

    [self clrCCN];
    [self clrCCZ];
    [self clrCCV];
    [self clrCCC];

    NSUInteger answer = [self subtract:value :amount];
    if (answer == 0) [self setCCZ];
    if ([self bitSet:answer :kSignBit]) [self setCCN];
}



- (void)compare16:(NSUInteger)value :(NSUInteger)amount
{
    // TODO
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

            d = [self exchange16:((regA * 256) + regB) :destReg];

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
                regX = [self exchange16:regX :destReg];
            }
            else
            {
                [self exchange16:regX :destReg];
            }

            break;

        case 0x02:

            // Y - can only swap to D, X, U, S, PC

            if (destReg > 0x05) return;

            if (swapOp)
            {
                regY = [self exchange16:regY :destReg];
            }
            else
            {
                [self exchange16:regY :destReg];
            }

            break;

        case 0x03:

            // U - can only swap to D, X, Y, S, PC

            if (destReg > 0x05) return;

            if (swapOp)
            {
                regUSP = [self exchange16:regUSP :destReg];
            }
            else
            {
                [self exchange16:regUSP :destReg];
            }

            break;

        case 0x04:

            // S - can only swap to D, X, Y, U, PC

            if (destReg > 0x05) return;

            if (swapOp)
            {
                regHSP = [self exchange16:regHSP :destReg];
            }
            else
            {
                [self exchange16:regHSP :destReg];
            }

            break;

        case 0x05:

            // PC - can only swap to D, X, Y, U, S

            if (destReg > 0x05) return;

            if (swapOp)
            {
                regPC = [self exchange16:regPC :destReg];
            }
            else
            {
                [self exchange16:regPC :destReg];
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



- (NSUInteger)exchange16:(NSUInteger)value :(NSUInteger)regCode
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
            regX = amount;
            if (amount == 0) [self setCCZ];
            break;

        case 0x02:
            regY = amount;
            if (amount == 0) [self setCCZ];
            break;

        case 0x03:
            regUSP = amount;
            break;

        case 0x04:
            regHSP = amount;
    }
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



- (NSUInteger)subtract:(NSUInteger)value :(NSUInteger)amount
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



#pragma mark - Addressing Methods


- (NSUInteger)addressFromMode:(NSUInteger)mode
{
    NSUInteger address = 0;

    if (mode == kAddressModeImmediate)
    {
        address = (NSUInteger)regPC;
        regPC++;
    }
    else if (mode == kAddressModeDirect)
    {
        address = [self addressFromDPR];
        regPC++;
    }
    else if (mode == kAddressModeIndexed)
    {
        address = [self indexedAddressing:[self loadFromRam]];
    }
    else
    {
        address = [self addressFromNextTwoBytes];
    }

    return address;
}



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
    // Convenience method that calls 'addressFromDPR:' with no offset

    return [self addressFromDPR:0];
}



- (NSUInteger)addressFromDPR:(NSInteger)offset
{
    // Returns the address composed from regDP (MSB) and the byte at
    // regPC (LSB) plus any supplied offset
    // Does not increment regPC
    // TODO Should we increment regPC?

    unsigned short address = regPC;
    address += (short)offset;
    return (regDP << 8) + [self fromRam:(NSUInteger)address];
}



- (NSUInteger)indexedAddressing:(NSUInteger)postByte
{
    // This method increases the PC

    NSUInteger sourceReg, opcode, value, msb, lsb, address;
    unsigned short subAddress;

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

        char offset = (char)(opcode + (bit[5] * 0x20));
        subAddress = (unsigned short)address;
        subAddress += offset;
        address = (NSUInteger)subAddress;
    }
    else
    {
        // All other opcodes have bit 7 set to 1

        switch (opcode)
        {
            case 0:
                // Auto-increment (,R+)
                address = [self registerValue:sourceReg];
                [self incrementRegister:sourceReg :1];
                break;

            case 1:
                // Auto-increment (,R++)
                address = [self registerValue:sourceReg];
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
                // No offset (,R) - 'address' already set
                break;

            case 5:
                // Accumulator offset B; B is a 2s-comp offset
                address += [self bitSet:regB :kSignBit] ? regB - 256 : regB;
                break;

            case 6:
                // Accumulator offset A; A is a 2s-comp offset
                address += [self bitSet:regA :kSignBit] ? regA - 256 : regA;
                break;

            case 8:
                // 8-bit constant offset; offset is 2s-comp
                value = [self loadFromRam];
                address += [self bitSet:value :kSignBit] ? value - 256 : value;
                break;

            case 9:
                // 16-bit constant offset; offset is 2s-comp
                msb = [self loadFromRam];
                lsb = [self loadFromRam];
                value = (msb << 8) + lsb;
                address += [self bitSet:value :15] ? value - 65536 : value;
                break;

            case 11:
                // Accumulator offset D; D is a 2s-comp offset
                value = (regA << 8) + regB;
                address += [self bitSet:value :15] ? value - 65536 : value;
                break;

            case 12:
                // PC relative 8-bit offset; offset is 2s-comp
                value = [self loadFromRam];
                address = regPC + ([self bitSet:value :kSignBit] ? value - 256 : value);
                break;

            case 13:
                // regPC relative 16-bit offset; offset is 2s-comp
                msb = [self loadFromRam];
                lsb = [self loadFromRam];
                value = (msb << 8) + lsb;
                address = regPC + ([self bitSet:value :15] ? value - 65536 : value);
                break;

                // From here on, the addressing is indirect: the Effective Address is a handle not a pointer
                // So we need to add a second memoryMap[] lookup to return the ultimate address we are pointing to

            case 17:
                // Indirect auto-increment + 2
                [self incrementRegister:sourceReg :2];
                break;

            case 19:
                // Indirect auto-decrement - 2
                [self incrementRegister:sourceReg :-2];

                // Reacquire returnValue as decrement is performed FIRST
                address = [self registerValue:sourceReg];
                break;

            case 20:
                // Indirect constant zero offset
                // address = [ram peek:address];
                break;

            case 21:
                // Indirect Accumulator offset B
                // eg. LDA [B,X]
                address += ([self bitSet:regB :kSignBit] ? regB - 256 : regB);
                break;

            case 22:
                // Indirect Accumulator offset A
                // eg. LDA [A,Y]
                address += ([self bitSet:regA :kSignBit] ? regA - 256 : regA);
                break;

            case 24:
                // Indirect constant 8-bit offset
                // eg. LDA [n,X]
                value = [self loadFromRam];
                address += [self bitSet:value :kSignBit] ? value - 256 : value;
                break;

            case 25:
                // Indirect constant 16-bit offset
                // eg. LDA [n,X]
                msb = [self loadFromRam];
                lsb = [self loadFromRam];
                value = (msb << 8) + lsb;
                address += [self bitSet:value :15] ? value - 65536 : value;
                break;

            case 27:
                // Indirect Accumulator offset D
                // eg. LDA [D,X]
                value = (regA << 8) + regB;
                address += [self bitSet:value :15] ? value - 65536 : value;
                break;

            case 28:
                // Indirect regPC relative 8-bit offset
                // eg. LDA [n,PCR]
                value = [self loadFromRam];
                address = regPC + ([self bitSet:value :kSignBit] ? value - 256 : value);
                break;

            case 29:
                // Indirect regPC relative 16-bit offset
                // eg. LDX [n,PCR]
                msb = [self loadFromRam];
                lsb = [self loadFromRam];
                value = (msb << 8) + lsb;
                address = regPC + ([self bitSet:value :15] ? value - 65536 : value);
                break;

            case 31:
                // Extended indirect
                // eg. LDA [n]
                msb = [self loadFromRam];
                lsb = [self loadFromRam];
                address = (msb << 8) + lsb;
                break;
        }
    }

    // Ensure address aligns to 16-bit range

    address = [self checkRange:address];

    // For indirect address, use 'address' as a handle

    if (opcode > 16)
    {
        NSUInteger pc = regPC;
        regPC = address;
        address = [self addressFromNextTwoBytes];
        regPC = pc;
    }

    return address;
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



- (void)incrementRegister:(NSUInteger)sourceRegister :(NSInteger)amount
{
    // Calculate the offset from the Two's Complement of the 8- or 16-bit value

    NSUInteger *regPtr = &regX;
    if (sourceRegister == 1) regPtr = &regY;
    if (sourceRegister == 2) regPtr = &regUSP;
    if (sourceRegister == 3) regPtr = &regHSP;

    unsigned short newValue = (unsigned short)*regPtr;
    newValue += (short)amount;
    *regPtr = (NSUInteger)newValue;
}




@end
