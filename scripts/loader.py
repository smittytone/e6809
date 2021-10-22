#!/usr/bin/env python3

"""
Loader -- code loader for e6809 on Pico

Version:
    1.0.0

Copyright:
    2021, Tony Smith (@smittytone)

License:
    MIT (terms attached to this repo)
"""

"""
IMPORTS
"""
import os
import sys
import serial
import time


"""
GLOBALS
"""
verbose = True


"""
FUNCTIONS
"""
def get_file(file_name):
    b_a = bytearray()
    with open(file_name, "rb") as f:
        while (b := f.read(1)):
            # Do stuff with byte.
            b_a += b
    return b_a

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

"""
Show the utility version info
"""
def show_version():
    print("Loader 1.0.0 copyright (c) 2021 Tony Smith (@smittytone)")


"""
RUNTIME START
"""
if __name__ == '__main__':

    arg_flag = False
    start_address = 0x0000
    bin_file = None
    device = None

    if len(sys.argv) > 1:
        for index, item in enumerate(sys.argv):
            file_ext = os.path.splitext(item)[1]
            if arg_flag is True:
                arg_flag = False
            elif item in ("-v", "--version"):
                show_version()
                sys.exit(0)
            elif item in ("-h", "--help"):
                show_help()
                sys.exit(0)
            elif item in ("-q", "--quiet"):
                show_verbose = False
            elif item in ("-s", "--startaddress"):
                if index + 1 >= len(sys.argv):
                    print("[ERROR] -s / --startaddress must be followed by an address")
                    sys.exit(1)
                an_address = str_to_int(sys.argv[index + 1])
                if an_address is False:
                    print("[ERROR] -s / --startaddress must be followed by a valid address")
                    sys.exit(1)
                start_address = an_address
                arg_flag = True
            elif item in ("-d", "--device"):
                if index + 1 >= len(sys.argv):
                    print("[ERROR] -d / --device must be followed by a device file")
                    sys.exit(1)
                device = sys.argv[index + 1]
                arg_flag = True
            else:
                if item[0] == "-":
                    print("[ERROR] unknown option: " + item)
                    sys.exit(1)
                elif index != 0 and arg_flag is False:
                    # Handle any included .rom files
                    _, arg_file_ext = os.path.splitext(item)
                    if arg_file_ext in (".rom"):
                        bin_file = item
                    else:
                        print("[ERROR] File " + item + " is not a .rom file")

    if bin_file is None:
        print("[ERROR] No .rom file specified")
        sys.exit(1)

    if device is None:
        print("[ERROR] No e6809 device file specified")
        sys.exit(1)

    ser = None
    data_bytes = get_file(bin_file)
    length = len(data_bytes)
    send_bytes = bytearray(4)

    try:
        port = serial.Serial(port=device, baudrate=19200)
    except:
        print("[ERROR] An invalid e6809 device file was specified:",device)
        sys.exit(1)

    # Send Hail
    print("Hailing...")
    r = port.write(b'HAIL')
    time.sleep(2)
    print(r,"bytes sent")
    # Send address
    print("Start address:", start_address)
    send_bytes[0] = (start_address >> 8) & 0xFF
    send_bytes[1] = start_address & 0xFF
    #r = port.write(addr_bytes)
    #print(r,"bytes sent")
    # Send length
    send_bytes[2] = (length >> 8) & 0xFF
    send_bytes[3] = length & 0xFF
    r = port.write(send_bytes)
    print(r,"bytes sent")

    # Send data in chunks
    print("Code length:", length)
    send_bytes = bytearray(8096)
    counter = 0

    while True:
        if length - counter > 8096:
            total = 8096
        else:
            total = length - counter
            if total > 1: send_bytes = bytearray(total)

        if total < 1:
            break
        else:
            for i in range(0, total):
                send_bytes[i] = data_bytes[counter]
                counter +=1

            r = port.write(send_bytes)
            print(r,"bytes sent")
            time.sleep(0.250)
