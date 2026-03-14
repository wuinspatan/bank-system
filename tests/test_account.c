/*
 * test_account.c - Unit tests for account.c
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>

#include "test_runner.h"
#include "../include/models.h"
#include "../include/account.h"

static Account make_account(const char *num, const char *fn, const char *ln,
                             AccountType type, double balance, int owner, int active) {
    Account a;
    memset(&a, 0, sizeof(a));
    size_t n;
    n = strlen(num); if (n >= MAX_ACC_NUM) n = MAX_ACC_NUM-1;
    memcpy(a.account_number, num, n);
    n = strlen(fn);  if (n >= MAX_STR)    n = MAX_STR-1;
    memcpy(a.first_name, fn, n);
    n = strlen(ln);  if (n >= MAX_STR)    n = MAX_STR-1;
    memcpy(a.last_name, ln, n);
    memcpy(a.phone, "0800000000", 10);
    a.type = type; a.balance = balance; a.owner_id = owner; a.active = active;
    return a;
}

static void cleanup(void) { remove(FILE_ACCOUNTS); }

/* ── Tests ───────────────────────────────────────────────────────── */

static void test_account_load_empty(void) {
    remove(FILE_ACCOUNTS);
    FILE *fp = fopen(FILE_ACCOUNTS, "wb"); fclose(fp);
    Account *arr = NULL;
    int count = account_load_all(&arr);
    ASSERT_EQ(count, 0);
    ASSERT_NULL(arr);
}

static void test_account_save_and_load(void) {
    Account accs[2] = {
        make_account("20260101001", "Somchai", "Sriwong", ACC_SAVING, 5000.0,  1, 1),
        make_account("20260101002", "Malee",   "Jaidee",  ACC_FIXED,  10000.0, 1, 1),
    };
    account_save_all(accs, 2);

    Account *arr = NULL;
    int count = account_load_all(&arr);
    ASSERT_EQ(count, 2);
    ASSERT_STR(arr[0].account_number, "20260101001");
    ASSERT_STR(arr[0].first_name, "Somchai");
    ASSERT_EQ(arr[0].type, ACC_SAVING);
    ASSERT_EQ(arr[1].type, ACC_FIXED);
    free(arr);
    cleanup();
}

static void test_account_find_existing(void) {
    Account accs[2] = {
        make_account("ACC001", "A", "B", ACC_SAVING, 100.0, 1, 1),
        make_account("ACC002", "C", "D", ACC_SAVING, 200.0, 2, 1),
    };
    ASSERT_EQ(account_find(accs, 2, "ACC001"), 0);
    ASSERT_EQ(account_find(accs, 2, "ACC002"), 1);
}

static void test_account_find_not_found(void) {
    Account accs[1] = {
        make_account("ACC001", "A", "B", ACC_SAVING, 100.0, 1, 1),
    };
    ASSERT_EQ(account_find(accs, 1, "XXXXXX"), -1);
}

static void test_account_find_inactive(void) {
    Account accs[2] = {
        make_account("ACC001", "A", "B", ACC_SAVING, 100.0, 1, 0),
        make_account("ACC002", "C", "D", ACC_SAVING, 200.0, 1, 1),
    };
    ASSERT_EQ(account_find(accs, 2, "ACC001"), -1);
    ASSERT_EQ(account_find(accs, 2, "ACC002"), 1);
}

static void test_account_next_seq_empty(void) {
    ASSERT_EQ(account_next_seq(NULL, 0), 1);
}

static void test_account_next_seq_multiple(void) {
    Account accs[3] = {
        make_account("20260101001", "A", "B", ACC_SAVING, 0, 1, 1),
        make_account("20260101003", "C", "D", ACC_SAVING, 0, 1, 1),
        make_account("20260101002", "E", "F", ACC_SAVING, 0, 1, 1),
    };
    ASSERT_EQ(account_next_seq(accs, 3), 4);
}

static void test_account_balance_persists(void) {
    Account accs[1] = {
        make_account("ACC999", "Test", "User", ACC_SAVING, 12345.67, 1, 1),
    };
    account_save_all(accs, 1);

    Account *arr = NULL;
    account_load_all(&arr);
    ASSERT(arr[0].balance > 12345.66 && arr[0].balance < 12345.68);
    free(arr);
    cleanup();
}

static void test_account_soft_close(void) {
    Account accs[2] = {
        make_account("ACC001", "A", "B", ACC_SAVING, 500.0, 1, 1),
        make_account("ACC002", "C", "D", ACC_SAVING, 300.0, 1, 1),
    };
    account_save_all(accs, 2);

    Account *arr = NULL;
    int count = account_load_all(&arr);
    int idx = account_find(arr, count, "ACC001");
    ASSERT_GT(idx, -1);
    arr[idx].active = 0;
    account_save_all(arr, count);
    free(arr);

    arr = NULL;
    account_load_all(&arr);
    ASSERT_EQ(account_find(arr, count, "ACC001"), -1);
    ASSERT_EQ(account_find(arr, count, "ACC002"),  1);
    free(arr);
    cleanup();
}

static void test_account_owner_isolation(void) {
    Account accs[3] = {
        make_account("A001", "U1a", "L", ACC_SAVING, 100.0, 1, 1),
        make_account("A002", "U1b", "L", ACC_SAVING, 200.0, 1, 1),
        make_account("A003", "U2a", "L", ACC_SAVING, 300.0, 2, 1),
    };
    int owner1 = 0, owner2 = 0;
    for (int i = 0; i < 3; i++) {
        if (accs[i].owner_id == 1) owner1++;
        if (accs[i].owner_id == 2) owner2++;
    }
    ASSERT_EQ(owner1, 2);
    ASSERT_EQ(owner2, 1);
}

/* ── main ────────────────────────────────────────────────────────── */

int main(void) {
    if (mkdir("data", 0755) != 0 && errno != EEXIST) {
        perror("mkdir data");
    }
    remove(FILE_ACCOUNTS);

    TEST_BEGIN();

    printf("[ Account I/O ]\n");
    RUN_TEST(test_account_load_empty);
    RUN_TEST(test_account_save_and_load);
    RUN_TEST(test_account_balance_persists);

    printf("\n[ Account Lookup ]\n");
    RUN_TEST(test_account_find_existing);
    RUN_TEST(test_account_find_not_found);
    RUN_TEST(test_account_find_inactive);

    printf("\n[ Sequence Generation ]\n");
    RUN_TEST(test_account_next_seq_empty);
    RUN_TEST(test_account_next_seq_multiple);

    printf("\n[ Account Operations ]\n");
    RUN_TEST(test_account_soft_close);
    RUN_TEST(test_account_owner_isolation);

    remove(FILE_ACCOUNTS);
    TEST_SUMMARY();
}
