//
//  LEDView.h
//  e6809
//
//  Created by Tony Smith on 23/05/2019.
//  Copyright © 2019 Tony Smith. All rights reserved.
//

#import <Cocoa/Cocoa.h>

NS_ASSUME_NONNULL_BEGIN

@interface LEDView : NSView
{
    BOOL isLightOn;
}


- (void)setLight:(BOOL)onOrOff;
- (BOOL)getState;


@end

NS_ASSUME_NONNULL_END
