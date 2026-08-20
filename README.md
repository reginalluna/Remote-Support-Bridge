# Windows Remote Support

A consent-first remote-support controller with local auditing and hand-offs to established RDP, SSH/SFTP and VNC clients.

The redesigned controller is available for **Windows, macOS and Ubuntu/Linux**. Historical Windows/MFC material under `client/` and `server/` remains separate for academic comparison and is not linked into the current application.

> Use the software only on systems you own or are explicitly authorised to administer or test.

## Release

**v0.1.0** is the first public release.

Release files:

```text
Windows-Remote-Support-x86.exe
Windows-Remote-Support-x64.exe
Windows-Remote-Support-x86.zip
Windows-Remote-Support-x64.zip
Windows-Remote-Support-macOS.dmg
Windows-Remote-Support-Linux-x86_64.AppImage
```

- Windows: native x86 and x64 C++20 controllers.
- macOS: `Windows Remote Support.app` distributed inside the DMG.
- Ubuntu/Linux: x86_64 AppImage.

The macOS and Linux builds package their Python/Tk runtime, so Python is not required on the destination computer.

## What it can open

Enter a remote hostname or IP address, start an explicitly consented support session, then choose a connection type:

- **RDP desktop** — Windows and RDP-enabled Linux desktops;
- **SSH terminal** — macOS, Ubuntu/Linux and other SSH servers;
- **SFTP files** — authenticated file transfer to macOS, Ubuntu/Linux and other SFTP servers;
- **VNC desktop** — macOS Screen Sharing and VNC-enabled Linux desktops.

The controller validates the target and records the hand-off before opening the selected client. Remote passwords are not stored by this application.

## Quick start

1. Download the file for the computer running the controller:
   - Windows 64-bit: `Windows-Remote-Support-x64.exe`
   - Windows 32-bit: `Windows-Remote-Support-x86.exe`
   - macOS: `Windows-Remote-Support-macOS.dmg`
   - Ubuntu/Linux x86_64: `Windows-Remote-Support-Linux-x86_64.AppImage`
2. Run the controller.
3. Enter the remote computer hostname or IP address.
4. Select **Start consented session** and approve the local session.
5. Select **RDP desktop**, **SSH terminal**, **SFTP files** or **VNC desktop**.
6. Authenticate in the remote-access client opened by your operating system.
7. Select **End session** when finished.

### macOS

Open the DMG and launch **Windows Remote Support.app**. macOS may warn that the first release is unsigned; only run a copy obtained from this repository's release page.

### Ubuntu/Linux

Make the AppImage executable if your desktop does not do this automatically:

```bash
chmod +x Windows-Remote-Support-Linux-x86_64.AppImage
./Windows-Remote-Support-Linux-x86_64.AppImage
```

## Target setup

### Windows target

Enable Windows Remote Desktop on an edition that supports incoming RDP connections, then use **RDP desktop**.

### macOS target

In **System Settings > General > Sharing**:

- enable **Remote Login** for SSH and SFTP;
- enable **Screen Sharing** for VNC-compatible desktop access.

### Ubuntu/Linux target

Depending on the desktop and services installed:

- enable OpenSSH Server for **SSH terminal** and **SFTP files**;
- enable GNOME Remote Desktop or another authorised RDP server for **RDP desktop**;
- enable a VNC server for **VNC desktop**.

The controller delegates authentication, encryption and remote-session authorisation to the selected RDP, SSH/SFTP or VNC implementation. It does not install a custom background listener or hidden control service.

## Client requirements

The controller opens established remote-access clients instead of embedding protocol engines.

- Windows uses built-in Remote Desktop and OpenSSH where available; VNC requires a registered VNC viewer.
- macOS uses registered `rdp://`, `ssh://`, `sftp://` and `vnc://` handlers; Screen Sharing provides the built-in VNC handler.
- Linux uses `xdg-open` for registered protocol handlers and the local terminal/OpenSSH for SSH where available.

If a protocol has no compatible local client or URL handler, the controller reports that the hand-off could not be opened.

## Audit log

Security-relevant events include consent decisions, session termination and protocol hand-offs.

Windows:

```text
%LOCALAPPDATA%\WindowsRemoteSupport\audit.log
```

macOS:

```text
~/Library/Logs/WindowsRemoteSupport/audit.log
```

Linux:

```text
${XDG_STATE_HOME:-~/.local/state}/windows-remote-support/audit.log
```

## Build from source

### Windows

Requirements: Visual Studio 2026 with Desktop development with C++, a current Windows SDK and CMake 3.30 or newer.

```powershell
cmake -S modern -B build-modern-x64 -A x64
cmake --build build-modern-x64 --config Release --parallel
.\build-modern-x64\Release\RemoteSupport.exe --self-test
```

For x86, use `-A Win32` and the corresponding build directory.

### macOS / Linux controller source

The portable controller source is `portable/remote_support.py`. Its non-interactive check is:

```bash
python3 portable/remote_support.py --self-test
```

Release packaging uses PyInstaller; the macOS package is wrapped as a DMG and the Linux package as an AppImage.

## Security baseline

The redesigned application uses visible consent, random 128-bit session identifiers, UTC audit records, fail-closed audit checks for connection hand-offs, target validation and no stored remote credentials. The Windows native build additionally uses `/W4`, SDL checks, `/GS`, Control Flow Guard, ASLR, DEP/NX and x64 CET-compatible linking.

See [`docs/SECURE_REDESIGN.md`](docs/SECURE_REDESIGN.md) and [`SECURITY.md`](SECURITY.md).

## Repository layout

```text
Windows-Remote-Support/
├── modern/                     # native Windows controller
│   ├── CMakeLists.txt
│   └── RemoteSupport/main.cpp
├── portable/                   # macOS/Linux controller
│   └── remote_support.py
├── .github/workflows/
│   ├── codeql.yml
│   ├── modern-windows.yml
│   └── release.yml
├── client/                     # historical reference source
├── server/                     # historical reference source
├── docs/SECURE_REDESIGN.md
├── SECURITY.md
├── MODERNIZATION.md
├── VERSION
└── README.md
```
