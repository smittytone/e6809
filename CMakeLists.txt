cmake_minimum_required(VERSION 3.14)

# Set project data
set(PROJECT_NAME "e6809")
set(VERSION_NUMBER "0.0.2")
set(BUILD_NUMBER "2")

# Make project data accessible to compiler
add_compile_definitions(APP_NAME="${PROJECT_NAME}")
add_compile_definitions(APP_VERSION="${VERSION_NUMBER}")
add_compile_definitions(BUILD_NUM=${BUILD_NUMBER})
add_compile_definitions(DEBUG=1)

include(pico_sdk_import.cmake)

project(${PROJECT_NAME}
        LANGUAGES C CXX ASM
        VERSION 0.0.2
        DESCRIPTION "Motorola 6809e simulator")

add_executable(${PROJECT_NAME}
    source/main.c
    source/cpu.c
    source/cpu_tests.c
    source/ht16k33.c
    source/keypad.c
    source/monitor.c
    source/pia.c
)

pico_sdk_init()

target_link_libraries(${PROJECT_NAME}
    pico_stdlib
    hardware_gpio
    hardware_i2c
    hardware_spi
    hardware_adc
    hardware_flash
    hardware_sync
)

pico_enable_stdio_usb(${PROJECT_NAME} 1)
pico_enable_stdio_uart(${PROJECT_NAME} 0)
pico_add_extra_outputs(${PROJECT_NAME})
