cmake_minimum_required(VERSION 3.13)
include(pico_sdk_import.cmake)
project(e6809 VERSION 1.0.0)
add_executable(e6809
               e6809.c)

pico_sdk_init()

pico_enable_stdio_usb(e6809 1)
pico_enable_stdio_uart(e6809 1)
pico_add_extra_outputs(e6809)

target_link_libraries(e6809
                      pico_stdlib
                      hardware_gpio
                      hardware_i2c
                      hardware_adc)
