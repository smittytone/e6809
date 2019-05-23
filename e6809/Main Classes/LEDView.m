//
//  LEDView.m
//  e6809
//
//  Created by Tony Smith on 23/05/2019.
//  Copyright © 2019 Tony Smith. All rights reserved.
//

#import "LEDView.h"

@implementation LEDView


- (void)drawRect:(NSRect)dirtyRect
{
    [super drawRect:dirtyRect];
    
    float alpha = isLightOn ? 1.0 : 0.2;
    
    NSImage *theCurrentImage = [NSImage imageNamed:@"light_full"];
    
    [theCurrentImage drawAtPoint: NSMakePoint(0.0, 0.0)
                        fromRect: NSMakeRect(0, 0, 0, 0)
                       operation: NSCompositingOperationSourceOver
                        fraction: alpha];
}



- (void)setLight:(BOOL)onOrOff
{
    // Controls the StatusLight icon's opacity: full when there is at least one project open
    // or low when there are NO projects open
    
    isLightOn = onOrOff;
    [self setNeedsDisplay:YES];
}



- (BOOL)getState
{
    return isLightOn;
}



@end
