
//  Created by Tony Smith on 17/09/2014.
//  Copyright (c) 2014-19 Tony Smith. All rights reserved.


#import "AppDelegate.h"

@interface AppDelegate ()

@property (weak) IBOutlet NSWindow *window;

@end

@implementation AppDelegate


#pragma mark - Initialization Methods


- (void)applicationDidFinishLaunching:(NSNotification *)aNotification
{
    cpu = [[MC6809 alloc] init];
    [vdu setMemory:cpu];
    
    memoryStartField.stringValue = @"0x0000";
    
    isPausedFlag = NO;
    isRunningFlag = NO;
    showHexFlag = YES;
    runSpeed = speedSlider.doubleValue;
    vidSpeed = 0.03;
    memStartAdddress = 0x0000;
    
    // Insert code here to initialize your application

    for (NSUInteger i = 1024 ; i < 1536 ; i++)
    {
        [cpu toRam:i :0];
    }

	// Every second, display the registers' values
    
	[NSTimer scheduledTimerWithTimeInterval:vidSpeed
									 target:self
								   selector:@selector(refreshTextScreen)
								   userInfo:nil
									repeats:YES];
    
    [self showMemoryContents:memStartAdddress];
    
    [self showRegisters];
}



#pragma mark - Application Quit Methods


- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)theApp
{
    // Return YES to quit app when user clicks on the close button
    
    return YES;
}



#pragma mark - Monitor Control Methods


- (IBAction)memDown:(id)sender
{
    // Set the memory view to a lower address (8 bytes)

    memStartAdddress -= 8;
    if (memStartAdddress < 0) memStartAdddress = 0;
    [self showMemoryContents:memStartAdddress];
}



- (IBAction)memUp:(id)sender
{
    // Set the memory view to a higher address (8 bytes)

    memStartAdddress += 8;
    if (memStartAdddress > 65423) memStartAdddress = 65423;
    [self showMemoryContents:memStartAdddress];
}



- (IBAction)setMemoryStart:(id)sender
{
    // Set the memory view to the address specified in the text field

    unsigned int val;
    NSScanner* scanner = [NSScanner scannerWithString:memoryStartField.stringValue];
    [scanner scanHexInt:&val];
    [self showMemoryContents:(NSInteger)val];
}



- (IBAction)setRegister:(id)sender
{
    // Set the specified register to the value in the text field
    // NOTE assumes decimal, but we should allow hex too

    NSInteger a = 0;
    BOOL changed = NO;
    
    if (sender == aSetButton)
    {
        a = aField.integerValue;
        if (a < -127 || a > 255) return;
        cpu.regA = a;
        changed = YES;
    }
    
    if (sender == bSetButton)
    {
        a = bField.integerValue;
        if (a < -127 || a > 255) return;
        cpu.regB = a;
        changed = YES;
    }
    
    if (sender == dpSetButton)
    {
        a = dpField.integerValue;
        if (a < -128 || a > 255) return;
        cpu.regDP = a;
        changed = YES;
    }
    
    if (sender == ccSetButton)
    {
        NSInteger a = ccField.integerValue;
        if (a < -128 || a > 255) return;
        cpu.regCC = a;
        changed = YES;
    }
    
    if (sender == xSetButton)
    {
        a = xField.integerValue;
        if (a < -32768 || a > 65535) return;
        cpu.regX = a;
        changed = YES;
    }
    
    if (sender == ySetButton)
    {
        a = yField.integerValue;
        if (a < -32768 || a > 65535) return;
        cpu.regY = a;
        changed = YES;
    }
    
    if (sender == sSetButton)
    {
        a = sField.integerValue;
        if (a < -32768 || a > 65535) return;
        cpu.regHSP = a;
        changed = YES;
    }
    
    if (sender == uSetButton)
    {
        a = uField.integerValue;
        if (a < -32768 || a > 65535) return;
        cpu.regUSP = a;
        changed = YES;
    }
    
    if (changed) [self showRegisters];
}



