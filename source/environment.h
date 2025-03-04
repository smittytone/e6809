/*
 * e6809 for Raspberry Pi Pico
 *
 * @version     0.0.2
 * @author      smittytone
 * @copyright   2025
 * @licence     MIT
 *
 */


enum Memory_Map_Band_Type {
    Ram_User = 0,
    Ram_System,
    Rom_System,
    Rom_Cartridge,
    None = 99
}


typedef struct {
    uint16_t        start_address;
    uint16_t        size;
} Rom_File;


typedef struct {
    uint16_t                start_address;
    Memory_Map_Band_Type    type;
} Memory_Map_Band;


typedef struct {
    Rom_File        rom;
    bool            has_rom;
    bool            has_68221;
    uint16_t        ram_size;
    Memory_Map_Band[24];
} Environment;


