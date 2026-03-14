# Bank Management System

A modular C project implementing a CLI bank management system with file persistence.  
Built as a lab exercise (LAB 9) for Operating Systems coursework.

---

## Features

- **Admin** — add, delete, edit, list system users
- **User** — open/close accounts, deposit, withdraw, transfer, view history
- **CSV storage** — all data saved as human-readable `.csv` files
- **Nix environment** — reproducible dev shell for Linux, macOS, and WSL2
- **Unit tests** — 34 tests across user, account, and transaction modules

---

## Quick Start

```bash
# 1. Enter the Nix dev shell (installs gcc, make, gdb, valgrind)
nix-shell

# 2. Compile
make

# 3. Run
make run
```

Default admin credentials: `admin` / `admin1234`

---

## Documentation

| File | Description |
|------|-------------|
| [INSTALL.md](INSTALL.md) | Setup guide for Linux, macOS, and Windows (WSL2) |
| [MANUAL.md](MANUAL.md)   | Full user guide — all menus and features explained |

---

## Project Structure

```
bank-system/
├── src/                 C source files
│   ├── main.c
│   ├── menu.c
│   ├── auth.c
│   ├── user.c
│   ├── account.c
│   └── transaction.c
├── include/             Header files
│   ├── models.h         Shared structs and constants
│   └── *.h
├── data/                CSV data files (git-ignored)
├── tests/               Unit test suite
├── build/               Compiled .o files (auto-created)
├── Makefile
├── shell.nix
├── .gitignore
├── INSTALL.md
└── MANUAL.md
```

---

## Commands

```bash
make              # compile
make run          # compile and run
make test         # run all 34 unit tests
make clean        # remove build/ and binary
make clean-data   # delete data files (fresh start)
make valgrind     # run under valgrind
```

---

## Data Format

Data is stored as plain CSV — readable with `cat` or any text editor.

```
# users.csv
# user_id,username,password,active
1,john,secret,1

# accounts.csv
# account_number,first_name,last_name,phone,type,balance,owner_id,active
20260314001,Somchai,Sriwong,0812345678,0,5000.00,1,1

# transactions.csv
# txn_id,account_number,ref_account,type,amount,balance_after,timestamp
1,20260314001,-,0,5000.00,5000.00,1741910400
```

---

## Tech Stack

- Language: **C11**
- Compiler: **gcc** with `-Wall -Wextra -Wpedantic`
- Build: **GNU Make**
- Environment: **Nix** (`shell.nix`)
- Storage: **CSV text files**

---

## License

MIT
