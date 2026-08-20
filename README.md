# Windows Remote Support

A native **x86/x64 Windows remote-support controller** built around explicit consent, auditing and hand-offs to established remote-access protocols.

The controller application lives under [`modern/`](modern/). The historical Windows/MFC material under `client/` and `server/` is kept separate for academic comparison and migration analysis; the current application does **not** link to or activate that historical command protocol.

> Use the software only on systems you own or are explicitly authorised to administer or test.

## Release

**v0.1.0** is the first public release of the redesigned application. It supports Windows controllers and can connect to authorised **Windows, macOS and Ubuntu/Linux** targets through existing authenticated remote-access clients.

Release builds are provided for:

- **x86 / Win32** — native 32-bit Windows controller;
- **x64** — native 64-bit Windows controller.

## What it can open

Enter the remote computer hostname or IP address, start an explicitly consented support session, then choose a connection type:

- **RDP desktop** — Windows and RDP-enabled Ubuntu/Linux desktops;
- **SSH terminal** — macOS, Ubuntu/Linux and other SSH servers through Windows OpenSSH;
- **SFTP files** — authenticated file transfer to macOS, Ubuntu/Linux and other SFTP servers through Windows OpenSSH;
- **VNC desktop** — macOS Screen Sharing and VNC-enabled Linux desktops through a VNC viewer registered on the Windows controller.

The connection controls remain disabled until the local support session has been explicitly approved. The application validates the target and records the hand-off before opening the selected client.

## Quick start

1. Download `Windows-Remote-Support-x64.exe` for a normal 64-bit Windows PC, or `Windows-Remote-Support-x86.exe` for a 32-bit Windows system.
2. Run the executable.
3. Enter the remote computer hostname or IP address.
4. Select **Start consented session** and approve the session locally.
5. Select **RDP desktop**, **SSH terminal**, **SFTP files** or **VNC desktop**.
6. Authenticate in the client opened by Windows.
7. Select **End session** when finished.

The application does not store remote passwords.

## Target setup

### Windows

Enable Windows Remote Desktop on an edition that supports incoming RDP connections, then use **RDP desktop**.

### macOS

Use macOS **System Settings > General > Sharing**:

- enable **Remote Login** for SSH and SFTP;
- enable **Screen Sharing** for VNC-compatible desktop access.

For VNC desktop access from the Windows controller, install a trusted VNC viewer that registers the `vnc://` URL scheme.

### Ubuntu/Linux

Depending on the desktop and services installed:

- enable OpenSSH Server for **SSH terminal** and **SFTP files**;
- enable GNOME Remote Desktop or another authorised RDP server for **RDP desktop**;
- enable a VNC server for **VNC desktop**.

Authentication, encryption and authorisation remain the responsibility of the selected RDP, SSH/SFTP or VNC implementation. The controller does not install a custom background listener or hidden control service.

## Audit log

Security-relevant events are written to:

```text
%LOCALAPPDATA%\WindowsRemoteSupport\audit.log
```

Records include consent decisions, session termination and protocol hand-offs such as `rdp_launch_requested`, `ssh_client_started`, `sftp_client_started` and `vnc_client_started`.

## Build from source

Requirements:

- Visual Studio 2026 with **Desktop development with C++**;
- a current Windows SDK;
- CMake 3.30 or newer;
- Git.

Clone the repository:

```powershell
git clone https://github.com/reginalluna/Windows-Remote-Support.git
cd Windows-Remote-Support
```

Build x64:

```powershell
cmake -S modern -B build-modern-x64 -A x64
cmake --build build-modern-x64 --config Release --parallel
.\build-modern-x64\Release\RemoteSupport.exe
```

Build x86:

```powershell
cmake -S modern -B build-modern-x86 -A Win32
cmake --build build-modern-x86 --config Release --parallel
.\build-modern-x86\Release\RemoteSupport.exe
```

Run the self-test:

```powershell
.\build-modern-x64\Release\RemoteSupport.exe --self-test
.\build-modern-x86\Release\RemoteSupport.exe --self-test
```

A successful self-test exits with code `0`.

## Release files

The release pipeline publishes:

```text
Windows-Remote-Support-x86.exe
Windows-Remote-Support-x64.exe
Windows-Remote-Support-x86.zip
Windows-Remote-Support-x64.zip
```

The `.exe` files are encapsulated single-file builds with the MSVC runtime linked statically.

The first release is not code-signed; production distribution should add trusted Windows code signing and signed update metadata.

## Security baseline

The redesigned application uses visible consent, random session identifiers, local UTC audit records, fail-closed audit checks, target validation, no requested administrator elevation, no custom background listener, `/W4`, SDL checks, `/GS`, Control Flow Guard, ASLR, DEP/NX and x64 CET-compatible linking.

See [`docs/SECURE_REDESIGN.md`](docs/SECURE_REDESIGN.md) and [`SECURITY.md`](SECURITY.md) for the wider security requirements.

## Repository layout

```text
Windows-Remote-Support/
├── modern/
│   ├── CMakeLists.txt
│   └── RemoteSupport/
│       └── main.cpp            # consent UI + audited RDP/SSH/SFTP/VNC hand-offs
├── .github/workflows/
│   ├── codeql.yml
│   ├── modern-windows.yml
│   └── release.yml
├── client/                     # historical client source
├── server/                     # historical MFC controller source
├── docs/SECURE_REDESIGN.md
├── SECURITY.md
├── MODERNIZATION.md
├── VERSION
└── README.md
```
