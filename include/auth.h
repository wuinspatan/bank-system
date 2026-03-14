#ifndef AUTH_H
#define AUTH_H

#include "models.h"

/*
 * auth.h – Authentication helpers
 */

/* Verify admin credentials. Returns 1 on success, 0 on failure. */
int  auth_admin_login(void);

/*
 * Verify a regular user's credentials.
 * On success, copies the matched User into *out and returns 1.
 * Returns 0 on failure.
 */
int  auth_user_login(User *out);

/* Simple password masking input (reads until Enter, shows '*'). */
void read_password(char *buf, int maxlen);

#endif /* AUTH_H */
