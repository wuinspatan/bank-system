# MANUAL – Bank Management System User Guide

---

## Overview

The Bank Management System is a CLI application written in C.  
It supports two roles: **Admin** and **User**.

| Role  | Responsibilities |
|-------|-----------------|
| Admin | Manage system user accounts |
| User  | Manage bank accounts, deposit, withdraw, transfer |

All data is stored in human-readable CSV files inside `data/`.

---

## Starting the Program

```bash
make run
# or
./bank
```

You will see the main menu:

```
================================
    BANK MANAGEMENT SYSTEM
================================

  1. Admin Login
  2. User Login
  3. Exit

  Select:
```

---

## Admin Section

### Default Admin Credentials

```
Username : admin
Password : admin1234
```

### Admin Menu

```
================================
         ADMIN MENU
================================

  1. Add User
  2. Delete User
  3. Edit User
  4. List Users
  5. List All Accounts
  0. Logout
```

---

### 1. Add User

Creates a new user account that can log in to the system.

```
-- Add User --
  Username: john
  Password: ****
  Confirm : ****
  User 'john' (ID: 1) added
```

- Username must be unique
- Password is masked during input
- Confirm password must match

---

### 2. Delete User

Soft-deletes a user (data is kept but account is deactivated).

```
-- Delete User --
  UserID   Username
  ------   --------
  1        john
  2        jane

  User ID to delete: 2
  Confirm delete 'jane'? (y/n): y
  User ID 2 deleted
```

---

### 3. Edit User

Change a user's username or password.

```
-- Edit User --
  UserID   Username
  ------   --------
  1        john

  User ID to edit: 1
  Current username: john
  New username (Enter to skip): johnny
  Change password? (y/n): n
  User ID 1 updated
```

---

### 4. List Users

Shows all active user accounts.

```
-- User List --
  UserID   Username
  ------   --------
  1        john
  2        jane
  Total: 2
```

---

### 5. List All Accounts

Shows every bank account in the system (all users).

---

## User Section

### User Login

```
  1. Admin Login
  2. User Login   <-- select this

-- User Login --
  Username: john
  Password: ****
  Welcome, john
```

### User Menu

```
================================
    USER MENU  [john]
================================

  1. Open Account
  2. Close Account
  3. Edit Account
  4. Search Account
  5. List My Accounts
  6. Deposit
  7. Withdraw
  8. Transaction History
  9. Transfer
  0. Logout
```

---

### 1. Open Account

Create a new bank account. The account number is generated automatically.

```
-- Open New Account --
  Account number assigned: 20260314001
  First name : Somchai
  Last name  : Sriwong
  Phone      : 0812345678
  Account type (1=Saving, 2=Fixed): 1
  Initial deposit: 5000
  Account 20260314001 opened
```

**Account types:**

| Type    | Description       |
|---------|-------------------|
| Saving  | Standard savings  |
| Fixed   | Fixed deposit     |

---

### 2. Close Account

Deactivates a bank account. Your accounts are listed automatically before asking.

```
-- Close Account --
  Account No.   First Name     Last Name       Balance
  20260314001   Somchai        Sriwong         5000.00

  Account number to close: 20260314001
  Confirm close account 20260314001? (y/n): y
  Account 20260314001 closed
```

---

### 3. Edit Account

Update first name, last name, or phone number.

```
-- Edit Account --
  Account No.   First Name     ...
  20260314001   Somchai        ...

  Account number to edit: 20260314001
  Current first name: Somchai
  New first name (Enter to skip): Somying
  ...
  Account updated
```

---

### 4. Search Account

Look up any account by account number.

```
-- Search Account --
  Account number: 20260314001

  Account   : 20260314001
  Name      : Somchai Sriwong
  Phone     : 0812345678
  Type      : Saving
  Balance   : 5000.00
```

---

### 5. List My Accounts

Shows all open accounts belonging to the logged-in user.

```
-- My Accounts --
  Account No.   First Name     Last Name      Type     Balance
  20260314001   Somchai        Sriwong        Saving   5000.00
  20260314002   Somchai        Sriwong        Fixed   10000.00
  Total: 2
```

---

### 6. Deposit

Add money to one of your accounts.

```
-- Deposit --
  Account No.   First Name     ...    Balance
  20260314001   Somchai        ...    5000.00

  Account number: 20260314001
  Current balance: 5000.00
  Amount to deposit: 1000
  Deposited 1000.00  |  New balance: 6000.00
```

---

### 7. Withdraw

Remove money from one of your accounts.

```
-- Withdraw --
  Account No.   First Name     ...    Balance
  20260314001   Somchai        ...    6000.00

  Account number: 20260314001
  Current balance: 6000.00
  Amount to withdraw: 500
  Withdrawn 500.00  |  New balance: 5500.00
```

- Cannot withdraw more than the current balance

---

### 8. Transaction History

View the complete transaction log for one of your accounts.

```
-- Transaction History --
  Account No.   ...  Balance
  20260314001   ...  5500.00

  Account number: 20260314001

  TxID   Type        Amount   Ref Account   Timestamp
  -----  ---------   -------  ------------  ---------------------
  1      Deposit     5000.00  -             2026-03-14 10:00:00
  2      Deposit     1000.00  -             2026-03-14 10:05:00
  3      Withdraw     500.00  -             2026-03-14 10:10:00
  Total: 3 transactions
```

---

### 9. Transfer

Send money from your account to any other existing account.

```
-- Transfer --
  [Your accounts]
  Account No.   First Name   Balance
  20260314001   Somchai      5500.00

  From account number: 20260314001
  Source balance: 5500.00

  To account number  : 20260314002
  Destination : Malee Jaidee (Account: 20260314002)
  Amount to transfer: 1000

  Confirm transfer 1000.00 from 20260314001 to Malee Jaidee? (y/n): y

  Transfer complete
  20260314001  new balance: 4500.00
  20260314002  new balance: 11000.00
```

- Transfer records appear in both accounts' transaction history
- Cannot transfer to the same account
- Cannot transfer more than the available balance

---

## Data Files

All data is saved as plain-text CSV in the `data/` directory.

```
data/
├── users.csv         system user accounts
├── accounts.csv      bank accounts
└── transactions.csv  complete transaction log
```

You can inspect files at any time:

```bash
cat data/users.csv
cat data/accounts.csv
cat data/transactions.csv
```

Example `users.csv`:

```
# user_id,username,password,active
1,john,secret,1
2,jane,pass123,1
```

Example `accounts.csv`:

```
# account_number,first_name,last_name,phone,type,balance,owner_id,active
20260314001,Somchai,Sriwong,0812345678,0,4500.00,1,1
```

Example `transactions.csv`:

```
# txn_id,account_number,ref_account,type,amount,balance_after,timestamp
1,20260314001,-,0,5000.00,5000.00,1741910400
2,20260314001,20260314002,2,1000.00,4000.00,1741910500
```

**Type codes:** `0` = Deposit · `1` = Withdraw · `2` = Transfer

---

## Quick Reference

| Action           | Menu Option |
|------------------|-------------|
| Login as admin   | Main → 1    |
| Login as user    | Main → 2    |
| Add user         | Admin → 1   |
| Open account     | User → 1    |
| Deposit          | User → 6    |
| Withdraw         | User → 7    |
| View history     | User → 8    |
| Transfer         | User → 9    |
| Logout           | Any → 0     |
