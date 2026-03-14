#ifndef MENU_H
#define MENU_H

#include "models.h"

/*
 * menu.h – Top-level menu entry points
 */

/* Show the main menu and dispatch to admin/user login. */
void menu_main(void);

/* Admin sub-menu (called after successful admin login). */
void menu_admin(void);

/* User sub-menu (called after successful user login). */
void menu_user(User *u);

/* ── Utility ─────────────────────────────────────────────────────── */
void clear_screen(void);
void press_enter(void);
int  read_int(const char *prompt);
void read_line(const char *prompt, char *buf, int maxlen);

#endif /* MENU_H */
