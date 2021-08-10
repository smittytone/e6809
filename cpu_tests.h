/*
 * e6809 for Raspberry Pi Pico
 *
 * @version     1.0.0
 * @author      smittytone
 * @copyright   2021
 * @licence     MIT
 *
 */
#ifndef _CPU_TESTS_HEADER_
#define _CPU_TESTS_HEADER_


/*
 * CONSTANTS
 */


/*
 * STRUCTURES
 */


/*
 * PROTOTYPES
 */
void        test_main();
void        test_alu();
void        test_index();
void        test_logic();
void        test_setup();


uint32_t errors = 0;
uint32_t passes = 0;
uint32_t tests = 0;


#endif // _CPU_TESTS_HEADER_
