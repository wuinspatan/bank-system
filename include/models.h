#ifndef MODELS_H
#define MODELS_H

#include <time.h>

/* ─── Constants ─────────────────────────────────────────────────── */
#define MAX_STR         64
#define MAX_PHONE       16
#define MAX_ACC_NUM     13
#define DATA_DIR        "data/"
#define FILE_USERS      "data/users.csv"
#define FILE_ACCOUNTS   "data/accounts.csv"
#define FILE_TRANS      "data/transactions.csv"

/* Admin credentials (hardcoded, stored in memory only) */
#define ADMIN_USERNAME  "admin"
#define ADMIN_PASSWORD  "admin1234"

/* ─── Account type enum ──────────────────────────────────────────── */
typedef enum {
    ACC_SAVING = 0,
    ACC_FIXED  = 1
} AccountType;

/* ─── User record ────────────────────────────────────────────────── */
typedef struct {
    int  user_id;
    char username[MAX_STR];
    char password[MAX_STR];
    int  active;
} User;

/* ─── Bank account record ────────────────────────────────────────── */
typedef struct {
    char        account_number[MAX_ACC_NUM];
    char        first_name[MAX_STR];
    char        last_name[MAX_STR];
    char        phone[MAX_PHONE];
    AccountType type;
    double      balance;
    int         owner_id;
    int         active;
} Account;

/* ─── Transaction record ─────────────────────────────────────────── */
typedef enum {
    TXN_DEPOSIT   = 0,
    TXN_WITHDRAW  = 1,
    TXN_TRANSFER  = 2   /* ref_account holds the counterpart account */
} TxnType;

typedef struct {
    long        txn_id;
    char        account_number[MAX_ACC_NUM];  /* account this entry belongs to   */
    char        ref_account[MAX_ACC_NUM];     /* counterpart (transfer only)      */
    TxnType     type;
    double      amount;
    double      balance_after;
    time_t      timestamp;
} Transaction;

#endif /* MODELS_H */
