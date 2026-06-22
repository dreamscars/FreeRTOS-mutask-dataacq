/**
 * @file test_runner.c
 * @brief Unity test runner — main entry point for all unit tests.
 */

#include "unity.h"

/* Test group declarations */
void run_data_process_tests(void);

/* ======================================================================
 * Main
 * ====================================================================== */
int main(void)
{
    UNITY_BEGIN();

    run_data_process_tests();

    return UNITY_END();
}
