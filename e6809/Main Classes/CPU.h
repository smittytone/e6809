

#import <Foundation/Foundation.h>
#import <Math.h>
#import "Constants.h"
#import "RAM.h"

@interface MC6809 : NSObject

{
	// Ram

	RAM *ram;
    
	NSUInteger bit[16];
	NSUInteger topAddress;
    
	BOOL waitForInterruptFlag;
}


// Setup Methods
- (void)reset;
- (void)configMemory:(NSUInteger)memSizeInBytes;

// Memory Access Methods
- (NSUInteger)fromRam:(NSUInteger)address;
- (void)toRam:(NSUInteger)address :(NSUInteger)value;
- (NSUInteger)loadFromRam;
- (void)incrementPC:(NSInteger)amount;
- (NSInteger)checkRange:(NSInteger)address;

// Bit Handling Methods
- (void)clearBits;
- (void)decimalToBits:(NSUInteger)value;
- (void)decimalToBits16:(NSUInteger)value;
- (NSUInteger)bitsToDecimal;
- (NSUInteger)bitsToDecimal16;
- (BOOL)bitSet:(NSUInteger)value :(NSUInteger)bit;
- (NSUInteger)setBit:(NSUInteger)value :(NSUInteger)bit;
- (NSUInteger)clrBit:(NSUInteger)value :(NSUInteger)bit;
- (void)setCCV;
- (void)clrCCV;
- (void)setCCZ;
- (void)clrCCZ;
- (void)setCCN;
- (void)clrCCN;
- (void)setCCC;
- (void)clrCCC;

// Processing
- (void)processNextInstruction;
- (void)doBranch:(NSUInteger)opcode :(BOOL)isLong;

// Operations
- (void)abx;
- (void)adc:(NSUInteger)op :(NSUInteger)mode;
- (void)add:(NSUInteger)op :(NSUInteger)mode;
- (void)add16:(NSUInteger)op :(NSUInteger)mode;
- (void)and:(NSUInteger)op :(NSUInteger)mode;
- (void)andcc:(NSUInteger)value;
- (void)asl:(NSUInteger)op :(NSUInteger)mode;
- (void)asr:(NSUInteger)op :(NSUInteger)mode;
- (void)bit:(NSUInteger)op :(NSUInteger)mode;
- (void)clr:(NSUInteger)op :(NSUInteger)mode;
- (void)cmp:(NSUInteger)op :(NSUInteger)mode;
- (void)cmp16:(NSUInteger)op :(NSUInteger)mode :(NSUInteger)exOp; // NOT DONE
- (void)com:(NSUInteger)op :(NSUInteger)mode;
- (void)cwai;
- (void)daa;
- (void)dec:(NSUInteger)op :(NSUInteger)mode;
- (void)eor:(NSUInteger)op :(NSUInteger)mode;
- (void)inc:(NSUInteger)op :(NSUInteger)mode;
- (void)jmp:(NSUInteger)mode;
- (void)jsr:(NSUInteger)mode;
- (void)ld:(NSUInteger)op :(NSUInteger)mode;
- (void)ld16:(NSUInteger)op :(NSUInteger)mode :(NSUInteger)exOp;
- (void)lea:(NSUInteger)op;
- (void)lsr:(NSUInteger)op :(NSUInteger)mode;
- (void)mul;
- (void)neg:(NSUInteger)op :(NSUInteger)mode;
- (void)orr:(NSUInteger)op :(NSUInteger)mode;
- (void)orcc:(NSUInteger)value;
- (void)rol:(NSUInteger)op :(NSUInteger)mode;
- (void)ror:(NSUInteger)op :(NSUInteger)mode;
- (void)rti;
- (void)rts;
- (void)sbc:(NSUInteger)op :(NSUInteger)mode;
- (void)sex;
- (void)st:(NSUInteger)op :(NSUInteger)mode;
- (void)st16:(NSUInteger)op :(NSUInteger)mode :(NSUInteger)exOp;
- (void)sub:(NSUInteger)op :(NSUInteger)mode;
- (void)sub16:(NSUInteger)op :(NSUInteger)mode;
- (void)swi;
- (void)swi2;
- (void)swi3;
- (void)sync; // NOT DONE
- (void)tst:(NSUInteger)op :(NSUInteger)mode;

// Operation Helpers
- (NSUInteger)alu:(NSUInteger)value1 :(NSUInteger)value2 :(BOOL)useCarry;
- (NSUInteger)alu16:(NSUInteger)value1 :(NSUInteger)value2 :(BOOL)useCarry;
- (NSUInteger)doAdd:(NSUInteger)value :(NSUInteger)amount;
- (NSUInteger)addWithCarry:(NSUInteger)value :(NSUInteger)amount;
- (NSUInteger)subtract:(NSUInteger)value :(NSUInteger)amount;
- (NSUInteger)subtract16:(NSUInteger)value :(NSUInteger)amount;
- (NSUInteger)subWithCarry:(NSUInteger)value :(NSUInteger)amount;
- (NSUInteger)doAnd:(NSUInteger)value :(NSUInteger)amount;
- (NSUInteger)doOr:(NSUInteger)value :(NSUInteger)amount;
- (NSUInteger)doXOr:(NSUInteger)value :(NSUInteger)amount;
- (NSUInteger)arithmeticShiftRight:(NSUInteger)value;
- (NSUInteger)logicShiftRight:(NSUInteger)value;
- (NSUInteger)logicShiftLeft:(NSUInteger)value;
- (NSUInteger)rotateLeft:(NSUInteger)value;
- (NSUInteger)rotateRight:(NSUInteger)value;
- (void)setCCAfterClear;
- (void)setCCAfterLoad:(NSUInteger)value :(BOOL)is16;
- (void)setCCAfterStore:(NSUInteger)value :(BOOL)is16;
- (void)compare:(NSUInteger)value :(NSUInteger)amount;
- (NSUInteger)negate:(NSUInteger)value;
- (NSUInteger)complement:(NSUInteger)value;
- (NSUInteger)decrement:(NSUInteger)value;
- (NSUInteger)increment:(NSUInteger)value;
- (void)transferDecode:(NSUInteger)regCode :(BOOL)swapOp;
- (NSUInteger)exchange:(NSUInteger)value :(NSUInteger)regCode;
- (unsigned short)exchange16:(NSUInteger)value :(NSUInteger)regCode;
- (void)loadEffective:(NSUInteger)amount :(NSUInteger)regCode;
- (void)push:(BOOL)toHardwareStack :(NSUInteger)postbyte;
- (void)pull:(BOOL)fromHardwareStack :(NSUInteger)postbyte;
- (void)test:(NSUInteger)value;

// Indexed Addressing Helpers
- (NSUInteger)addressFromMode:(NSUInteger)mode;
- (NSUInteger)addressFromNextTwoBytes;
- (NSUInteger)addressFromDPR;
- (NSUInteger)addressFromDPR:(NSInteger)offset;
- (NSUInteger)indexedAddress:(NSUInteger)postByte;
- (NSUInteger)registerValue:(NSUInteger)sourceRegister;
- (void)incrementRegister:(unsigned short)sourceRegister :(NSInteger)amount;



// Properties: the 6809's registers
@property (assign) NSUInteger regA;
@property (assign) NSUInteger regB;
@property (assign) NSUInteger regCC;
@property (assign) NSUInteger regDP;
@property (assign) unsigned short regX;
@property (assign) unsigned short regY;
@property (assign) unsigned short regUSP;
@property (assign) unsigned short regHSP;
@property (assign) unsigned short regPC;


@end
