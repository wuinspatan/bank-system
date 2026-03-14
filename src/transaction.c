/*
 * transaction.c - Deposit, Withdraw, Transfer, and History.
 *
 * Storage format (data/transactions.csv):
 *   txn_id,account_number,ref_account,type,amount,balance_after,timestamp
 *   1,20260314001,,0,1000.00,6000.00,1741910400
 *   2,20260314001,20260314002,2,500.00,5500.00,1741910500
 *
 *   type: 0=deposit  1=withdraw  2=transfer
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "../include/transaction.h"
#include "../include/account.h"
#include "../include/menu.h"

static void safe_copy(char *dst, const char *src, size_t dstsize) {
    size_t n = strlen(src);
    if (n >= dstsize) n = dstsize - 1;
    memcpy(dst, src, n);
    dst[n] = '\0';
}

/* ── File I/O ────────────────────────────────────────────────────── */

int txn_load_all(Transaction **arr) {
    FILE *fp = fopen(FILE_TRANS, "r");
    if (!fp) { *arr = NULL; return 0; }

    int capacity = 32;
    *arr = (Transaction *)malloc((size_t)capacity * sizeof(Transaction));
    if (!*arr) { fclose(fp); return 0; }

    int count = 0;
    char line[512];
    while (fgets(line, sizeof(line), fp)) {
        if (line[0] == '#' || line[0] == '\n') continue;

        Transaction t;
        memset(&t, 0, sizeof(t));
        int type_int = 0;

        /*
         * Format: txn_id,account_number,ref_account,type,amount,balance_after,timestamp
         * ref_account may be empty → use a placeholder token then clear it.
         */
        char ref_buf[MAX_ACC_NUM] = "";
        if (sscanf(line, "%ld,%12[^,],%12[^,],%d,%lf,%lf,%ld",
                   &t.txn_id, t.account_number, ref_buf,
                   &type_int, &t.amount, &t.balance_after,
                   (long *)&t.timestamp) == 7) {
            /* ref_buf will be "-" when empty (see save) */
            if (strcmp(ref_buf, "-") != 0)
                safe_copy(t.ref_account, ref_buf, MAX_ACC_NUM);
            t.type = (TxnType)type_int;
        } else {
            continue;
        }

        if (count >= capacity) {
            capacity *= 2;
            Transaction *tmp = (Transaction *)realloc(
                *arr, (size_t)capacity * sizeof(Transaction));
            if (!tmp) { free(*arr); *arr = NULL; fclose(fp); return 0; }
            *arr = tmp;
        }
        (*arr)[count++] = t;
    }
    fclose(fp);
    if (count == 0) { free(*arr); *arr = NULL; }
    return count;
}

static void txn_save_all(Transaction *arr, int count) {
    FILE *fp = fopen(FILE_TRANS, "w");
    if (!fp) { perror("Cannot open transactions file"); return; }

    fprintf(fp, "# txn_id,account_number,ref_account,type,amount,balance_after,timestamp\n");
    for (int i = 0; i < count; i++) {
        /* Use "-" as placeholder for empty ref_account so CSV columns stay aligned */
        const char *ref = (arr[i].ref_account[0] != '\0') ? arr[i].ref_account : "-";
        fprintf(fp, "%ld,%s,%s,%d,%.2f,%.2f,%ld\n",
                arr[i].txn_id,
                arr[i].account_number,
                ref,
                (int)arr[i].type,
                arr[i].amount,
                arr[i].balance_after,
                (long)arr[i].timestamp);
    }
    fclose(fp);
}

long txn_next_id(Transaction *arr, int count) {
    long max = 0;
    for (int i = 0; i < count; i++)
        if (arr[i].txn_id > max) max = arr[i].txn_id;
    return max + 1;
}

void txn_record_full(const char *acc_num, const char *ref_acc,
                     TxnType type, double amount, double bal_after) {
    Transaction *arr = NULL;
    int count = txn_load_all(&arr);

    Transaction *newarr = (Transaction *)realloc(
        arr, (size_t)(count + 1) * sizeof(Transaction));
    if (!newarr) { free(arr); return; }
    arr = newarr;

    Transaction *t = &arr[count];
    memset(t, 0, sizeof(Transaction));
    t->txn_id        = txn_next_id(arr, count);
    safe_copy(t->account_number, acc_num,  MAX_ACC_NUM);
    safe_copy(t->ref_account,    ref_acc,  MAX_ACC_NUM);
    t->type          = type;
    t->amount        = amount;
    t->balance_after = bal_after;
    t->timestamp     = time(NULL);
    count++;

    txn_save_all(arr, count);
    free(arr);
}

