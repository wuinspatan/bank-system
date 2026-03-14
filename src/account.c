/*
 * account.c - Bank account CRUD operations.
 *
 * Storage format (data/accounts.csv):
 *   account_number,first_name,last_name,phone,type,balance,owner_id,active
 *   20260314001,Somchai,Sriwong,0812345678,0,5000.00,1,1
 *
 *   type: 0=saving  1=fixed
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "../include/account.h"
#include "../include/menu.h"

static void safe_copy(char *dst, const char *src, size_t dstsize) {
    size_t n = strlen(src);
    if (n >= dstsize) n = dstsize - 1;
    memcpy(dst, src, n);
    dst[n] = '\0';
}

/* ── File I/O ────────────────────────────────────────────────────── */

int account_load_all(Account **arr) {
    FILE *fp = fopen(FILE_ACCOUNTS, "r");
    if (!fp) { *arr = NULL; return 0; }

    int capacity = 16;
    *arr = (Account *)malloc((size_t)capacity * sizeof(Account));
    if (!*arr) { fclose(fp); return 0; }

    int count = 0;
    char line[512];
    while (fgets(line, sizeof(line), fp)) {
        if (line[0] == '#' || line[0] == '\n') continue;

        Account a;
        memset(&a, 0, sizeof(a));
        int type_int = 0;
        if (sscanf(line, "%12[^,],%63[^,],%63[^,],%15[^,],%d,%lf,%d,%d",
                   a.account_number, a.first_name, a.last_name,
                   a.phone, &type_int, &a.balance,
                   &a.owner_id, &a.active) != 8)
            continue;
        a.type = (AccountType)type_int;

        if (count >= capacity) {
            capacity *= 2;
            Account *tmp = (Account *)realloc(*arr, (size_t)capacity * sizeof(Account));
            if (!tmp) { free(*arr); *arr = NULL; fclose(fp); return 0; }
            *arr = tmp;
        }
        (*arr)[count++] = a;
    }
    fclose(fp);
    if (count == 0) { free(*arr); *arr = NULL; }
    return count;
}

void account_save_all(Account *arr, int count) {
    FILE *fp = fopen(FILE_ACCOUNTS, "w");
    if (!fp) { perror("Cannot open accounts file"); return; }

    fprintf(fp, "# account_number,first_name,last_name,phone,type,balance,owner_id,active\n");
    for (int i = 0; i < count; i++) {
        fprintf(fp, "%s,%s,%s,%s,%d,%.2f,%d,%d\n",
                arr[i].account_number,
                arr[i].first_name,
                arr[i].last_name,
                arr[i].phone,
                (int)arr[i].type,
                arr[i].balance,
                arr[i].owner_id,
                arr[i].active);
    }
    fclose(fp);
}

/* ── Helpers ─────────────────────────────────────────────────────── */

int account_find(Account *arr, int count, const char *acc_num) {
    for (int i = 0; i < count; i++) {
        if (arr[i].active && strcmp(arr[i].account_number, acc_num) == 0)
            return i;
    }
    return -1;
}

int account_next_seq(Account *arr, int count) {
    int max = 0;
    for (int i = 0; i < count; i++) {
        int seq = 0;
        sscanf(arr[i].account_number + 8, "%d", &seq);
        if (seq > max) max = seq;
    }
    return max + 1;
}

void account_generate_number(char *buf) {
    Account *arr = NULL;
    int count = account_load_all(&arr);
    int seq   = account_next_seq(arr, count) % 1000;
    free(arr);

    time_t t = time(NULL);
    struct tm *tm_info = localtime(&t);
    int year = tm_info->tm_year + 1900;
    int mon  = tm_info->tm_mon  + 1;
    int day  = tm_info->tm_mday;
    year = (year<1000)?1000:(year>9999)?9999:year;
    mon  = (mon <   1)?   1:(mon > 12)?  12:mon;
    day  = (day <   1)?   1:(day > 31)?  31:day;
    seq  = (seq <   0)?   0:(seq >999)? 999:seq;

    char tmp[32];
    snprintf(tmp, sizeof(tmp), "%04d%02d%02d%03d", year, mon, day, seq);
    safe_copy(buf, tmp, MAX_ACC_NUM);
}

static void print_my_accounts(int owner_id) {
    Account *arr = NULL;
    int count = account_load_all(&arr);

    printf("  %-13s %-14s %-14s %-8s %12s\n",
           "Account No.", "First Name", "Last Name", "Type", "Balance");
    printf("  %-13s %-14s %-14s %-8s %12s\n",
           "------------", "-------------", "-------------", "-------", "-----------");

    int shown = 0;
    for (int i = 0; i < count; i++) {
        if (!arr[i].active) continue;
        if (owner_id >= 0 && arr[i].owner_id != owner_id) continue;
        const char *t = (arr[i].type == ACC_SAVING) ? "Saving" : "Fixed";
        printf("  %-13s %-14s %-14s %-8s %12.2f\n",
               arr[i].account_number, arr[i].first_name,
               arr[i].last_name, t, arr[i].balance);
        shown++;
    }
    if (shown == 0) printf("  (no accounts)\n");
    free(arr);
}

/* ── Create ──────────────────────────────────────────────────────── */

