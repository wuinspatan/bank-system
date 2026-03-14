/*
 * auth.c - Authentication for admin and regular users.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

#include "../include/auth.h"
#include "../include/user.h"
#include "../include/menu.h"

void read_password(char *buf, int maxlen) {
    struct termios oldt, newt;
    int idx = 0;
    int c;

    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= (tcflag_t)~(ECHO | ICANON);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    while ((c = getchar()) != '\n' && c != EOF && idx < maxlen - 1) {
        if (c == 127 || c == '\b') {
            if (idx > 0) { idx--; printf("\b \b"); fflush(stdout); }
        } else {
            buf[idx++] = (char)c;
            printf("*");
            fflush(stdout);
        }
    }
    buf[idx] = '\0';
    printf("\n");

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
}

int auth_admin_login(void) {
    char uname[MAX_STR];
    char pwd[MAX_STR];
    int attempts = 3;

    while (attempts--) {
        printf("\n-- Admin Login --\n");
        read_line("  Username: ", uname, MAX_STR);
        printf("  Password: ");
        read_password(pwd, MAX_STR);

        if (strcmp(uname, ADMIN_USERNAME) == 0 &&
            strcmp(pwd,   ADMIN_PASSWORD) == 0) {
            printf("  Login successful\n");
            press_enter();
            return 1;
        }
        printf("  Invalid credentials (%d attempt(s) left)\n", attempts);
    }
    printf("  Login failed\n");
    press_enter();
    return 0;
}

int auth_user_login(User *out) {
    char uname[MAX_STR];
    char pwd[MAX_STR];
    int  attempts = 3;

    while (attempts--) {
        printf("\n-- User Login --\n");
        read_line("  Username: ", uname, MAX_STR);
        printf("  Password: ");
        read_password(pwd, MAX_STR);

        User *arr = NULL;
        int count = user_load_all(&arr);
        int idx   = user_find_by_name(arr, count, uname);

        if (idx >= 0 && arr[idx].active &&
            strcmp(arr[idx].password, pwd) == 0) {
            *out = arr[idx];
            free(arr);
            printf("  Welcome, %s\n", out->username);
            press_enter();
            return 1;
        }
        free(arr);
        printf("  Invalid credentials (%d attempt(s) left)\n", attempts);
    }
    printf("  Login failed\n");
    press_enter();
    return 0;
}
