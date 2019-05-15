
//  Created by Tony Smith on 17/09/2014.
//  Copyright (c) 2014 Tony Smith. All rights reserved.


#import "AppDelegate.h"

@interface AppDelegate ()

@property (weak) IBOutlet NSWindow *window;
@end

@implementation AppDelegate

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification
{

	unsigned char a, b;

	cpu = [[MC6809 alloc] init];
    [vdu setMemory:cpu];
    [self showMemoryContents:0x0000];
    memoryStartField.stringValue = @"0x0000";
    
	a =  0x00;
	b = 0x42;
	cpu.regA = a;
	cpu.regB = b;
	[cpu transferDecode:0x89 :YES];
    
    NSLog(@"A: %u DP: %u A: %u DP: %u" , a, b, cpu.regA, cpu.regB);

    a = [cpu add:0x40 :0x40];
    if ([cpu bitSet:cpu.regCC :kCC_c]) { NSLog(@"Carry set"); } else { NSLog(@"Carry not set"); }
    if ([cpu bitSet:cpu.regCC :kCC_v]) { NSLog(@"Overflow set"); } else { NSLog(@"Overflow not set"); }

    isPausedFlag = NO;
    isRunningFlag = NO;
    
    // Insert code here to initialize your application

    for (NSUInteger i = 1024 ; i < 1536 ; i++)
    {
        [cpu setContentsOfMemory:i :0];
    }

	// Every second, display the registers' values

	[NSTimer scheduledTimerWithTimeInterval:1.0
									 target:self
								   selector:@selector(showRegisters)
								   userInfo:nil
									repeats:YES];


    NSInteger testOffset = 5;
    NSUInteger testAddress = 0xFFFF;
    testAddress = testAddress + testOffset;
    testAddress = testAddress & 0xFFFF;
    NSLog(@"%lu", (unsigned long)testAddress);

    cpu.regPC = 0xFFFF;
    NSUInteger l = [cpu loadFromRam];
    NSLog(@"%lu", (unsigned long)cpu.regPC);

    UInt8 testy = 0xFF;
    testy++;
    NSLog(@"%hhu", testy);
}



- (void)applicationWillTerminate:(NSNotification *)aNotification {
    // Insert code here to tear down your application
}



- (void)showMemoryContents:(NSInteger)startAddress
{
    NSString *string = @"";
    NSInteger endAddress;
    
    if (startAddress + 112 > 65535)
    {
        endAddress = 65423;
    }
    else
    {
        endAddress = startAddress + 112;
    }
    
    for (NSInteger i = startAddress ; i < endAddress ; i = i + 8)
    {
        string = [string stringByAppendingFormat:@"[0x%04X] ", (unsigned int)i];
                  
        for (NSInteger j = 0 ; j < 8 ; j++)
        {
            unsigned char value = [cpu contentsOfMemory:(short)i + j];
            string = [string stringByAppendingFormat:@"0x%02X ", value];
        }
        string = [string stringByAppendingString:@"\n"];
    }
    
    [memoryView setStringValue:string];
    memStartAdddress = startAddress;
}


- (IBAction)memDown:(id)sender
{
    memStartAdddress = memStartAdddress - 8;
    if (memStartAdddress < 0) memStartAdddress = 0;
    [self showMemoryContents:memStartAdddress];
}




- (IBAction)memUp:(id)sender
{
    memStartAdddress = memStartAdddress + 8;
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
    if (sender == aSetButton)
    {
        NSInteger a = aField.integerValue;
        if (a < -127 || a > 255) return;
    }
}



- (void)showRegisters
{
	aField.stringValue = [NSString stringWithFormat:@"%u", cpu.regA];
	bField.stringValue = [NSString stringWithFormat:@"%u", cpu.regB];
	dpField.stringValue = [NSString stringWithFormat:@"%u", cpu.regDP];
    ccField.stringValue = [NSString stringWithFormat:@"%lu", (unsigned long)cpu.regCC];

	xField.stringValue = [NSString stringWithFormat:@"%u", cpu.regX];
	yField.stringValue = [NSString stringWithFormat:@"%u", cpu.regY];
	uField.stringValue = [NSString stringWithFormat:@"%u", cpu.regUSP];
	sField.stringValue = [NSString stringWithFormat:@"%u", cpu.regHSP];
	pcField.stringValue = [NSString stringWithFormat:@"%u", cpu.regPC];

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

    cpu.regPC = memStartAdddress;
    isRunningFlag = YES;
    [self oneStep];
}



- (void)oneStep
{
    if (stepTimer) [stepTimer invalidate];
    [cpu processNextInstruction];
    stepTimer = [NSTimer scheduledTimerWithTimeInterval:0.25 target:self selector:@selector(oneStep) userInfo:nil repeats:NO];
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
    if (!isRunningFlag) return;

    [cpu processNextInstruction];
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
                NSRange hRange;
                NSString *aString = [values objectAtIndex:i];
                hRange = [aString rangeOfString:@"0x" options:NSCaseInsensitiveSearch range:NSMakeRange(0, aString.length)];
                if (hRange.location != NSNotFound)
                {
                    unsigned int val;
                    NSScanner* scanner = [NSScanner scannerWithString:aString];
                    [scanner scanHexInt:&val];
                    [cpu setContentsOfMemory:(unsigned short)(memStartAdddress + count) :(unsigned char)val];
                }
                else
                {
                    [cpu setContentsOfMemory:(unsigned short)(memStartAdddress + count) :(unsigned char)[aString integerValue]];
                }
                count++;
            }
        }
    }

    // NSLog(@"Selector = %@", NSStringFromSelector( commandSelector ) );
    
    return retval;  
}


@end
