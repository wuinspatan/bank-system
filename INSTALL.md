# INSTALL – Setting Up the Development Environment

This guide covers setup on **Linux**, **macOS**, and **Windows (WSL2)** using the Nix package manager.

---

## Prerequisites — Install Nix

Nix is the only prerequisite. It handles gcc, make, gdb, and valgrind automatically.

### Linux / macOS

```bash
sh <(curl -L https://nixos.org/nix/install) --daemon
```

After installation, restart your terminal or run:

```bash
source ~/.nix-profile/etc/profile.d/nix.sh
```

### Windows — WSL2

1. Install WSL2 from PowerShell (run as Administrator):

```powershell
wsl --install
```

2. Open your WSL terminal (Ubuntu), then install Nix:

```bash
sh <(curl -L https://nixos.org/nix/install) --no-daemon
source ~/.nix-profile/etc/profile.d/nix.sh
```

> **NixOS users** — Nix is already installed. Skip this section.

---

## Getting the Project

### Option A — Clone from GitHub

```bash
git clone https://github.com/<your-username>/bank-system.git
cd bank-system
```

### Option B — Download ZIP

Download and unzip, then:

```bash
cd bank-system
```

---

## Enter the Nix Development Shell

```bash
nix-shell
```

This automatically installs:

| Tool       | Purpose               |
|------------|-----------------------|
| gcc        | C compiler            |
| gnumake    | build system          |
| gdb        | debugger              |
| valgrind   | memory leak checker   |
| clang-tools| LSP / code formatting |

You will see:

```
Bank Management System - Dev Shell
  make          -> compile
  make run      -> compile & run
  make clean    -> remove build files
  make valgrind -> run with valgrind
```

---

## Build and Run

```bash
# Compile
make

# Compile and run immediately
make run

# Run after compiling
./bank
```

---

## Run Tests

```bash
make test
```

Expected output:

```
--- test_user ---
  test_user_load_empty          PASS
  ...
Total  : 8
Failed : 0

--- test_account ---
  ...
Total  : 10
Failed : 0

--- test_transaction ---
  ...
Total  : 16
Failed : 0

All tests complete
```

---

## Other Commands

```bash
make clean        # remove build/ directory and bank binary
make clean-data   # delete all .csv data files (fresh start)
make valgrind     # run program under valgrind memory checker
make clean-tests  # remove compiled test binaries
```

---

## Project Structure

```
bank-system/
├── src/              C source files
├── include/          Header files
├── data/             Persistent CSV data (git-ignored)
├── tests/            Unit test suite
├── build/            Compiled .o files (auto-created, git-ignored)
├── Makefile
├── shell.nix         Nix dev environment
├── .gitignore
├── INSTALL.md        This file
└── MANUAL.md         User guide
```

---

## Troubleshooting

| Problem | Solution |
|---------|----------|
| `nix-shell: command not found` | Nix not installed, or shell not sourced — run `source ~/.nix-profile/etc/profile.d/nix.sh` |
| `make: command not found` | You are not inside `nix-shell` — run `nix-shell` first |
| Old `.dat` files cause errors | Run `make clean-data` to remove legacy binary files |
| Program crashes on first run | Run `make clean-data` to ensure data/ only has `.csv` files |
| `valgrind` not found on macOS | valgrind has limited macOS support — use Linux/WSL2 instead |