void account_create(int owner_id) {
    printf("\n-- Open New Account --\n");

    Account acc;
    memset(&acc, 0, sizeof(acc));
    account_generate_number(acc.account_number);
    printf("  Account number assigned: %s\n", acc.account_number);

    read_line("  First name : ", acc.first_name, MAX_STR);
    read_line("  Last name  : ", acc.last_name,  MAX_STR);
    read_line("  Phone      : ", acc.phone,      MAX_PHONE);

    printf("  Account type (1=Saving, 2=Fixed): ");
    int t = read_int("");
    acc.type = (t == 2) ? ACC_FIXED : ACC_SAVING;

    printf("  Initial deposit: ");
    double amt = 0;
    if (scanf("%lf", &amt) != 1) amt = 0;
    while (getchar() != '\n');

    if (amt < 0) {
        printf("  [!] Amount cannot be negative\n");
        press_enter(); return;
    }
    acc.balance  = amt;
    acc.owner_id = owner_id;
    acc.active   = 1;

    Account *arr = NULL;
    int count = account_load_all(&arr);
    Account *newarr = (Account *)realloc(arr, (size_t)(count+1)*sizeof(Account));
    if (!newarr) { free(arr); return; }
    arr = newarr;
    arr[count++] = acc;
    account_save_all(arr, count);
    free(arr);

    printf("  Account %s opened\n", acc.account_number);
    press_enter();
}

/* ── Delete ──────────────────────────────────────────────────────── */

void account_delete(int owner_id) {
    printf("\n-- Close Account --\n");
    print_my_accounts(owner_id);
    printf("\n");
    char acc_num[MAX_ACC_NUM];
    read_line("  Account number to close: ", acc_num, MAX_ACC_NUM);

    Account *arr = NULL;
    int count = account_load_all(&arr);
    int idx   = account_find(arr, count, acc_num);

    if (idx < 0) {
        printf("  [!] Account %s not found\n", acc_num);
        free(arr); press_enter(); return;
    }
    if (arr[idx].owner_id != owner_id) {
        printf("  [!] This account does not belong to you\n");
        free(arr); press_enter(); return;
    }

    printf("  Confirm close account %s? (y/n): ", acc_num);
    char c = (char)getchar(); while (getchar() != '\n');
    if (c != 'y' && c != 'Y') {
        printf("  Cancelled\n"); free(arr); press_enter(); return;
    }

    arr[idx].active = 0;
    account_save_all(arr, count);
    free(arr);
    printf("  Account %s closed\n", acc_num);
    press_enter();
}

/* ── Edit ────────────────────────────────────────────────────────── */

void account_edit(int owner_id) {
    printf("\n-- Edit Account --\n");
    print_my_accounts(owner_id);
    printf("\n");
    char acc_num[MAX_ACC_NUM];
    read_line("  Account number to edit: ", acc_num, MAX_ACC_NUM);

    Account *arr = NULL;
    int count = account_load_all(&arr);
    int idx   = account_find(arr, count, acc_num);

    if (idx < 0 || arr[idx].owner_id != owner_id) {
        printf("  [!] Account not found or does not belong to you\n");
        free(arr); press_enter(); return;
    }

    char buf[MAX_STR];
    printf("  Current first name: %s\n", arr[idx].first_name);
    read_line("  New first name (Enter to skip): ", buf, MAX_STR);
    if (strlen(buf) > 0) safe_copy(arr[idx].first_name, buf, MAX_STR);

    printf("  Current last name: %s\n", arr[idx].last_name);
    read_line("  New last name (Enter to skip): ", buf, MAX_STR);
    if (strlen(buf) > 0) safe_copy(arr[idx].last_name, buf, MAX_STR);

    printf("  Current phone: %s\n", arr[idx].phone);
    read_line("  New phone (Enter to skip): ", buf, MAX_PHONE);
    if (strlen(buf) > 0) safe_copy(arr[idx].phone, buf, MAX_PHONE);

    account_save_all(arr, count);
    free(arr);
    printf("  Account updated\n");
    press_enter();
}

/* ── Search ──────────────────────────────────────────────────────── */

void account_search(void) {
    printf("\n-- Search Account --\n");
    char acc_num[MAX_ACC_NUM];
    read_line("  Account number: ", acc_num, MAX_ACC_NUM);

    Account *arr = NULL;
    int count = account_load_all(&arr);
    int idx   = account_find(arr, count, acc_num);

    if (idx < 0) {
        printf("  [!] Account %s not found\n", acc_num);
    } else {
        Account *a = &arr[idx];
        const char *type_str = (a->type == ACC_SAVING) ? "Saving" : "Fixed";
        printf("\n  Account   : %s\n", a->account_number);
        printf("  Name      : %s %s\n", a->first_name, a->last_name);
        printf("  Phone     : %s\n", a->phone);
        printf("  Type      : %s\n", type_str);
        printf("  Balance   : %.2f\n", a->balance);
    }
    free(arr);
    press_enter();
}

/* ── List ────────────────────────────────────────────────────────── */

void account_list(int owner_id) {
    const char *title = (owner_id < 0) ? "All Accounts" : "My Accounts";
    printf("\n-- %s --\n", title);
    print_my_accounts(owner_id);
    press_enter();
}
