/*
 * user.c - Admin-side CRUD for system users.
 *
 * Storage format (data/users.csv):
 *   user_id,username,password,active
 *   1,alice,secret1,1
 *   2,bob,secret2,0
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/user.h"
#include "../include/auth.h"
#include "../include/menu.h"

/* ── File I/O ────────────────────────────────────────────────────── */

int user_load_all(User **arr) {
    FILE *fp = fopen(FILE_USERS, "r");
    if (!fp) { *arr = NULL; return 0; }

    /* Count lines to pre-allocate */
    int capacity = 16;
    *arr = (User *)malloc((size_t)capacity * sizeof(User));
    if (!*arr) { fclose(fp); return 0; }

    int count = 0;
    char line[256];
    while (fgets(line, sizeof(line), fp)) {
        /* Skip comment/header lines */
        if (line[0] == '#' || line[0] == '\n') continue;

        User u;
        memset(&u, 0, sizeof(u));
        if (sscanf(line, "%d,%63[^,],%63[^,],%d",
                   &u.user_id, u.username, u.password, &u.active) != 4)
            continue;

        if (count >= capacity) {
            capacity *= 2;
            User *tmp = (User *)realloc(*arr, (size_t)capacity * sizeof(User));
            if (!tmp) { free(*arr); *arr = NULL; fclose(fp); return 0; }
            *arr = tmp;
        }
        (*arr)[count++] = u;
    }
    fclose(fp);
    if (count == 0) { free(*arr); *arr = NULL; }
    return count;
}

void user_save_all(User *arr, int count) {
    FILE *fp = fopen(FILE_USERS, "w");
    if (!fp) { perror("Cannot open users file"); return; }

    fprintf(fp, "# user_id,username,password,active\n");
    for (int i = 0; i < count; i++) {
        fprintf(fp, "%d,%s,%s,%d\n",
                arr[i].user_id,
                arr[i].username,
                arr[i].password,
                arr[i].active);
    }
    fclose(fp);
}

/* ── Helpers ─────────────────────────────────────────────────────── */

int user_find_by_name(User *arr, int count, const char *username) {
    for (int i = 0; i < count; i++) {
        if (arr[i].active && strcmp(arr[i].username, username) == 0)
            return i;
    }
    return -1;
}

static int find_by_id(User *arr, int count, int uid) {
    for (int i = 0; i < count; i++) {
        if (arr[i].user_id == uid) return i;
    }
    return -1;
}

int user_next_id(User *arr, int count) {
    int max = 0;
    for (int i = 0; i < count; i++)
        if (arr[i].user_id > max) max = arr[i].user_id;
    return max + 1;
}

static void safe_copy(char *dst, const char *src, size_t dstsize) {
    size_t n = strlen(src);
    if (n >= dstsize) n = dstsize - 1;
    memcpy(dst, src, n);
    dst[n] = '\0';
}

/* ── Shared: print active user table ────────────────────────────── */

static void print_user_table(User *arr, int count) {
    printf("  %-8s %-32s\n", "UserID", "Username");
    printf("  %-8s %-32s\n", "------", "--------");
    int shown = 0;
    for (int i = 0; i < count; i++) {
        if (!arr[i].active) continue;
        printf("  %-8d %-32s\n", arr[i].user_id, arr[i].username);
        shown++;
    }
    if (shown == 0) printf("  (no users)\n");
}

/* ── CRUD ────────────────────────────────────────────────────────── */

void user_add(void) {
    printf("\n-- Add User --\n");
    char uname[MAX_STR], pwd[MAX_STR], pwd2[MAX_STR];

    read_line("  Username: ", uname, MAX_STR);
    if (strlen(uname) == 0) {
        printf("  [!] Username must not be empty\n");
        press_enter(); return;
    }

    User *arr = NULL;
    int count = user_load_all(&arr);
    if (user_find_by_name(arr, count, uname) >= 0) {
        printf("  [!] Username '%s' already exists\n", uname);
        free(arr); press_enter(); return;
    }

    printf("  Password: ");
    read_password(pwd, MAX_STR);
    printf("  Confirm : ");
    read_password(pwd2, MAX_STR);
    if (strcmp(pwd, pwd2) != 0) {
        printf("  [!] Passwords do not match\n");
        free(arr); press_enter(); return;
    }

    User *newarr = (User *)realloc(arr, (size_t)(count + 1) * sizeof(User));
    if (!newarr) { free(arr); return; }
    arr = newarr;

    User *u = &arr[count];
    memset(u, 0, sizeof(User));
    u->user_id = user_next_id(arr, count);
    safe_copy(u->username, uname, MAX_STR);
    safe_copy(u->password, pwd,   MAX_STR);
    u->active = 1;
    count++;

    user_save_all(arr, count);
    free(arr);
    printf("  User '%s' (ID: %d) added\n", uname, u->user_id);
    press_enter();
}

void user_delete(void) {
    printf("\n-- Delete User --\n");
    User *arr = NULL;
    int count = user_load_all(&arr);
    print_user_table(arr, count);
    printf("\n");
    int uid = read_int("  User ID to delete: ");
    int idx = find_by_id(arr, count, uid);

    if (idx < 0 || !arr[idx].active) {
        printf("  [!] User ID %d not found\n", uid);
        free(arr); press_enter(); return;
    }

    printf("  Confirm delete '%s'? (y/n): ", arr[idx].username);
    char c = (char)getchar(); while (getchar() != '\n');
    if (c != 'y' && c != 'Y') {
        printf("  Cancelled\n"); free(arr); press_enter(); return;
    }

    arr[idx].active = 0;
    user_save_all(arr, count);
    free(arr);
    printf("  User ID %d deleted\n", uid);
    press_enter();
}

void user_edit(void) {
    printf("\n-- Edit User --\n");
    User *arr = NULL;
    int count = user_load_all(&arr);
    print_user_table(arr, count);
    printf("\n");
    int uid = read_int("  User ID to edit: ");
    int idx = find_by_id(arr, count, uid);

    if (idx < 0 || !arr[idx].active) {
        printf("  [!] User ID %d not found\n", uid);
        free(arr); press_enter(); return;
    }

    printf("  Current username: %s\n", arr[idx].username);
    char newname[MAX_STR];
    read_line("  New username (Enter to skip): ", newname, MAX_STR);
    if (strlen(newname) > 0) {
        int dup = user_find_by_name(arr, count, newname);
        if (dup >= 0 && dup != idx) {
            printf("  [!] Username already taken\n");
            free(arr); press_enter(); return;
        }
        safe_copy(arr[idx].username, newname, MAX_STR);
    }

    printf("  Change password? (y/n): ");
    char c = (char)getchar(); while (getchar() != '\n');
    if (c == 'y' || c == 'Y') {
        char pwd[MAX_STR], pwd2[MAX_STR];
        printf("  New password: ");
        read_password(pwd, MAX_STR);
        printf("  Confirm     : ");
        read_password(pwd2, MAX_STR);
        if (strcmp(pwd, pwd2) != 0) {
            printf("  [!] Passwords do not match\n");
            free(arr); press_enter(); return;
        }
        safe_copy(arr[idx].password, pwd, MAX_STR);
    }

    user_save_all(arr, count);
    free(arr);
    printf("  User ID %d updated\n", uid);
    press_enter();
}

void user_list(void) {
    printf("\n-- User List --\n");
    User *arr = NULL;
    int count = user_load_all(&arr);
    print_user_table(arr, count);

    int shown = 0;
    for (int i = 0; i < count; i++) if (arr[i].active) shown++;
    printf("  Total: %d\n", shown);

    free(arr);
    press_enter();
}
