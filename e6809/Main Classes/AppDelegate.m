
//  Created by Tony Smith on 17/09/2014.
//  Copyright (c) 2014-19 Tony Smith. All rights reserved.


#import "AppDelegate.h"

@interface AppDelegate ()

@property (weak) IBOutlet NSWindow *window;

@end

@implementation AppDelegate


- (void)applicationDidFinishLaunching:(NSNotification *)aNotification
{
    cpu = [[MC6809 alloc] init];
    [vdu setMemory:cpu];
    
    memoryStartField.stringValue = @"0x0000";
    
    isPausedFlag = NO;
    isRunningFlag = NO;
    showHexFlag = YES;
    runSpeed = 0.001;
    vidSpeed = 0.03;
    
    // Insert code here to initialize your application

    for (NSUInteger i = 1024 ; i < 1536 ; i++)
    {
        [cpu toRam:i :0];
    }

	// Every second, display the registers' values
    
	[NSTimer scheduledTimerWithTimeInterval:vidSpeed
									 target:self
								   selector:@selector(doTextScreen)
								   userInfo:nil
									repeats:YES];
    
    [self showMemoryContents:0x0000];
    [self showRegisters];
}



- (void)applicationWillTerminate:(NSNotification *)aNotification {
    // Insert code here to tear down your application
}



- (void)showMemoryContents:(NSInteger)startAddress
{
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



- (IBAction)memDown:(id)sender
{
    memStartAdddress -= 8;
    if (memStartAdddress < 0) memStartAdddress = 0;
    [self showMemoryContents:memStartAdddress];
}



- (IBAction)memUp:(id)sender
{
    memStartAdddress += 8;
    if (memStartAdddress > 65423) memStartAdddress = 65423;
    [self showMemoryContents:memStartAdddress];
}



- (IBAction)setMemoryStart:(id)sender
{
    unsigned int val;
    NSScanner* scanner = [NSScanner scannerWithString:memoryStartField.stringValue];
    [scanner scanHexInt:&val];
    [self showMemoryContents:(NSInteger)val];
}



- (IBAction)setRegister:(id)sender
{
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



- (void)showRegisters
{
    aField.stringValue = showHexFlag ? [NSString stringWithFormat:@"%02lX", (unsigned long)cpu.regA] : [NSString stringWithFormat:@"%lu", (unsigned long)cpu.regA];
    bField.stringValue = showHexFlag ? [NSString stringWithFormat:@"%02lX", (unsigned long)cpu.regB] : [NSString stringWithFormat:@"%lu", (unsigned long)cpu.regB];
    dpField.stringValue = showHexFlag ? [NSString stringWithFormat:@"%02lX", (unsigned long)cpu.regDP] : [NSString stringWithFormat:@"%lu", (unsigned long)cpu.regDP];
    ccField.stringValue = showHexFlag ? [NSString stringWithFormat:@"%02lX", (unsigned long)cpu.regCC] : [NSString stringWithFormat:@"%lu", (unsigned long)cpu.regCC];

	xField.stringValue = showHexFlag ? [NSString stringWithFormat:@"%04lX", (unsigned long)cpu.regX] : [NSString stringWithFormat:@"%lu", (unsigned long)cpu.regX];
	yField.stringValue = showHexFlag ? [NSString stringWithFormat:@"%04lX", (unsigned long)cpu.regY] : [NSString stringWithFormat:@"%lu", (unsigned long)cpu.regY];
	uField.stringValue = showHexFlag ? [NSString stringWithFormat:@"%04lX", (unsigned long)cpu.regUSP] : [NSString stringWithFormat:@"%lu", (unsigned long)cpu.regUSP];
	sField.stringValue = showHexFlag ? [NSString stringWithFormat:@"%04lX", (unsigned long)cpu.regHSP] : [NSString stringWithFormat:@"%lu", (unsigned long)cpu.regHSP];
	pcField.stringValue = showHexFlag ? [NSString stringWithFormat:@"%04lX", (unsigned long)cpu.regPC] : [NSString stringWithFormat:@"%lu", (unsigned long)cpu.regPC];

    if ([cpu bitSet:cpu.regCC :kCC_e]) { cceField.stringValue = @"1"; } else { cceField.stringValue = @"0"; }
	if ([cpu bitSet:cpu.regCC :kCC_f]) { ccfField.stringValue = @"1"; } else { ccfField.stringValue = @"0"; }
	if ([cpu bitSet:cpu.regCC :kCC_h]) { cchField.stringValue = @"1"; } else { cchField.stringValue = @"0"; }
	if ([cpu bitSet:cpu.regCC :kCC_i]) { cciField.stringValue = @"1"; } else { cciField.stringValue = @"0"; }
	if ([cpu bitSet:cpu.regCC :kCC_n]) { ccnField.stringValue = @"1"; } else { ccnField.stringValue = @"0"; }
	if ([cpu bitSet:cpu.regCC :kCC_z]) { cczField.stringValue = @"1"; } else { cczField.stringValue = @"0"; }
	if ([cpu bitSet:cpu.regCC :kCC_v]) { ccvField.stringValue = @"1"; } else { ccvField.stringValue = @"0"; }
	if ([cpu bitSet:cpu.regCC :kCC_c]) { cccField.stringValue = @"1"; } else { cccField.stringValue = @"0"; }

    [vdu setNeedsDisplayInRect:(NSMakeRect(0, 0, 512, 384))];
}


- (IBAction)run:(id)sender
{
    if (isRunningFlag) return;
    
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



- (IBAction)setRegValueType:(id)sender
{
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



- (IBAction)getCode:(id)sender
{
    // Open a code file
    
    if (openDialog == nil)
    {
        openDialog = [NSOpenPanel openPanel];
        openDialog.message = @"Select a 6809 code file...";
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
                           [self openFileHandler:result];
    }
     ];
    
    [NSApp runModalForWindow:openDialog];
    [openDialog makeKeyWindow];
}


- (void)openFileHandler:(NSInteger)result
{
    // This is where we open/add all files selected by the open file dialog
    // Multiple files may be passed in, as an array of URLs, and they will contain either
    // project files OR source code files (as specified by 'openActionType'
    
    
    
    if (result == NSModalResponseOK)
    {
        NSArray *urls = openDialog.URLs;
        
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
                    NSDictionary *dict = (NSDictionary *)parsedData;
                    NSString *strAddress = [dict valueForKey:@"address"];
                    NSUInteger address = [strAddress intValue];
                    NSString *code = [dict valueForKey:@"code"];
                    NSScanner *scanner = nil;
                    
                    //const char *ccode = code.UTF8String;
                    
                    for (NSUInteger i = 0 ; i < code.length ; i += 2)
                    {
                        // Poke in the code
                        
                        NSString *hexChars = [code substringWithRange:NSMakeRange(i, 2)];
                        unsigned int value;
                        scanner = [NSScanner scannerWithString:hexChars];
                        [scanner scanHexInt:&value];
                        [cpu toRam:address :(NSUInteger)value];
                        address++;
                    }
                    
                    [self showMemoryContents:[strAddress intValue]];
                }
            }
        }
        
        openDialog = nil;
    }
}


- (void)doTextScreen
{
    [vdu setNeedsDisplay:YES];
}


@end
