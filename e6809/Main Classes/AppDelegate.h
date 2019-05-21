//
//  AppDelegate.h
//  e6809
//
//  Created by Tony Smith on 17/09/2014.
//  Copyright (c) 2014-19 Tony Smith. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "CPU.h"
#import "ScreenView.h"


@interface AppDelegate : NSObject  <NSApplicationDelegate,
                                    NSTextFieldDelegate,
                                    NSOpenSavePanelDelegate>

{
    IBOutlet NSTextField    *aField;
    IBOutlet NSTextField    *bField;
    IBOutlet NSTextField    *dpField;
    IBOutlet NSTextField    *ccField;
    IBOutlet NSTextField    *xField;
    IBOutlet NSTextField    *yField;
    IBOutlet NSTextField    *uField;
    IBOutlet NSTextField    *sField;
    IBOutlet NSTextField    *pcField;
    
    IBOutlet NSButton       *aSetButton;
    IBOutlet NSButton       *bSetButton;
    IBOutlet NSButton       *dpSetButton;
    IBOutlet NSButton       *ccSetButton;
    IBOutlet NSButton       *xSetButton;
    IBOutlet NSButton       *ySetButton;
    IBOutlet NSButton       *uSetButton;
    IBOutlet NSButton       *sSetButton;
    IBOutlet NSButton       *pcSetButton;

	IBOutlet NSTextField    *cceField;
	IBOutlet NSTextField    *ccfField;
	IBOutlet NSTextField    *cchField;
	IBOutlet NSTextField    *cciField;
	IBOutlet NSTextField    *ccnField;
	IBOutlet NSTextField    *cczField;
	IBOutlet NSTextField    *ccvField;
	IBOutlet NSTextField    *cccField;

    IBOutlet NSTextField    *memoryView;
    IBOutlet NSTextField    *memoryStartField;
    
    IBOutlet NSButton       *regValueHexButton;
    IBOutlet NSButton       *regValueDecButton;

    IBOutlet ScreenView *vdu;
    
    BOOL isPausedFlag, isRunningFlag, showHexFlag;
    NSTimer *stepTimer;
    
    MC6809 *cpu;

    NSInteger memStartAdddress;
    double runSpeed, vidSpeed;
    NSOpenPanel *openDialog;
}


- (IBAction)setRegister:(id)sender;
- (IBAction)setMemoryStart:(id)sender;
- (void)showRegisters;
- (IBAction)memUp:(id)sender;
- (IBAction)memDown:(id)sender;

- (IBAction)run:(id)sender;
- (IBAction)pause:(id)sender;
- (IBAction)step:(id)sender;
- (IBAction)stop:(id)sender;
- (void)oneStep;
- (IBAction)getCode:(id)sender;
- (IBAction)setRegValueType:(id)sender;

@end
