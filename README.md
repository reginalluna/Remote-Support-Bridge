# Remote Support Bridge

Remote Support Bridge is a consent-first, cross-platform remote-support controller for **Windows, macOS and Ubuntu/Linux**. It provides local session approval, audit logging and hand-offs to established RDP, SSH/SFTP and VNC clients rather than embedding a custom privileged remote-control protocol.

The current application is implemented in [`modern/`](modern/) for native Windows builds and [`portable/`](portable/) for packaged macOS/Linux builds. Historical Windows/MFC material under `client/` and `server/` is retained only for comparison and migration analysis and is not linked into the current controller.

> Use the software only on systems you own or are explicitly authorised to administer or test.

## Release

**v0.1.0** is the first public release.

The existing v0.1.0 asset names retain the original `Windows-Remote-Support` prefix for release continuity:

```text
Windows-Remote-Support-x86.exe
Windows-Remote-Support-x64.exe
Windows-Remote-Support-x86.zip
Windows-Remote-Support-x64.zip
Windows-Remote-Support-macOS-AppleSilicon.dmg
Windows-Remote-Support-macOS-Intel.dmg
Windows-Remote-Support-Linux-x86_64.AppImage
```

- Windows: native x86 and x64 C++20 controllers.
- macOS Apple Silicon: ARM64 `Windows Remote Support.app` inside the Apple Silicon DMG.
- macOS Intel: x86_64 `Windows Remote Support.app` inside the Intel DMG.
- Ubuntu/Linux: x86_64 AppImage.

The macOS and Linux packages include their Python/Tk runtime, so Python is not required on the destination computer.

## Supported hand-offs

Enter a remote hostname or IP address, start an explicitly consented session, then choose a connection type:

- **RDP desktop** — Windows and RDP-enabled Linux desktops;
- **SSH terminal** — macOS, Ubuntu/Linux and other SSH servers;
- **SFTP files** — authenticated file transfer to macOS, Ubuntu/Linux and other SFTP servers;
- **VNC desktop** — macOS Screen Sharing and VNC-enabled Linux desktops.

The controller validates the target and records the hand-off before opening the selected client. It does not store remote passwords.

## Quick start

1. Download the package for the computer running the controller:
   - Windows 64-bit: `Windows-Remote-Support-x64.exe`
   - Windows 32-bit: `Windows-Remote-Support-x86.exe`
   - Apple Silicon Mac: `Windows-Remote-Support-macOS-AppleSilicon.dmg`
   - Intel Mac: `Windows-Remote-Support-macOS-Intel.dmg`
   - Ubuntu/Linux x86_64: `Windows-Remote-Support-Linux-x86_64.AppImage`
2. Run the controller.
3. Enter the remote computer hostname or IP address.
4. Select **Start consented session** and approve the local session.
5. Select **RDP desktop**, **SSH terminal**, **SFTP files** or **VNC desktop**.
6. Authenticate in the remote-access client opened by the operating system.
7. Select **End session** when finished.

### macOS

Open the DMG matching the Mac architecture and launch **Windows Remote Support.app**. The first release is not Developer ID signed, so macOS may display a security warning.

### Ubuntu/Linux

If required, mark the AppImage as executable before launching it:

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

Authentication, encryption and remote-session authorisation remain the responsibility of the selected RDP, SSH/SFTP or VNC implementation.

## Local client requirements

- Windows uses built-in Remote Desktop and OpenSSH where available; VNC requires a registered VNC viewer.
- macOS uses registered `rdp://`, `ssh://`, `sftp://` and `vnc://` handlers; Screen Sharing provides the built-in VNC handler.
- Linux uses `xdg-open` for registered protocol handlers and the local terminal/OpenSSH where available.

If no compatible local client or URL handler exists, Remote Support Bridge reports that the hand-off could not be opened.

## Audit logs

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

Clone the renamed repository:

```bash
git clone https://github.com/reginalluna/Remote-Support-Bridge.git
cd Remote-Support-Bridge
```

### Windows

Requirements: Visual Studio 2026 with Desktop development with C++, a current Windows SDK and CMake 3.30 or newer.

```powershell
cmake -S modern -B build-modern-x64 -A x64
cmake --build build-modern-x64 --config Release --parallel
.\build-modern-x64\Release\RemoteSupport.exe --self-test
```

For x86, use `-A Win32` and the corresponding build directory.

### macOS / Linux

The portable controller source is `portable/remote_support.py`:

```bash
python3 portable/remote_support.py --self-test
```

Release packaging uses PyInstaller; macOS packages are wrapped as DMGs and Linux is packaged as an AppImage.

## Security baseline

Remote Support Bridge uses visible local consent, random 128-bit session identifiers, UTC audit records, fail-closed audit checks for hand-offs, target validation and no stored remote credentials. The native Windows build additionally uses `/W4`, SDL checks, `/GS`, Control Flow Guard, ASLR, DEP/NX and x64 CET-compatible linking.

See [`docs/SECURE_REDESIGN.md`](docs/SECURE_REDESIGN.md) and [`SECURITY.md`](SECURITY.md).

## Repository layout

```text
Remote-Support-Bridge/
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
