//
//  ScreenView.h
//  e6809
//
//  Created by Tony Smith on 17/07/2015.
//  Copyright (c) 2015-19 Tony Smith. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#include "CPU.h"

@interface ScreenView : NSView

{
    NSImage *charset;
    MC6809 *cpu;
}


- (void)setMemory:(MC6809 *)memoryObject;

@end
