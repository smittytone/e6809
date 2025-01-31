/*
 * e6809 for Raspberry Pi Pico
 * Monitor code
 *
 * @version     0.0.2
 * @author      smittytone
 * @copyright   2025
 * @licence     MIT
 *
 */
#ifndef _MONITOR_HEADER_
#define _MONITOR_HEADER_


/*
 *  CONSTANTS
 */
#define DEBOUNCE_TIME_US            5000        // 5ms
#define UPLOAD_TIMEOUT_US           20000000    // 20s

#define DISPLAY_LEFT                0
#define DISPLAY_RIGHT               1

#define MENU_MODE_MAIN              0
#define MENU_MAIN_ADDR              1
#define MENU_MAIN_BYTE              2
#define MENU_MAIN_RUN_STEP          3
#define MENU_MAIN_RUN               4
#define INPUT_MAIN_ADDR             0x8000
#define INPUT_MAIN_BYTE             0x4000
#define INPUT_MAIN_RUN_STEP         0x2000
#define INPUT_MAIN_RUN              0x1000
#define INPUT_MAIN_MEM_UP           0x0008
#define INPUT_MAIN_MEM_DOWN         0x0001
#define INPUT_MAIN_LOAD             0x0800
#define INPUT_MAIN_MASK             0xF809

#define MENU_MODE_STEP              10
#define INPUT_STEP_NEXT             0x8000
#define INPUT_STEP_SHOW_CC          0x4000
#define INPUT_STEP_SHOW_AD          0x2000
#define INPUT_STEP_EXIT             0x1000
#define INPUT_STEP_MEM_UP           0x0008
#define INPUT_STEP_MEM_DOWN         0x0001
#define INPUT_STEP_MASK             0xF009

#define MENU_MODE_CONFIRM           20
#define MENU_CONF_OK                21
#define MENU_CONF_CANCEL            22
#define MENU_CONF_CONTINUE          23
#define INPUT_CONF_OK               0x8000
#define INPUT_CONF_CANCEL           0x1000
#define INPUT_CONF_CONTINUE         0x4000
#define INPUT_CONF_MASK_ADDR        0x9000
#define INPUT_CONF_MASK_BYTE        0xD000

#define MENU_MODE_RUN               30
#define MENU_MODE_RUN_DONE          31
#define INPUT_RUN_MASK              0xFFFF


/*
 *  PROTOTYPES
 */
bool init_board(void);
void monitor_event_loop(void);


#endif  // _MONITOR_HEADER_
