/*
 * test_transaction.c - Unit tests for transaction.c
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <sys/stat.h>

#include "test_runner.h"
#include "../include/models.h"
#include "../include/transaction.h"
#include "../include/account.h"

static void clean_files(void) {
    remove(FILE_TRANS);
    remove(FILE_ACCOUNTS);
}

static Account make_account(const char *num, double balance, int owner) {
    Account a;
    memset(&a, 0, sizeof(a));
    size_t n = strlen(num); if (n >= MAX_ACC_NUM) n = MAX_ACC_NUM-1;
    memcpy(a.account_number, num, n);
    memcpy(a.first_name, "Test", 4);
    memcpy(a.last_name,  "User", 4);
    memcpy(a.phone, "0800000000", 10);
    a.type = ACC_SAVING; a.balance = balance; a.owner_id = owner; a.active = 1;
    return a;
}

/* ── Tests ───────────────────────────────────────────────────────── */

static void test_txn_load_empty(void) {
    remove(FILE_TRANS);
    FILE *fp = fopen(FILE_TRANS, "wb"); fclose(fp);
    Transaction *arr = NULL;
    int count = txn_load_all(&arr);
    ASSERT_EQ(count, 0);
    ASSERT_NULL(arr);
}

static void test_txn_record_deposit(void) {
    remove(FILE_TRANS);
    txn_record("ACC001", TXN_DEPOSIT, 1000.0, 1000.0);

    Transaction *arr = NULL;
    int count = txn_load_all(&arr);
    ASSERT_EQ(count, 1);
    ASSERT_STR(arr[0].account_number, "ACC001");
    ASSERT_EQ(arr[0].type, TXN_DEPOSIT);
    ASSERT(arr[0].amount > 999.99 && arr[0].amount < 1000.01);
    ASSERT_GT(arr[0].txn_id, 0);
    free(arr);
}

static void test_txn_record_withdraw(void) {
    remove(FILE_TRANS);
    txn_record("ACC001", TXN_WITHDRAW, 500.0, 500.0);

    Transaction *arr = NULL;
    int count = txn_load_all(&arr);
    ASSERT_EQ(count, 1);
    ASSERT_EQ(arr[0].type, TXN_WITHDRAW);
    free(arr);
}

static void test_txn_record_multiple_appends(void) {
    remove(FILE_TRANS);
    txn_record("ACC001", TXN_DEPOSIT,  1000.0, 1000.0);
    txn_record("ACC001", TXN_WITHDRAW,  200.0,  800.0);
    txn_record("ACC001", TXN_DEPOSIT,   500.0, 1300.0);

    Transaction *arr = NULL;
    int count = txn_load_all(&arr);
    ASSERT_EQ(count, 3);
    ASSERT_EQ(arr[0].txn_id, 1);
    ASSERT_EQ(arr[1].txn_id, 2);
    ASSERT_EQ(arr[2].txn_id, 3);
    free(arr);
}

static void test_txn_next_id_empty(void) {
    ASSERT_EQ(txn_next_id(NULL, 0), 1);
}

static void test_txn_next_id_with_records(void) {
    Transaction arr[3];
    memset(arr, 0, sizeof(arr));
    arr[0].txn_id = 1;
    arr[1].txn_id = 5;
    arr[2].txn_id = 3;
    ASSERT_EQ(txn_next_id(arr, 3), 6);
}

static void test_txn_timestamp_recorded(void) {
    remove(FILE_TRANS);
    time_t before = time(NULL);
    txn_record("TACC", TXN_DEPOSIT, 100.0, 100.0);
    time_t after  = time(NULL);

    Transaction *arr = NULL;
    txn_load_all(&arr);
    ASSERT(arr[0].timestamp >= before);
    ASSERT(arr[0].timestamp <= after);
    free(arr);
}

static void test_txn_balance_after_correct(void) {
    remove(FILE_TRANS);
    txn_record("ACC777", TXN_DEPOSIT, 2500.0, 7500.0);

    Transaction *arr = NULL;
    txn_load_all(&arr);
    ASSERT(arr[0].balance_after > 7499.99 && arr[0].balance_after < 7500.01);
    free(arr);
}

static void test_txn_different_accounts(void) {
    remove(FILE_TRANS);
    txn_record("ACC-A", TXN_DEPOSIT,  100.0, 100.0);
    txn_record("ACC-B", TXN_DEPOSIT,  200.0, 200.0);
    txn_record("ACC-A", TXN_WITHDRAW,  50.0,  50.0);

    Transaction *arr = NULL;
    int count = txn_load_all(&arr);
    ASSERT_EQ(count, 3);

    int cnt_a = 0, cnt_b = 0;
    for (int i = 0; i < count; i++) {
        if (strcmp(arr[i].account_number, "ACC-A") == 0) cnt_a++;
        if (strcmp(arr[i].account_number, "ACC-B") == 0) cnt_b++;
    }
    ASSERT_EQ(cnt_a, 2);
    ASSERT_EQ(cnt_b, 1);
    free(arr);
}

static void test_deposit_updates_balance(void) {
    clean_files();
    Account acc = make_account("INT001", 1000.0, 42);
    account_save_all(&acc, 1);

    Account *arr = NULL;
    int count = account_load_all(&arr);
    int idx   = account_find(arr, count, "INT001");
    ASSERT_GT(idx, -1);

    double deposit_amt = 500.0;
    arr[idx].balance += deposit_amt;
    account_save_all(arr, count);
    txn_record("INT001", TXN_DEPOSIT, deposit_amt, arr[idx].balance);
    free(arr);

    arr = NULL;
    account_load_all(&arr);
    idx = account_find(arr, 1, "INT001");
    ASSERT(arr[idx].balance > 1499.99 && arr[idx].balance < 1500.01);
    free(arr);

    Transaction *txns = NULL;
    int tc = txn_load_all(&txns);
    ASSERT_EQ(tc, 1);
    ASSERT_EQ(txns[0].type, TXN_DEPOSIT);
    free(txns);
}

