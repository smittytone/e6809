
//  Created by Tony Smith on 13/02/2015.
//  Copyright (c) 2015 Tony Smith. All rights reserved.


#import "RAM.h"


@implementation RAM

@synthesize topAddress;


- (BOOL)bootRam:(NSUInteger)anAddress
{
	if (anAddress > 0xFFFF) return NO;
	for (NSUInteger i = 0 ; i < anAddress + 1 ; i++) memoryMap[i] = opcode_NOP;
	topAddress = anAddress;
	return YES;
}



- (NSUInteger)peek:(NSUInteger)address
{
	// Handle RAM rollover
    if (address > topAddress) address -= topAddress;
	return memoryMap[address];
}



- (void)poke:(NSUInteger)address :(NSUInteger)value
{
	// Handle RAM rollover
	if (address > topAddress) address -= topAddress;
	memoryMap[address] = value;
}



@end
