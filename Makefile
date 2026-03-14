# ----------------------------------------------------------------------
#  Makefile - Bank Management System
# ----------------------------------------------------------------------

CC       = gcc
CFLAGS   = -Wall -Wextra -Wpedantic -std=c11 -g
TARGET   = bank
SRCDIR   = src
INCDIR   = include
DATADIR  = data
BUILDDIR = build
TESTDIR  = tests

SRCS = $(SRCDIR)/main.c \
       $(SRCDIR)/menu.c \
       $(SRCDIR)/auth.c \
       $(SRCDIR)/user.c \
       $(SRCDIR)/account.c \
       $(SRCDIR)/transaction.c

# Object files go into build/ not src/
OBJS = $(patsubst $(SRCDIR)/%.c, $(BUILDDIR)/%.o, $(SRCS))

# Test binaries
TEST_BINS = $(TESTDIR)/test_user \
            $(TESTDIR)/test_account \
            $(TESTDIR)/test_transaction

STUBS = $(TESTDIR)/test_stubs.c

# ----------------------------------------------------------------------
#  Main targets
# ----------------------------------------------------------------------

all: $(BUILDDIR) $(DATADIR) $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^
	@echo "Build OK -> ./$(TARGET)"

$(BUILDDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -I$(INCDIR) -c $< -o $@

$(BUILDDIR):
	mkdir -p $(BUILDDIR)

$(DATADIR):
	mkdir -p $(DATADIR)

run: all
	./$(TARGET)

clean:
	rm -rf $(BUILDDIR) $(TARGET)
	@echo "Clean OK"

clean-data:
	rm -f $(DATADIR)/*.dat
	@echo "Data files removed"

valgrind: all
	valgrind --leak-check=full --track-origins=yes ./$(TARGET)

# ----------------------------------------------------------------------
#  Test targets
# ----------------------------------------------------------------------

$(TESTDIR)/test_user: $(TESTDIR)/test_user.c $(SRCDIR)/user.c $(STUBS)
	$(CC) $(CFLAGS) -I$(INCDIR) $^ -o $@

$(TESTDIR)/test_account: $(TESTDIR)/test_account.c $(SRCDIR)/account.c $(STUBS)
	$(CC) $(CFLAGS) -I$(INCDIR) $^ -o $@

$(TESTDIR)/test_transaction: $(TESTDIR)/test_transaction.c \
                              $(SRCDIR)/transaction.c \
                              $(SRCDIR)/account.c $(STUBS)
	$(CC) $(CFLAGS) -I$(INCDIR) $^ -o $@

test-build: $(DATADIR) $(TEST_BINS)
	@echo "Test build OK"

# Run all tests from project root so data/ path resolves correctly
test: test-build
	@echo ""
	@echo "--- test_user ---"
	@./$(TESTDIR)/test_user

	@echo ""
	@echo "--- test_account ---"
	@./$(TESTDIR)/test_account

	@echo ""
	@echo "--- test_transaction ---"
	@./$(TESTDIR)/test_transaction

	@echo ""
	@echo "All tests complete"

clean-tests:
	rm -f $(TEST_BINS)
	@echo "Test binaries removed"

.PHONY: all run clean clean-data valgrind test test-build clean-tests
