# VAULT - C++ Password Manager

A modern, secure, and user-friendly command-line password manager built in C++ with AES-256-CBC encryption. VAULT features a polished terminal UI with full color support, ASCII art, typewriter animations, and multi-user support.

---

## Features

- **Military-Grade Encryption:** Uses AES-256-CBC via OpenSSL to secure all vault data.
- **Multi-User Support:** Create multiple independent vault files, each protected by a unique User ID and Master Password.
- **Interactive Terminal UI:** A beautiful, colorized interface with animated banners, box-drawn menus, and progress bars.
- **Key Derivation:** Uses PBKDF2-HMAC-SHA256 with 10,000 iterations to derive encryption keys from your credentials.
- **Full CRUD Operations:** Add, List, Search, Update, and Delete stored entries (Services, Usernames, Passwords, Notes).
- **Password Generator:** Generates strong, random passwords of customizable length (8-64 characters).
- **Session Management:** Save & Logout functionality allows users to switch between accounts without restarting the program.
- **Robust Input Handling:** Sanitized inputs and buffered output to prevent Windows-specific CRLF input bugs.

---

## Prerequisites

### 1. MSYS2 / MinGW

This project uses the MSYS2 environment with the UCRT64 toolchain.

- Download and install [MSYS2](https://www.msys2.org/).
- Update packages:
```bash
pacman -Syu