- (IBAction)setRegValueType:(id)sender
{
    // Manage the radio button to show either hex or decimal values
    // in the register readouts

    if (sender == regValueHexButton)
    {
        if (regValueHexButton.state == NSOnState)
        {
            regValueDecButton.state = NSOffState;
            showHexFlag = YES;
        }
    }
    else
    {
        if (regValueDecButton.state == NSOnState)
        {
            regValueHexButton.state = NSOffState;
            showHexFlag = NO;
        }
    }

    [self showRegisters];
}



- (void)resetRegisters
{
    [cpu reset];
    [self showRegisters];
}


- (IBAction)setSpeed:(id)sender
{
    double speed = speedSlider.doubleValue;
    runSpeed = speed;
}



#pragma mark - Code Execution Methods


- (IBAction)run:(id)sender
{
    if (isPausedFlag)
    {
        [self pause:self];
        return;
    }
    
    if (isRunningFlag) return;
    
    [self resetRegisters];
    [self cls];
    
    // Set the start address and stack address
    cpu.regPC = memStartAdddress;
    cpu.regHSP = 0xFFFF;
    
    // Push the PC onto the stack in case there's an RTS at the end of the code
    cpu.regHSP--;
    [cpu toRam:cpu.regHSP :(cpu.regPC & 0xFF)];
    cpu.regHSP--;
    [cpu toRam:cpu.regHSP :((cpu.regPC >> 8) & 0xFF)];
    
    // Start running
    isRunningFlag = YES;
    
    [self oneStep];
}



- (void)oneStep
{
    if (stepTimer) [stepTimer invalidate];
    
    [cpu processNextInstruction];
    [self showRegisters];
    [self showMemoryContents:memStartAdddress];
    
    stepTimer = [NSTimer scheduledTimerWithTimeInterval:runSpeed
                                                 target:self
                                               selector:@selector(oneStep)
                                               userInfo:nil
                                                repeats:NO];
}



- (IBAction)pause:(id)sender
{
    if (!isRunningFlag) return;

    if (!isPausedFlag)
    {
        if (stepTimer) [stepTimer invalidate];
        isPausedFlag = YES;
    }
    else
    {
        [self oneStep];
        isPausedFlag = NO;
    }
}



- (IBAction)step:(id)sender
{
    if (!isRunningFlag)
    {
        cpu.regPC = memStartAdddress;
        cpu.regHSP = 0xFFFF;
        
        cpu.regHSP--;
        [cpu toRam:cpu.regHSP :(cpu.regPC & 0xFF)];
        cpu.regHSP--;
        [cpu toRam:cpu.regHSP :((cpu.regPC >> 8) & 0xFF)];
        
    }
    
    isRunningFlag = YES;
    [cpu processNextInstruction];
    [self showRegisters];
    [self showMemoryContents:memStartAdddress];
}



- (IBAction)stop:(id)sender
{
    if (isRunningFlag)
    {
        if (stepTimer) [stepTimer invalidate];
        isRunningFlag = NO;
        cpu.regPC = memStartAdddress;
    }
    
    [self showRegisters];
    [self showMemoryContents:memStartAdddress];
}



- (IBAction)doReset:(id)sender
{
    [self resetRegisters];
}



#pragma mark - NSTextFieldDelegate Methods


- (BOOL)control:(NSControl *)control textView:(NSTextView *)fieldEditor doCommandBySelector:(SEL)commandSelector
{
    BOOL retval = NO;

    if (commandSelector == @selector(insertNewline:))
    {
        retval = YES;

        NSString *text = [memoryView stringValue];
        NSArray *values = [text componentsSeparatedByString:@" "];

        NSUInteger count = 0;

        for (NSUInteger i = 0 ; i < values.count ; i++)
        {
            if (i % 9 != 0)
            {
                NSString *aString = [values objectAtIndex:i];
                //NSRange hRange = [aString rangeOfString:@"0x" options:NSCaseInsensitiveSearch range:NSMakeRange(0, aString.length)];
                unsigned int val;
                NSScanner *scanner = [NSScanner scannerWithString:aString];
                [scanner scanHexInt:&val];
                [cpu toRam:(memStartAdddress + count) :val];
                count++;
            }
        }
    }

    // NSLog(@"Selector = %@", NSStringFromSelector( commandSelector ) );
    
    return retval;  
}