void txn_record(const char *acc_num, TxnType type,
                double amount, double bal_after) {
    txn_record_full(acc_num, "", type, amount, bal_after);
}

/* ── Shared: print user's own accounts ───────────────────────────── */

static void print_my_accounts_compact(int owner_id) {
    Account *arr = NULL;
    int count = account_load_all(&arr);

    printf("  %-13s %-14s %-14s %12s\n",
           "Account No.", "First Name", "Last Name", "Balance");
    printf("  %-13s %-14s %-14s %12s\n",
           "------------", "-------------", "-------------", "-----------");

    int shown = 0;
    for (int i = 0; i < count; i++) {
        if (!arr[i].active || arr[i].owner_id != owner_id) continue;
        printf("  %-13s %-14s %-14s %12.2f\n",
               arr[i].account_number, arr[i].first_name,
               arr[i].last_name, arr[i].balance);
        shown++;
    }
    if (shown == 0) printf("  (no accounts)\n");
    free(arr);
}

/* ── Deposit ─────────────────────────────────────────────────────── */

void txn_deposit(int owner_id) {
    printf("\n-- Deposit --\n");
    print_my_accounts_compact(owner_id);
    printf("\n");

    char acc_num[MAX_ACC_NUM];
    read_line("  Account number: ", acc_num, MAX_ACC_NUM);

    Account *arr = NULL;
    int count = account_load_all(&arr);
    int idx   = account_find(arr, count, acc_num);

    if (idx < 0 || arr[idx].owner_id != owner_id) {
        printf("  [!] Account not found or does not belong to you\n");
        free(arr); press_enter(); return;
    }

    printf("  Current balance: %.2f\n", arr[idx].balance);
    printf("  Amount to deposit: ");
    double amount = 0;
    if (scanf("%lf", &amount) != 1) amount = 0;
    while (getchar() != '\n');

    if (amount <= 0) {
        printf("  [!] Amount must be greater than 0\n");
        free(arr); press_enter(); return;
    }

    arr[idx].balance += amount;
    account_save_all(arr, count);
    txn_record(acc_num, TXN_DEPOSIT, amount, arr[idx].balance);

    printf("  Deposited %.2f  |  New balance: %.2f\n", amount, arr[idx].balance);
    free(arr);
    press_enter();
}

/* ── Withdraw ────────────────────────────────────────────────────── */

void txn_withdraw(int owner_id) {
    printf("\n-- Withdraw --\n");
    print_my_accounts_compact(owner_id);
    printf("\n");

    char acc_num[MAX_ACC_NUM];
    read_line("  Account number: ", acc_num, MAX_ACC_NUM);

    Account *arr = NULL;
    int count = account_load_all(&arr);
    int idx   = account_find(arr, count, acc_num);

    if (idx < 0 || arr[idx].owner_id != owner_id) {
        printf("  [!] Account not found or does not belong to you\n");
        free(arr); press_enter(); return;
    }

    printf("  Current balance: %.2f\n", arr[idx].balance);
    printf("  Amount to withdraw: ");
    double amount = 0;
    if (scanf("%lf", &amount) != 1) amount = 0;
    while (getchar() != '\n');

    if (amount <= 0) {
        printf("  [!] Amount must be greater than 0\n");
        free(arr); press_enter(); return;
    }
    if (amount > arr[idx].balance) {
        printf("  [!] Insufficient funds (available: %.2f)\n", arr[idx].balance);
        free(arr); press_enter(); return;
    }

    arr[idx].balance -= amount;
    account_save_all(arr, count);
    txn_record(acc_num, TXN_WITHDRAW, amount, arr[idx].balance);

    printf("  Withdrawn %.2f  |  New balance: %.2f\n", amount, arr[idx].balance);
    free(arr);
    press_enter();
}

/* ── Transfer ────────────────────────────────────────────────────── */