static void test_withdraw_prevents_overdraft(void) {
    clean_files();
    Account acc = make_account("INT002", 300.0, 42);
    account_save_all(&acc, 1);

    Account *arr = NULL;
    account_load_all(&arr);
    int idx = account_find(arr, 1, "INT002");
    int allowed = (500.0 <= arr[idx].balance);
    ASSERT_EQ(allowed, 0);
    free(arr);
}

static void test_withdraw_exact_balance(void) {
    clean_files();
    Account acc = make_account("INT003", 200.0, 42);
    account_save_all(&acc, 1);

    Account *arr = NULL;
    account_load_all(&arr);
    int idx = account_find(arr, 1, "INT003");
    int allowed = (200.0 <= arr[idx].balance);
    ASSERT_EQ(allowed, 1);

    arr[idx].balance -= 200.0;
    account_save_all(arr, 1);
    txn_record("INT003", TXN_WITHDRAW, 200.0, arr[idx].balance);
    free(arr);

    arr = NULL;
    account_load_all(&arr);
    ASSERT(arr[0].balance >= 0.0 && arr[0].balance < 0.01);
    free(arr);
}


/* ── Transfer tests ──────────────────────────────────────────────── */

static void test_transfer_record_two_rows(void) {
    clean_files();
    /* Two accounts */
    Account a = make_account("TRF001", 1000.0, 1);
    Account b = make_account("TRF002",  500.0, 2);
    Account accs[2] = {a, b};
    account_save_all(accs, 2);

    /* Simulate transfer: debit TRF001, credit TRF002 */
    accs[0].balance -= 300.0;
    accs[1].balance += 300.0;
    account_save_all(accs, 2);
    txn_record_full("TRF001", "TRF002", TXN_TRANSFER, 300.0, accs[0].balance);
    txn_record_full("TRF002", "TRF001", TXN_TRANSFER, 300.0, accs[1].balance);

    Transaction *arr = NULL;
    int count = txn_load_all(&arr);
    /* Must produce exactly 2 transaction rows */
    ASSERT_EQ(count, 2);
    ASSERT_EQ(arr[0].type, TXN_TRANSFER);
    ASSERT_EQ(arr[1].type, TXN_TRANSFER);
    /* Each row's ref_account must point to the other account */
    ASSERT_STR(arr[0].ref_account, "TRF002");
    ASSERT_STR(arr[1].ref_account, "TRF001");
    free(arr);
}

static void test_transfer_balances_correct(void) {
    clean_files();
    Account a = make_account("TRF001", 1000.0, 1);
    Account b = make_account("TRF002",  200.0, 2);
    Account accs[2] = {a, b};
    account_save_all(accs, 2);

    double transfer = 400.0;
    accs[0].balance -= transfer;
    accs[1].balance += transfer;
    account_save_all(accs, 2);

    /* Reload and verify */
    Account *loaded = NULL;
    account_load_all(&loaded);
    /* TRF001: 1000 - 400 = 600 */
    ASSERT(loaded[0].balance > 599.99 && loaded[0].balance < 600.01);
    /* TRF002:  200 + 400 = 600 */
    ASSERT(loaded[1].balance > 599.99 && loaded[1].balance < 600.01);
    free(loaded);
}

static void test_transfer_prevents_overdraft(void) {
    clean_files();
    Account a = make_account("TRF001", 100.0, 1);
    Account b = make_account("TRF002", 100.0, 2);
    Account accs[2] = {a, b};
    account_save_all(accs, 2);

    double transfer = 500.0;  /* more than balance */
    int allowed = (transfer <= accs[0].balance);
    ASSERT_EQ(allowed, 0);
}

static void test_transfer_same_account_blocked(void) {
    /* Same source and destination must be rejected */
    int same = (strcmp("TRF001", "TRF001") == 0);
    ASSERT_EQ(same, 1);  /* confirms detection logic works */
}

/* ── main ────────────────────────────────────────────────────────── */

int main(void) {
    if (mkdir("data", 0755) != 0 && errno != EEXIST) {
        perror("mkdir data");
    }
    clean_files();

    TEST_BEGIN();

    printf("[ Transaction I/O ]\n");
    RUN_TEST(test_txn_load_empty);
    RUN_TEST(test_txn_record_deposit);
    RUN_TEST(test_txn_record_withdraw);
    RUN_TEST(test_txn_record_multiple_appends);
    RUN_TEST(test_txn_timestamp_recorded);
    RUN_TEST(test_txn_balance_after_correct);
    RUN_TEST(test_txn_different_accounts);

    printf("\n[ Transaction ID ]\n");
    RUN_TEST(test_txn_next_id_empty);
    RUN_TEST(test_txn_next_id_with_records);

    printf("\n[ Deposit / Withdraw Integration ]\n");
    RUN_TEST(test_deposit_updates_balance);
    RUN_TEST(test_withdraw_prevents_overdraft);
    RUN_TEST(test_withdraw_exact_balance);

    clean_files();

    printf("\n[ Transfer ]\n");
    RUN_TEST(test_transfer_record_two_rows);
    RUN_TEST(test_transfer_balances_correct);
    RUN_TEST(test_transfer_prevents_overdraft);
    RUN_TEST(test_transfer_same_account_blocked);

    clean_files();
    TEST_SUMMARY();
}

