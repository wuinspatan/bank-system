/*
 * main.c – Entry point for the Bank Management System.
 *
 * Responsibilities:
 *  1. Ensure data/ directory and .dat files exist.
 *  2. Launch the main menu loop.
 */
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <errno.h>

#include "../include/menu.h"

/*
 * ensure_data_files: create the data/ directory and empty .dat files
 * if they do not already exist.  This prevents fopen("rb") from failing
 * on the very first run.
 */
static void ensure_data_files(void) {
    /* Create data/ directory (ignore error if it already exists) */
#ifdef _WIN32
    mkdir(DATA_DIR);
#else
    mkdir(DATA_DIR, 0755);
#endif

    /* Touch each data file so fopen("rb") won't fail */
    const char *files[] = { FILE_USERS, FILE_ACCOUNTS, FILE_TRANS };
    for (int i = 0; i < 3; i++) {
        FILE *fp = fopen(files[i], "ab");   /* "ab" creates if not exists */
        if (fp) fclose(fp);
    }
}

int main(void) {
    ensure_data_files();
    menu_main();
    return EXIT_SUCCESS;
}
