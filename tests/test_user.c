/*
 * test_user.c - Unit tests for user.c
 */
#include <stdio.h>
#include <errno.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>

#include "test_runner.h"
#include "../include/models.h"
#include "../include/user.h"

static void cleanup(void) {
    remove(FILE_USERS);
}

/* ── Tests ───────────────────────────────────────────────────────── */

static void test_user_load_empty(void) {
    remove(FILE_USERS);
    FILE *fp = fopen(FILE_USERS, "wb"); fclose(fp);
    User *arr = NULL;
    int count = user_load_all(&arr);
    ASSERT_EQ(count, 0);
    ASSERT_NULL(arr);
    cleanup();
}

static void test_user_save_and_load(void) {
    User users[2] = {
        {1, "alice", "secret1", 1},
        {2, "bob",   "secret2", 1},
    };
    user_save_all(users, 2);

    User *arr = NULL;
    int count = user_load_all(&arr);
    ASSERT_EQ(count, 2);
    ASSERT_NOTNULL(arr);
    ASSERT_EQ(arr[0].user_id, 1);
    ASSERT_STR(arr[0].username, "alice");
    ASSERT_EQ(arr[0].active, 1);
    ASSERT_EQ(arr[1].user_id, 2);
    ASSERT_STR(arr[1].username, "bob");
    free(arr);
    cleanup();
}

static void test_user_find_by_name_found(void) {
    User users[3] = {
        {1, "alice",   "x", 1},
        {2, "charlie", "x", 1},
        {3, "dave",    "x", 0},
    };
    ASSERT_EQ(user_find_by_name(users, 3, "alice"),   0);
    ASSERT_EQ(user_find_by_name(users, 3, "charlie"), 1);
}

static void test_user_find_by_name_not_found(void) {
    User users[2] = {
        {1, "alice", "x", 1},
        {2, "dave",  "x", 0},
    };
    ASSERT_EQ(user_find_by_name(users, 2, "dave"),   -1);
    ASSERT_EQ(user_find_by_name(users, 2, "nobody"), -1);
}

static void test_user_next_id_empty(void) {
    ASSERT_EQ(user_next_id(NULL, 0), 1);
}

static void test_user_next_id_sequential(void) {
    User users[3] = {
        {1, "a", "x", 1},
        {5, "b", "x", 1},
        {3, "c", "x", 1},
    };
    ASSERT_EQ(user_next_id(users, 3), 6);
}

static void test_user_soft_delete(void) {
    User users[2] = {
        {1, "alice", "pass", 1},
        {2, "bob",   "pass", 1},
    };
    user_save_all(users, 2);

    User *arr = NULL;
    int count = user_load_all(&arr);
    ASSERT_EQ(count, 2);
    for (int i = 0; i < count; i++) {
        if (arr[i].user_id == 2) arr[i].active = 0;
    }
    user_save_all(arr, count);
    free(arr);

    arr = NULL;
    count = user_load_all(&arr);
    ASSERT_EQ(count, 2);
    ASSERT_EQ(arr[1].active, 0);
    ASSERT_EQ(user_find_by_name(arr, count, "bob"), -1);
    free(arr);
    cleanup();
}

static void test_user_overwrite_preserves_count(void) {
    User users[3] = {
        {1, "u1", "p", 1},
        {2, "u2", "p", 1},
        {3, "u3", "p", 1},
    };
    user_save_all(users, 3);

    User *arr = NULL;
    int count = user_load_all(&arr);
    ASSERT_EQ(count, 3);
    free(arr);
    cleanup();
}

/* ── main ────────────────────────────────────────────────────────── */

int main(void) {
    /* Ensure data/ exists (relative to where binary is run from) */
    if (mkdir("data", 0755) != 0 && errno != EEXIST) {
        perror("mkdir data");
    }
    remove(FILE_USERS);

    TEST_BEGIN();

    printf("[ User I/O ]\n");
    RUN_TEST(test_user_load_empty);
    RUN_TEST(test_user_save_and_load);
    RUN_TEST(test_user_overwrite_preserves_count);

    printf("\n[ User Lookup ]\n");
    RUN_TEST(test_user_find_by_name_found);
    RUN_TEST(test_user_find_by_name_not_found);

    printf("\n[ User ID Generation ]\n");
    RUN_TEST(test_user_next_id_empty);
    RUN_TEST(test_user_next_id_sequential);

    printf("\n[ Soft Delete ]\n");
    RUN_TEST(test_user_soft_delete);

    remove(FILE_USERS);
    TEST_SUMMARY();
}