#pragma mark - File Handling Methods


- (IBAction)getCode:(id)sender
{
    // Open a code file
    
    if (openDialog == nil)
    {
        openDialog = [NSOpenPanel openPanel];
        openDialog.message = @"Select a .6809 code file...";
        openDialog.allowedFileTypes = [NSArray arrayWithObjects:@"6809", nil];
        openDialog.allowsMultipleSelection = NO;
        openDialog.canChooseFiles = YES;
        openDialog.canChooseDirectories = NO;
        openDialog.delegate = self;
        openDialog.accessoryView = nil;
        
        // Start off at the working directory
    
        openDialog.directoryURL = [NSURL fileURLWithPath:[@"~" stringByExpandingTildeInPath] isDirectory:YES];
    }
    
    // Run the NSOpenPanel
    
    [openDialog beginSheetModalForWindow:_window
                       completionHandler:^(NSModalResponse result) {
           [NSApp stopModal];
           [NSApp endSheet:self->openDialog];
           [self->openDialog orderOut:self];
           if (result == NSModalResponseOK) [self openFileHandler:self->openDialog.URLs];
    }
    ];
    
    [NSApp runModalForWindow:openDialog];
    [openDialog makeKeyWindow];
}



- (void)openFileHandler:(NSArray *)urls
{
    // Open the selected file, read in the one or more chunks, and poke the data contents
    // into emulator RAM. Set the monitor to display the start address of the first chunk

    if (urls.count > 0)
    {
        NSFileManager *nsfm = NSFileManager.defaultManager;
        
        NSURL *url = [urls objectAtIndex:0];
        NSString *filePath = [url path];
        NSData *fileData = [nsfm contentsAtPath:filePath];
        
        if (fileData != nil && fileData.length > 0)
        {
            NSError *dataDecodeError = nil;
            id parsedData = [NSJSONSerialization JSONObjectWithData:fileData options:kNilOptions error:&dataDecodeError];
            
            if (dataDecodeError == nil && parsedData != nil)
            {
                NSArray *dataArray = (NSArray *)parsedData;
                NSString *startAddress = @"";

                for (NSDictionary *dict in dataArray)
                {
                    if (startAddress.length == 0) startAddress = [dict valueForKey:@"address"];
                    NSString *strAddress = [dict valueForKey:@"address"];
                    NSUInteger address = [strAddress intValue];
                    NSString *code = [dict valueForKey:@"code"];
                    NSScanner *scanner = nil;

                    for (NSUInteger i = 0 ; i < code.length ; i += 2)
                    {
                        // Get a pair of hex characters and convert to an integer, then
                        // poke the value into the emulator RAM

                        NSString *hexChars = [code substringWithRange:NSMakeRange(i, 2)];
                        unsigned int value;
                        scanner = [NSScanner scannerWithString:hexChars];
                        [scanner scanHexInt:&value];
                        [cpu toRam:address :(NSUInteger)value];
                        address++;
                    }
                }

                // Set the monitor to show memory at the start of the first chunk

                [self showMemoryContents:[startAddress intValue]];
            }
        }
    }
}



#pragma mark - Display Update Methods


- (void)refreshTextScreen
{
    // Force the VDU to update - this is called by the timer set up in
    // 'applicationDidFinishLaunching:'

    [vdu setNeedsDisplay:YES];
}



- (void)showMemoryContents:(NSInteger)startAddress
{
    // Update the memory readout

    NSString *string = @"";
    NSUInteger endAddress = (startAddress + 112 > 65535) ? 65423 : (startAddress + 112);

    for (NSUInteger i = startAddress ; i < endAddress ; i += 8)
    {
        string = [string stringByAppendingFormat:@"[%04X] ", (unsigned int)i];

        for (NSUInteger j = 0 ; j < 8 ; j++)
        {
            unsigned char value = [cpu fromRam:(i + j)];
            string = [string stringByAppendingFormat:@"%02X ", value];
        }

        string = [string stringByAppendingString:@"\n"];
    }

    [memoryView setStringValue:string];
    memStartAdddress = startAddress;
}



