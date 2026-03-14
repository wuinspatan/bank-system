#ifndef USER_H
#define USER_H

#include "models.h"

/*
 * user.h – Admin-side user management
 */

/* Add a new user (prompts for username + password). */
void user_add(void);

/* Delete a user by user_id. */
void user_delete(void);

/* Edit username/password of an existing user. */
void user_edit(void);

/* Print all active users. */
void user_list(void);

/* ── Internal helpers (also used by auth) ────────────────────────── */

/* Load all users from file into *arr. Returns count, caller must free(). */
int  user_load_all(User **arr);

/* Save (overwrite) the full users array to file. */
void user_save_all(User *arr, int count);

/* Find a user by username. Returns index in arr or -1. */
int  user_find_by_name(User *arr, int count, const char *username);

/* Generate a new unique user_id. */
int  user_next_id(User *arr, int count);

#endif /* USER_H */
