//
//  AppDelegate.h
//  e6809
//
//  Created by Tony Smith on 17/09/2014.
//  Copyright (c) 2014 Tony Smith. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "CPU.h"
#import "ScreenView.h"


@interface AppDelegate : NSObject <NSApplicationDelegate, NSTextFieldDelegate>

{
    IBOutlet NSTextField *aField;
    IBOutlet NSTextField *bField;
    IBOutlet NSTextField *dpField;
    IBOutlet NSTextField *ccField;
    IBOutlet NSTextField *xField;
    IBOutlet NSTextField *yField;
    IBOutlet NSTextField *uField;
    IBOutlet NSTextField *sField;
    IBOutlet NSTextField *pcField;
    
    IBOutlet NSButton *aSetButton;
    IBOutlet NSButton *bSetButton;
    IBOutlet NSButton *dpSetButton;
    IBOutlet NSButton *ccSetButton;
    IBOutlet NSButton *xSetButton;
    IBOutlet NSButton *ySetButton;
    IBOutlet NSButton *uSetButton;
    IBOutlet NSButton *sSetButton;
    IBOutlet NSButton *pcSetButton;

	IBOutlet NSTextField *cceField;
	IBOutlet NSTextField *ccfField;
	IBOutlet NSTextField *cchField;
	IBOutlet NSTextField *cciField;
	IBOutlet NSTextField *ccnField;
	IBOutlet NSTextField *cczField;
	IBOutlet NSTextField *ccvField;
	IBOutlet NSTextField *cccField;

    IBOutlet NSTextField *memoryView;
    IBOutlet NSTextField *memoryStartField;

    IBOutlet ScreenView *vdu;
    
    BOOL isPausedFlag, isRunningFlag;
    NSTimer *stepTimer;
    
    MC6809 *cpu;

    NSInteger memStartAdddress;
}


- (IBAction)setRegister:(id)sender;
- (IBAction)setMemoryStart:(id)sender;
- (void)showRegisters;
- (IBAction)memUp:(id)sender;
- (IBAction)memDown:(id)sender;

- (IBAction)run:(id)sender;
- (IBAction)pause:(id)sender;
- (IBAction)step:(id)sender;
- (void)oneStep;


@end