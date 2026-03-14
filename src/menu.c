/*
 * menu.c - CLI menus and shared input utilities
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/menu.h"
#include "../include/auth.h"
#include "../include/user.h"
#include "../include/account.h"
#include "../include/transaction.h"

/* ── Utilities ───────────────────────────────────────────────────── */

void clear_screen(void) {
    /* ANSI escape: move cursor to top-left and erase screen.
     * Avoids system() entirely, so no -Wunused-result warning. */
    fputs("\033[H\033[2J", stdout);
    fflush(stdout);
}

void press_enter(void) {
    printf("\n  Press Enter to continue...");
    while (getchar() != '\n');
}

int read_int(const char *prompt) {
    int val;
    char buf[32];
    while (1) {
        printf("%s", prompt);
        if (fgets(buf, sizeof(buf), stdin)) {
            if (sscanf(buf, "%d", &val) == 1) return val;
        }
        printf("  [!] Please enter a valid number\n");
    }
}

void read_line(const char *prompt, char *buf, int maxlen) {
    printf("%s", prompt);
    if (fgets(buf, maxlen, stdin)) {
        buf[strcspn(buf, "\n")] = '\0';
    }
}

/* ── Main Menu ───────────────────────────────────────────────────── */

void menu_main(void) {
    int choice;
    while (1) {
        clear_screen();
        printf("\n================================\n");
        printf("    BANK MANAGEMENT SYSTEM\n");
        printf("================================\n\n");
        printf("  1. Admin Login\n");
        printf("  2. User Login\n");
        printf("  3. Exit\n");

        choice = read_int("\n  Select: ");
        switch (choice) {
            case 1:
                if (auth_admin_login()) menu_admin();
                break;
            case 2: {
                User u;
                if (auth_user_login(&u)) menu_user(&u);
                break;
            }
            case 3:
                printf("\n  Goodbye.\n\n");
                return;
            default:
                printf("  [!] Invalid option\n");
                press_enter();
        }
    }
}

/* ── Admin Menu ──────────────────────────────────────────────────── */

void menu_admin(void) {
    int choice;
    while (1) {
        clear_screen();
        printf("\n================================\n");
        printf("         ADMIN MENU\n");
        printf("================================\n\n");
        printf("  1. Add User\n");
        printf("  2. Delete User\n");
        printf("  3. Edit User\n");
        printf("  4. List Users\n");
        printf("  5. List All Accounts\n");
        printf("  0. Logout\n");

        choice = read_int("\n  Select: ");
        switch (choice) {
            case 1: user_add();        break;
            case 2: user_delete();     break;
            case 3: user_edit();       break;
            case 4: user_list();       break;
            case 5: account_list(-1);  break;
            case 0: return;
            default:
                printf("  [!] Invalid option\n");
                press_enter();
        }
    }
}

/* ── User Menu ───────────────────────────────────────────────────── */

void menu_user(User *u) {
    int choice;
    while (1) {
        clear_screen();
        printf("\n================================\n");
        printf("    USER MENU  [%s]\n", u->username);
        printf("================================\n\n");
        printf("  1. Open Account\n");
        printf("  2. Close Account\n");
        printf("  3. Edit Account\n");
        printf("  4. Search Account\n");
        printf("  5. List My Accounts\n");
        printf("  6. Deposit\n");
        printf("  7. Withdraw\n");
        printf("  8. Transaction History\n");
        printf("  9. Transfer\n");
        printf("  0. Logout\n");

        choice = read_int("\n  Select: ");
        switch (choice) {
            case 1: account_create(u->user_id);  break;
            case 2: account_delete(u->user_id);  break;
            case 3: account_edit(u->user_id);    break;
            case 4: account_search();            break;
            case 5: account_list(u->user_id);    break;
            case 6: txn_deposit(u->user_id);     break;
            case 7: txn_withdraw(u->user_id);    break;
            case 8: txn_history(u->user_id);     break;
            case 9: txn_transfer(u->user_id);    break;
            case 0: return;
            default:
                printf("  [!] Invalid option\n");
                press_enter();
        }
    }
}