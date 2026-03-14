#ifndef ACCOUNT_H
#define ACCOUNT_H

#include "models.h"

/*
 * account.h – Bank account management
 */

/* Create a new bank account (owned by the logged-in user). */
void account_create(int owner_id);

/* Close (soft-delete) an account. */
void account_delete(int owner_id);

/* Edit account details. */
void account_edit(int owner_id);

/* Search account by account_number. */
void account_search(void);

/* List all accounts owned by owner_id (pass -1 for admin view all). */
void account_list(int owner_id);

/* ── Internal helpers ────────────────────────────────────────────── */

int  account_load_all(Account **arr);
void account_save_all(Account *arr, int count);
int  account_find(Account *arr, int count, const char *acc_num);
void account_generate_number(char *buf);  /* generates unique acc num */
int  account_next_seq(Account *arr, int count);

#endif /* ACCOUNT_H */
