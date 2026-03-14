/*
 * test_runner.h - Minimal unit test framework (no external dependencies)
 */
#ifndef TEST_RUNNER_H
#define TEST_RUNNER_H

#include <stdio.h>
#include <stdlib.h>

static int _tests_run    = 0;
static int _tests_passed = 0;
static int _tests_failed = 0;

#define ASSERT(cond) \
    do { \
        if (!(cond)) { \
            printf("    FAIL: %s  (line %d)\n", #cond, __LINE__); \
            _tests_failed++; \
            return; \
        } \
    } while (0)

#define ASSERT_EQ(a, b)   ASSERT((a) == (b))
#define ASSERT_STR(a, b)  ASSERT(strcmp((a),(b)) == 0)
#define ASSERT_GT(a, b)   ASSERT((a) >  (b))
#define ASSERT_NULL(p)    ASSERT((p) == NULL)
#define ASSERT_NOTNULL(p) ASSERT((p) != NULL)

#define RUN_TEST(fn) \
    do { \
        printf("  %-48s", #fn); \
        fflush(stdout); \
        _tests_run++; \
        fn(); \
        if (_tests_failed == _prev_failed) { \
            printf("PASS\n"); \
            _tests_passed++; \
        } \
        _prev_failed = _tests_failed; \
    } while (0)

#define TEST_BEGIN() \
    int _prev_failed = 0; \
    printf("\n=== BANK SYSTEM TEST SUITE ===\n\n")

#define TEST_SUMMARY() \
    do { \
        printf("\n------------------------------\n"); \
        printf("Total  : %d\n", _tests_run); \
        printf("Passed : %d\n", _tests_passed); \
        printf("Failed : %d\n", _tests_failed); \
        printf("------------------------------\n\n"); \
        return (_tests_failed > 0) ? EXIT_FAILURE : EXIT_SUCCESS; \
    } while (0)

#endif /* TEST_RUNNER_H */
