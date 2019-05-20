

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


- (void)transferDecode:(NSUInteger)regCode :(BOOL)swapOp;

// Indexed Addressing Helpers
- (NSUInteger)indexedAddressing:(NSUInteger)postByte;
- (NSUInteger)registerValue:(NSUInteger)sourceRegister;
- (void)incrementRegister:(NSUInteger)sourceRegister :(NSInteger)amount;

// Processing
- (void)processNextInstruction;
- (void)doBranch:(NSUInteger)opcode :(BOOL)isLong;

// Operations
- (void)sex;


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
