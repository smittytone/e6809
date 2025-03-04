#!/usr/bin/env python3

'''
Loader -- code loader for e6809 on Pico

Version:
    1.0.0

Copyright:
    2021, Tony Smith (@smittytone)

License:
    MIT (terms attached to this repo)
'''

'''
IMPORTS
'''
from os import path
from sys import exit, argv
from time import time_ns


'''
GLOBALS
'''
verbose = True


'''
FUNCTIONS
'''

'''
Load a .rom file into memory.

Args:
    file (String): The .rom's path and filename.

Returns:
    Bytearray: The loaded byte data.
'''
def get_file(file):
    ba = bytearray()
    with open(file, "rb") as f:
        while (b := f.read(1)):
            ba += b
    return ba


'''
Trigger a check for an ACK from the Monitor Board, or
close the UART and exit.

Args:
    uart (Serial): The chosen serial port.
'''
def await_ack_or_exit(uart):
    r = await_ack(uart)
    if r == False:
        uart.close()
        print("[ERROR] No ACK")
        exit(1)


'''
Sample the UART for an ACK response.

Args:
    uart (Serial): The chosen serial port.
    timeout (Int): The sampling timeout. Default: 2s.

Returns:
    int: True if the last block as ACK’d, False otherwise.
'''
def await_ack(uart, timeout=2000):
    buffer = bytes()
    now = (time_ns() // 1000000)
    while ((time_ns() // 1000000) - now) < timeout:
        if uart.in_waiting > 0:
            buffer += uart.read(uart.in_waiting)
            if "\n" in buffer.decode():
                show_verbose("RX: " + buffer[:-1].decode())
                return True
    # Error -- No Ack received
    return False


'''
Send a single address block. This is the code's
16-bit entry point.

Args:
    uart (Serial): The chosen serial port.
    address (Int): The 16-bit address.
'''
def send_addr_block(uart, address):
    out = bytearray(8)
    out[0] = 0x55                   # Head
    out[1] = 0x3C                   # Sync
    out[2] = 0x00                   # Block Type
    out[3] = 2                      # Data length
    out[4] = (address >> 8) & 0xFF
    out[5] = address & 0xFF

    # Compute checksum
    cs = 0
    for i in range (2, 6):
        cs += out[i]

    out[6] = cs & 0xFF              # Checksum
    out[7] = 0x55                   # Trailer
    r = uart.write(out)


'''
Send a single data block.

Args:
    uart (Serial):     The chosen serial port.
    bytes (Bytearray): The data store.
    counter (Int):     The index of the first byte to send.

Returns:
    Int: The updated byte index.
 '''
def send_data_block(uart, bytes, counter):
    length = len(bytes) - counter
    if length > 255: length = 255
    out = bytearray(length + 6)
    out[0] = 0x55                   # Head
    out[1] = 0x3C                   # Sync
    out[2] = 0x01                   # Block Type
    out[3] = length                 # Data length

    # Set the data
    for i in range(0, length):
        out[i + 4] = bytes[counter + i]

    # Compute checksum
    cs = 0
    for i in range (2, length + 6 - 2):
        cs += out[i]
    cs &= 0xFF
    out[length + 6 - 2] = cs        # Checksum
    out[length + 6 - 1] = 0x55      # Trailer
    counter += length
    r = uart.write(out)
    return counter


'''
Send a single end-of-file block.

Args:
    uart (Serial):     The chosen serial port.
 '''
def send_trailer_block(uart):
    out = b'\x55\x3C\xFF\x00\x00\x55'
    r = uart.write(out)


'''
Display a message if verbose mode is enabled.

Args:
    messsage (str): The text to print.
'''
def show_verbose(message):
    if verbose is True: print(message)


'''
Pass on all supplied '.asm' files on for assembly, '.6809' or '.rom' files for disassembly.

Args:
    the_files (list): The .asm, .rom or .6809 files.

Returns:
    int: The numerical value
'''
def str_to_int(num_str):
    num_base = 10
    if num_str[0] == "$": num_str = "0x" + num_str[1:]
    if num_str[:2] == "0x": num_base = 16
    try:
        return int(num_str, num_base)
    except ValueError:
        return False


'''
Show the utility help
'''
def show_help():
    show_version()
    print("\nTransfer binary data to the 6809e Monitor Board.\n")
    print("Usage:\n\n  loader.py [-s] [-d] [-q] [-h] <rom_file>\n")
    print("Options:\n")
    print("  -s / --start    Code 16-bit start address. Default: 0x0000.")
    print("  -d / --device   The Monitor Board USB-serial device file.")
    print("  -q / --quiet    Quiet output -- no messages other than errors.")
    print("  -h / --help     This help information.")
    print()


'''
Show the utility version info
'''
def show_version():
    print("Loader 1.0.0 copyright (c) 2021 Tony Smith (@smittytone)")


'''
RUNTIME START
'''
if __name__ == '__main__':

    arg_flag = False
    start_address = 0x0000
    rom_file = None
    device = None

    if len(argv) > 1:
        for index, item in enumerate(argv):
            file_ext = path.splitext(item)[1]
            if arg_flag is True:
                arg_flag = False
            elif item in ("-h", "--help"):
                show_help()
                exit(0)
            elif item in ("-q", "--quiet"):
                verbose = False
            elif item in ("-s", "--startaddress"):
                if index + 1 >= len(argv):
                    print("[ERROR] -s / --startaddress must be followed by an address")
                    exit(1)
                an_address = str_to_int(argv[index + 1])
                if an_address is False or an_address < 0 or an_address > 0xFFFF:
                    print("[ERROR] -s / --startaddress must be followed by a valid address")
                    exit(1)
                start_address = an_address
                arg_flag = True
            elif item in ("-d", "--device"):
                if index + 1 >= len(argv):
                    print("[ERROR] -d / --device must be followed by a device file")
                    exit(1)
                device = argv[index + 1]
                arg_flag = True
            else:
                if item[0] == "-":
                    print("[ERROR] unknown option: " + item)
                    exit(1)
                elif index != 0 and arg_flag is False:
                    # Handle any included .rom files
                    if not path.exists(item):
                        print("[ERROR] File " + item + " does not exist")
                        exit(1)
                    _, arg_file_ext = path.splitext(item)
                    if arg_file_ext in (".rom"):
                        rom_file = item
                    else:
                        print("[ERROR] File " + item + " is not a .rom file")

    if rom_file is None:
        print("[ERROR] No .rom file specified")
        exit(1)

    if device is None:
        print("[ERROR] No e6809 device file specified")
        exit(1)

    # Set the port or fail
    port = None
    try:
        import serial
        port = serial.Serial(port=device, baudrate=115200)
    except:
        print("[ERROR] An invalid e6809 device file was specified:",device)
        exit(1)

    # Load the data
    data_bytes = get_file(rom_file)
    length = len(data_bytes)
    show_verbose("Code start adddress set to 0x{:04X}".format(start_address))
    show_verbose(str(length) + " bytes to send")

    # Send the header
    send_addr_block(port, start_address)
    await_ack_or_exit(port)

    # Send the data out block by block
    c = 0
    while True:
        c = send_data_block(port, data_bytes, c)
        await_ack_or_exit(port)
        if length - c <= 0: break

    # Send the trailer
    send_trailer_block(port)
    await_ack_or_exit(port)

    # Close the port
    port.close()
