#ifndef TRANSACTION_H
#define TRANSACTION_H

#include "models.h"

/* Deposit money into an account. */
void txn_deposit(int owner_id);

/* Withdraw money from an account. */
void txn_withdraw(int owner_id);

/* Transfer money from one account to another existing account. */
void txn_transfer(int owner_id);

/* Show full transaction history for a given account_number. */
void txn_history(int owner_id);

/* ── Internal helpers ────────────────────────────────────────────── */

/*
 * txn_record_full: write one transaction row.
 * ref_acc is the counterpart account number (empty string "" for non-transfer).
 */
void txn_record_full(const char *acc_num, const char *ref_acc,
                     TxnType type, double amount, double bal_after);

/* Convenience wrapper for deposit/withdraw (no ref account). */
void txn_record(const char *acc_num, TxnType type,
                double amount, double bal_after);

/* Load all transactions from file. Returns count, caller must free(). */
int  txn_load_all(Transaction **arr);

/* Get the next transaction id. */
long txn_next_id(Transaction *arr, int count);

#endif /* TRANSACTION_H */
