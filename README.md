# Windows Remote Support

A native **x86/x64 Windows remote-support application** built around explicit consent, auditing and hand-offs to established remote-access protocols.

The controller application lives under [`modern/`](modern/). The historical Windows/MFC implementation remains under `client/` and `server/` for academic comparison and migration analysis; the new application does **not** link to or activate that historical command protocol.

> Use the software only on systems you own or are explicitly authorised to administer or test.

## Current release

Version **0.3.0** adds cross-platform target support for **Windows, macOS and Ubuntu/Linux** through existing authenticated remote-access clients.

Release builds are produced for:

- **x86 / Win32** — native 32-bit Windows controller;
- **x64** — native 64-bit Windows controller.

## Desktop interface

Enter the remote computer hostname or IP address, start an explicitly consented support session, then choose one of these hand-offs:

- **RDP desktop** — Windows and RDP-enabled Ubuntu desktops;
- **SSH terminal** — macOS, Ubuntu/Linux and other SSH servers through the Windows OpenSSH client;
- **SFTP files** — authenticated file transfer to macOS, Ubuntu/Linux and other SFTP servers through the Windows OpenSSH client;
- **VNC desktop** — macOS Screen Sharing and VNC-enabled Linux desktops through a VNC viewer registered on the Windows controller.

The connection buttons stay disabled until a support session has been explicitly approved. The application validates the target and records the hand-off before launching the selected client.

## Target setup

### Windows target

Enable Windows Remote Desktop on an edition that supports incoming RDP connections, then use **RDP desktop**.

### macOS target

Use the macOS Sharing settings for the capability you need:

- enable **Remote Login** for SSH and SFTP;
- enable **Screen Sharing** for a VNC-compatible desktop connection.

For VNC desktop access from Windows, install a trusted VNC viewer that registers the `vnc://` URL scheme.

### Ubuntu/Linux target

Depending on the desktop and services installed:

- enable OpenSSH Server for **SSH terminal** and **SFTP files**;
- enable GNOME Remote Desktop or another authorised RDP server for **RDP desktop**;
- enable a VNC server for **VNC desktop**.

Authentication, encryption and authorisation remain the responsibility of the selected RDP, SSH/SFTP or VNC implementation. This application does not install a custom background listener or hidden control service.

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

## Release artefacts

The release pipeline publishes:

```text
Windows-Remote-Support-x86.exe
Windows-Remote-Support-x64.exe
Windows-Remote-Support-x86.zip
Windows-Remote-Support-x64.zip
```

The `.exe` files are encapsulated single-file builds with the MSVC runtime linked statically.

The preview is not code-signed; production distribution should add trusted Windows code signing and signed update metadata.

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

## Historical implementation

The legacy `client/` and `server/` trees remain available for academic comparison, migration analysis and defensive review. They are separate from the new application and are not the security architecture of the redesign.
