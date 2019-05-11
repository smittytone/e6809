
//  Created by Tony Smith on 13/02/2015.
//  Copyright (c) 2015 Tony Smith. All rights reserved.


#import <Foundation/Foundation.h>
#import "Constants.h"


@interface RAM : NSObject
{
	NSUInteger memoryMap[65536];
}


- (BOOL)bootRam:(NSUInteger)anAddress;
- (NSUInteger)peek:(NSUInteger)address;
- (void)poke:(NSUInteger)address :(NSUInteger)value;


@property (assign, readwrite) NSUInteger topAddress;


@end