void txn_transfer(int owner_id) {
    printf("\n-- Transfer --\n");
    printf("  [Your accounts]\n");
    print_my_accounts_compact(owner_id);
    printf("\n");

    char from_num[MAX_ACC_NUM];
    read_line("  From account number: ", from_num, MAX_ACC_NUM);

    Account *arr = NULL;
    int count = account_load_all(&arr);
    int from_idx = account_find(arr, count, from_num);

    if (from_idx < 0 || arr[from_idx].owner_id != owner_id) {
        printf("  [!] Source account not found or does not belong to you\n");
        free(arr); press_enter(); return;
    }

    printf("  Source balance: %.2f\n\n", arr[from_idx].balance);

    char to_num[MAX_ACC_NUM];
    read_line("  To account number  : ", to_num, MAX_ACC_NUM);

    if (strcmp(from_num, to_num) == 0) {
        printf("  [!] Cannot transfer to the same account\n");
        free(arr); press_enter(); return;
    }

    int to_idx = account_find(arr, count, to_num);
    if (to_idx < 0) {
        printf("  [!] Destination account %s not found\n", to_num);
        free(arr); press_enter(); return;
    }

    printf("  Destination : %s %s (Account: %s)\n",
           arr[to_idx].first_name, arr[to_idx].last_name,
           arr[to_idx].account_number);

    printf("  Amount to transfer: ");
    double amount = 0;
    if (scanf("%lf", &amount) != 1) amount = 0;
    while (getchar() != '\n');

    if (amount <= 0) {
        printf("  [!] Amount must be greater than 0\n");
        free(arr); press_enter(); return;
    }
    if (amount > arr[from_idx].balance) {
        printf("  [!] Insufficient funds (available: %.2f)\n", arr[from_idx].balance);
        free(arr); press_enter(); return;
    }

    printf("\n  Confirm transfer %.2f from %s to %s %s? (y/n): ",
           amount, from_num, arr[to_idx].first_name, arr[to_idx].last_name);
    char c = (char)getchar(); while (getchar() != '\n');
    if (c != 'y' && c != 'Y') {
        printf("  Cancelled\n"); free(arr); press_enter(); return;
    }

    arr[from_idx].balance -= amount;
    arr[to_idx].balance   += amount;
    account_save_all(arr, count);

    txn_record_full(from_num, to_num,   TXN_TRANSFER, amount, arr[from_idx].balance);
    txn_record_full(to_num,   from_num, TXN_TRANSFER, amount, arr[to_idx].balance);

    printf("\n  Transfer complete\n");
    printf("  %s  new balance: %.2f\n", from_num, arr[from_idx].balance);
    printf("  %s  new balance: %.2f\n", to_num,   arr[to_idx].balance);

    free(arr);
    press_enter();
}

/* ── History ─────────────────────────────────────────────────────── */

void txn_history(int owner_id) {
    printf("\n-- Transaction History --\n");
    print_my_accounts_compact(owner_id);
    printf("\n");

    char acc_num[MAX_ACC_NUM];
    read_line("  Account number: ", acc_num, MAX_ACC_NUM);

    Account *accs = NULL;
    int acnt = account_load_all(&accs);
    int aidx = account_find(accs, acnt, acc_num);
    if (aidx < 0 || accs[aidx].owner_id != owner_id) {
        printf("  [!] Account not found or does not belong to you\n");
        free(accs); press_enter(); return;
    }
    free(accs);

    Transaction *arr = NULL;
    int count = txn_load_all(&arr);

    printf("\n  %-6s %-10s %12s %-13s %-22s\n",
           "TxID", "Type", "Amount", "Ref Account", "Timestamp");
    printf("  %-6s %-10s %12s %-13s %-22s\n",
           "-----", "---------", "-----------", "------------", "---------------------");

    int shown = 0;
    for (int i = 0; i < count; i++) {
        if (strcmp(arr[i].account_number, acc_num) != 0) continue;

        char tbuf[32];
        struct tm *tm_info = localtime(&arr[i].timestamp);
        strftime(tbuf, sizeof(tbuf), "%Y-%m-%d %H:%M:%S", tm_info);

        const char *type_str;
        switch (arr[i].type) {
            case TXN_DEPOSIT:  type_str = "Deposit";  break;
            case TXN_WITHDRAW: type_str = "Withdraw"; break;
            case TXN_TRANSFER: type_str = "Transfer"; break;
            default:           type_str = "Unknown";  break;
        }

        const char *ref = (arr[i].ref_account[0] != '\0') ? arr[i].ref_account : "-";
        printf("  %-6ld %-10s %12.2f %-13s %-22s\n",
               arr[i].txn_id, type_str, arr[i].amount, ref, tbuf);
        shown++;
    }
    if (shown == 0) printf("  (no transactions)\n");
    printf("  Total: %d transactions\n", shown);

    free(arr);
    press_enter();
}
