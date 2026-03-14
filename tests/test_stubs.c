/*
 * test_stubs.c – Stub implementations of menu.c UI functions.
 * Linked only during testing so modules compile without real I/O.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/menu.h"
#include "../include/auth.h"

/* No-op stubs: tests bypass interactive functions directly */
void clear_screen(void) {}
void press_enter(void)  {}

int read_int(const char *prompt) {
    (void)prompt;
    return 0;
}

void read_line(const char *prompt, char *buf, int maxlen) {
    (void)prompt;
    if (buf && maxlen > 0) buf[0] = '\0';
}

void read_password(char *buf, int maxlen) {
    if (buf && maxlen > 0) buf[0] = '\0';
}
