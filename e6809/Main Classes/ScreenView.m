
//  Created by Tony Smith on 17/07/2015.
//  Copyright (c) 2015-19 Tony Smith. All rights reserved.


#import "ScreenView.h"

@implementation ScreenView



- (id)initWithFrame:(NSRect)frameRect
{
    if (self == [super initWithFrame:frameRect])
    {
        charset = [NSImage imageNamed:@"charset"];
        cpu = nil;
    }

    return self;
}



- (void)drawRect:(NSRect)dirtyRect
{
    [super drawRect:dirtyRect];

    if (cpu == nil) return;

    for (NSUInteger row = 0 ; row < 16 ; row++)
    {
        for (NSUInteger col = 0 ; col < 32 ; col++)
        {
            NSInteger a = (NSInteger)[cpu fromRam:(1024 + col + (row * 32))];
            if (a < 32) a = 32;
            NSUInteger cy = a / 32;
            NSUInteger cx = a - (32 * cy);

            [charset drawAtPoint: NSMakePoint(col * 16, 360 - (row * 24))
                        fromRect: NSMakeRect(cx * 16, 192 - (cy * 24), 16, 24)
                       operation: NSCompositingOperationCopy
                        fraction: 1.0];
        }
    }
}



- (void)setMemory:(MC6809 *)memoryObject
{
    cpu = memoryObject;
}



@end
