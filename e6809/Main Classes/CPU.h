

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
- (void)configMemory:(NSUInteger)memSizeInBytes;

// Memory Access Methods
- (NSUInteger)addressFromNextTwoBytes;
- (NSUInteger)fromRam:(NSUInteger)address;
- (void)toRam:(NSUInteger)address :(NSUInteger)value;
- (NSUInteger)addressFromDPR;
- (NSUInteger)addressFromDPR:(NSInteger)offset;
- (NSUInteger)loadFromRam;
- (void)incrementPC:(NSInteger)amount;
- (NSInteger)checkRange:(NSInteger)address;

// Utility Methods
- (void)clearBits;
- (void)decimalToBits:(NSUInteger)value;
- (void)decimal16ToBits:(NSUInteger)value;
- (NSUInteger)bitsToDecimal;
- (NSUInteger)bits16ToDecimal;
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
- (NSInteger)unsign8ToSign8:(NSInteger)value;
- (NSUInteger)alu:(NSUInteger)value1 :(NSUInteger)value2 :(BOOL)useCarry;
- (NSUInteger)alu16:(NSUInteger)value1 :(NSUInteger)value2 :(BOOL)useCarry;

// Opcode Helper Methods
- (NSUInteger)add:(NSUInteger)value :(NSUInteger)amount;
- (NSUInteger)add16bit:(NSUInteger)value :(NSUInteger)amount;
- (NSUInteger)addWithCarry:(NSUInteger)value :(NSUInteger)amount;
- (void)andcc:(NSUInteger)value;

- (NSUInteger)aShiftLeft:(NSUInteger)value;
- (NSUInteger)aShiftRight:(NSUInteger)value;

- (void)bitTest:(NSUInteger)value with:(NSUInteger)amount;
- (void)setCCAfterClear;

- (void)compare:(NSUInteger)value :(NSUInteger)amount;
- (void)compare16bit:(NSUInteger)value :(NSUInteger)amount; // TBD
- (NSUInteger)complement:(NSUInteger)value;

- (void)decimalAdjustA;
- (NSUInteger)decrement:(NSUInteger)value;

- (NSUInteger)exclusiveor:(NSUInteger)value :(NSUInteger)amount;
- (NSUInteger)exchange:(NSUInteger)value :(NSUInteger)regCode;
- (NSUInteger)exchange16bit:(NSUInteger)value :(NSUInteger)regCode;

- (NSUInteger)increment:(NSUInteger)value;

- (void)load:(NSUInteger)value :(NSUInteger)regCode;
- (void)load16bit:(NSUInteger)value;
- (void)loadEffective:(NSUInteger)amount :(NSUInteger)regCode;

- (NSUInteger)lShiftLeft:(NSUInteger)value;
- (NSUInteger)lShiftRight:(NSUInteger)value;

- (void)multiply;

- (NSUInteger)negate:(NSUInteger)value;

- (NSUInteger)orr:(NSUInteger)value :(NSUInteger)amount;
- (void)orcc:(NSUInteger)value;

- (void)push:(BOOL)toHardwareStack :(NSUInteger)postbyte;
- (void)pull:(BOOL)fromHardwareStack :(NSUInteger)postbyte;

- (NSUInteger)rotateLeft:(NSUInteger)value;
- (NSUInteger)rotateRight:(NSUInteger)value;

- (void)sex;
- (void)store:(NSUInteger)value;
- (void)store16bit:(NSUInteger)value;
- (NSUInteger)sub:(NSUInteger)value :(NSUInteger)amount;
- (NSUInteger)subWithCarry:(NSUInteger)value :(NSUInteger)amount;
- (NSUInteger)sub16bit:(NSUInteger)value :(NSUInteger)amount;
- (void)subd:(NSUInteger)amount;

- (void)test:(NSUInteger)value;
- (void)transferDecode:(NSUInteger)regCode :(BOOL)swapOp;

// Indexed Addressing Helpers
- (NSUInteger)indexedAddressing:(NSUInteger)postByte;
- (NSUInteger)registerValue:(NSUInteger)sourceRegister;
- (void)incrementRegister:(NSUInteger)sourceRegister :(NSInteger)amount;

// Processing
- (void)processNextInstruction;


// Properties: the 6809's registers
@property (assign) NSUInteger regA;
@property (assign) NSUInteger regB;
@property (assign) NSUInteger regCC;
@property (assign) NSUInteger regDP;
@property (assign) NSUInteger regX;
@property (assign) NSUInteger regY;
@property (assign) NSUInteger regUSP;
@property (assign) NSUInteger regHSP;
@property (assign) NSUInteger regPC;


@end
