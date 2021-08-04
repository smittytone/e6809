/*
 * e6809 for Raspberry Pi Pico
 *
 * @version     1.0.0
 * @author      smittytone
 * @copyright   2021
 * @licence     MIT
 *
 */
#include "e6809.h"


int main() {

    boot();
    return 0;
}


void boot() {
    reset_registers();
}


void loop() {

    while(1) {
        process_next_instruction()
    }
}