- (void)showRegisters
{
    // Update the Register value fields with the current values
    // Display hex or decimal according to the radio selection

    aField.stringValue = showHexFlag ? [NSString stringWithFormat:@"%02lX", (unsigned long)cpu.regA] : [NSString stringWithFormat:@"%lu", (unsigned long)cpu.regA];
    bField.stringValue = showHexFlag ? [NSString stringWithFormat:@"%02lX", (unsigned long)cpu.regB] : [NSString stringWithFormat:@"%lu", (unsigned long)cpu.regB];
    dpField.stringValue = showHexFlag ? [NSString stringWithFormat:@"%02lX", (unsigned long)cpu.regDP] : [NSString stringWithFormat:@"%lu", (unsigned long)cpu.regDP];
    ccField.stringValue = showHexFlag ? [NSString stringWithFormat:@"%02lX", (unsigned long)cpu.regCC] : [NSString stringWithFormat:@"%lu", (unsigned long)cpu.regCC];

    xField.stringValue = showHexFlag ? [NSString stringWithFormat:@"%04lX", (unsigned long)cpu.regX] : [NSString stringWithFormat:@"%lu", (unsigned long)cpu.regX];
    yField.stringValue = showHexFlag ? [NSString stringWithFormat:@"%04lX", (unsigned long)cpu.regY] : [NSString stringWithFormat:@"%lu", (unsigned long)cpu.regY];
    uField.stringValue = showHexFlag ? [NSString stringWithFormat:@"%04lX", (unsigned long)cpu.regUSP] : [NSString stringWithFormat:@"%lu", (unsigned long)cpu.regUSP];
    sField.stringValue = showHexFlag ? [NSString stringWithFormat:@"%04lX", (unsigned long)cpu.regHSP] : [NSString stringWithFormat:@"%lu", (unsigned long)cpu.regHSP];
    pcField.stringValue = showHexFlag ? [NSString stringWithFormat:@"%04lX", (unsigned long)cpu.regPC] : [NSString stringWithFormat:@"%lu", (unsigned long)cpu.regPC];
    
    if ([cpu bitSet:cpu.regCC :kCC_e] != [cceField getState]) [cceField setLight:[cpu bitSet:cpu.regCC :kCC_e]];
    if ([cpu bitSet:cpu.regCC :kCC_f] != [ccfField getState]) [ccfField setLight:[cpu bitSet:cpu.regCC :kCC_f]];
    if ([cpu bitSet:cpu.regCC :kCC_h] != [cchField getState]) [cchField setLight:[cpu bitSet:cpu.regCC :kCC_h]];
    if ([cpu bitSet:cpu.regCC :kCC_i] != [cciField getState]) [cciField setLight:[cpu bitSet:cpu.regCC :kCC_i]];
    if ([cpu bitSet:cpu.regCC :kCC_n] != [ccnField getState]) [ccnField setLight:[cpu bitSet:cpu.regCC :kCC_n]];
    if ([cpu bitSet:cpu.regCC :kCC_z] != [cczField getState]) [cczField setLight:[cpu bitSet:cpu.regCC :kCC_z]];
    if ([cpu bitSet:cpu.regCC :kCC_v] != [ccvField getState]) [ccvField setLight:[cpu bitSet:cpu.regCC :kCC_v]];
    if ([cpu bitSet:cpu.regCC :kCC_c] != [cccField getState]) [cccField setLight:[cpu bitSet:cpu.regCC :kCC_c]];

    [vdu setNeedsDisplayInRect:(NSMakeRect(0, 0, 512, 384))];
}



- (void)cls
{
    if (isRunningFlag) return;
    
    for (NSUInteger i = 0x0400 ; i < 0x0600 ; i++)
    {
        [cpu toRam:i :0x00];
    }
    
    [vdu setNeedsDisplayInRect:(NSMakeRect(0, 0, 512, 384))];
}



@end
