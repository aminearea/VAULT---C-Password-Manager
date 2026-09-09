<p align="center">
  <img src="https://img.shields.io/badge/Language-C%2B%2B-blue?style=for-the-badge" alt="C++">
  <img src="https://img.shields.io/badge/Security-AES--256--CBC-green?style=for-the-badge" alt="Security">
  <img src="https://img.shields.io/badge/Platform-Windows%20%7C%20Linux-lightgrey?style=for-the-badge" alt="Platform">
</p>

<pre align="center">
██╗   ██╗ █████╗ ██╗   ██╗██╗  ████████╗
██║   ██║██╔══██╗██║   ██║██║  ╚══██╔══╝
██║   ██║███████║██║   ██║██║     ██║   
╚██╗ ██╔╝██╔══██║██║   ██║██║     ██║   
 ╚████╔╝ ██║  ██║╚██████╔╝███████╗██║   
  ╚═══╝  ╚═╝  ╚═╝ ╚═════╝ ╚══════╝╚═╝   
</pre>

<h2 align="center">🔐 A Modern C++ Command-Line Password Manager</h2>

<p align="center"><b>AES-256-CBC Encryption • Multi-User Support • Polished Terminal UI</b></p>

---

<h2>🌟 Features</h2>

- <b>Military-Grade Encryption:</b> Uses AES-256-CBC via OpenSSL to secure all vault data.
- <b>Multi-User Support:</b> Create multiple independent vault files, each protected by a unique User ID and Master Password.
- <b>Interactive Terminal UI:</b> A beautiful, colorized interface with animated banners, box-drawn menus, and progress bars.
- <b>Key Derivation:</b> Uses PBKDF2-HMAC-SHA256 with 10,000 iterations to derive encryption keys.
- <b>Full CRUD Operations:</b> Add, List, Search, Update, and Delete stored entries.
- <b>Password Generator:</b> Generates strong, random passwords of customizable length (8-64 characters).
- <b>Session Management:</b> Save & Logout functionality allows switching between accounts without restarting.

---

<h2>🛠️ Prerequisites</h2>

<h3>1. MSYS2 / MinGW</h3>
Install [MSYS2](https://www.msys2.org/), then update packages:
```bash
pacman -Syu
