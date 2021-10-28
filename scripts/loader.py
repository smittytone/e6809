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


def await_ack_or_exit(uart):
    #print("Awaiting ACK")
    r = await_ack(uart)
    if r == False:
        uart.close()
        print("No ACK")
        sys.exit(1)


def await_ack(uart, timeout=2000):
    buffer = bytes()
    now = (time.time_ns() // 1000000)
    while ((time.time_ns() // 1000000) - now) < timeout:
        if uart.in_waiting > 0:
            buffer += uart.read(uart.in_waiting)
            if "\n" in buffer.decode():
                print(buffer[:-1].decode())
                return True
    return False


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
        #print("{:02X} ".format(out[i + 4]), end="")

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


def send_trailer(uart):
    out = bytearray(6)
    out[0] = 0x55                   # Head
    out[1] = 0x3C                   # Sync
    out[2] = 0xFF                   # Block Type
    out[3] = 0x00                   # Data length
    out[4] = 0x00                   # Checksum
    out[5] = 0x55                   # Trailer
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

    port = None
    try:
        port = serial.Serial(port=device, baudrate=115200)
    except:
        print("[ERROR] An invalid e6809 device file was specified:",device)
        sys.exit(1)

    data_bytes = get_file(bin_file)
    length = len(data_bytes)
    print(length,"bytes to send")

    # Send the header
    send_addr_block(port, start_address)
    await_ack_or_exit(port)

    # Send the data
    c = 0
    while True:
        #print(c,"/",length - c)
        c = send_data_block(port, data_bytes, c)
        await_ack_or_exit(port)
        if length - c <= 0: break

    # Send the trailer
    send_trailer(port)
    await_ack_or_exit(port)
    port.close()