# 📡 IPv6-Compliant FTP Server
## 📘 Overview

This project implements a fully **IPv6-compliant**, **cross-platform**, **active-mode** FTP server in C++. It supports multiple standard FTP commands and is compatible with the built-in FTP client in Windows 11 using IPv6.

This server uses low-level BSD sockets and supports concurrent control/data channel handling, passive fallback, and essential error detection and command parsing. It adheres to **RFC 959** and **RFC 2428**.

---

## ✅ Implemented Features

- ✅ **Cross-platform**: Works on Windows, Linux, and macOS
- ✅ **IPv6 compliant**: Uses `AF_INET6`, `sockaddr_in6`, `inet_pton`, and `getaddrinfo`
- ✅ **Active mode (PORT/EPRT)** for data transfer
- ✅ **Supports essential FTP commands**:
  - `USER`, `PASS` – with static user credentials
  - `SYST` – system info
  - `TYPE` – binary (I) and ASCII (A)
  - `EPRT`, `PORT` – client-initiated data connection
  - `LIST` – returns directory listing (uses platform-specific `dir` or `ls`)
  - `RETR` – file download (ASCII or binary)
  - `QUIT` – clean disconnection
  - `OPTS`, `STOR`, `CWD` – correctly return `502 command not implemented`

- ✅ Handles both **ASCII** and **Binary** file transfers
- ✅ Graceful disconnection and logging
- ✅ Debugging output for testing and marking

---

## 🔐 Credentials

- **Username**: `napoleon`  
- **Password**: `342`

---

## 🧱 Technologies Used

- **C++17**
- **BSD Sockets API**
- **Winsock2** (Windows)
- **POSIX networking** (Linux/macOS)
- **Makefile** for cross-platform builds
- **IPv6 utilities**: `getaddrinfo()`, `sockaddr_in6`, `inet_pton()`, `getnameinfo()`

---

# ⚙️ Build & Run Instructions

```bash
make
server.exe           # Uses default port 1234

