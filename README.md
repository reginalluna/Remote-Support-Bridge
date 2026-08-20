# Windows Remote Support

A native **x86/x64 Windows remote-support application** built around explicit consent, auditing and Windows' authenticated Remote Desktop subsystem.

The current application lives under [`modern/`](modern/). The historical Windows/MFC implementation remains under `client/` and `server/` for academic comparison and migration analysis; the new application does **not** link to or activate that historical command protocol.

> Use the software only on systems you own or are explicitly authorised to administer or test.

## Current release

Version **0.2.0** adds a functional Windows Remote Desktop hand-off to the consent-first desktop application.

Release builds are produced for:

- **x86 / Win32** — native 32-bit Windows executable;
- **x64** — native 64-bit Windows executable.

Both architectures are built from the same C++20 source and are continuously compiled and self-tested in GitHub Actions.

## Desktop interface

When `RemoteSupport.exe` starts, the main window shows:

- current support-session status;
- the architecture of the running build (`x86` or `x64`);
- a **Remote computer name or IP address** field;
- **Start consented session** — generates a cryptographically random session identifier and asks the local user for explicit approval;
- **Connect with Windows RDP** — opens the built-in Windows Remote Desktop client for the entered computer after consent and audit checks succeed;
- **End session** — terminates the application-side support session and records the event;
- **Open audit log** — opens the local security-event log;
- **About** — shows architecture and security information.

The RDP button stays disabled until a support session has been explicitly approved. The application validates the target value and records the Remote Desktop hand-off before launching `mstsc.exe`.

## Functional remote support

The application now delegates the actual remote connection to **Windows Remote Desktop (RDP)** rather than implementing its own screen-capture, input-injection or background network service.

When the target Windows computer is configured to accept Remote Desktop connections and is reachable under the target system's normal Windows network and security policy, Windows RDP provides:

- the remote desktop display;
- keyboard and mouse interaction;
- Windows authentication and authorisation;
- Windows-managed session encryption;
- optional resource redirection according to the user's RDP settings and target policy.

This means the application is functional for real two-computer remote sessions while retaining Windows' existing security boundary instead of adding a custom privileged remote-control protocol.

## Security baseline

The redesigned application currently provides:

- native **x86 (Win32)** and **x64** output;
- Unicode Windows APIs;
- C++20;
- visible user consent before the RDP hand-off is enabled;
- 128-bit random session identifiers from Windows CNG (`BCryptGenRandom`);
- explicit application-side session termination;
- local UTC audit records;
- fail-closed behaviour when a security-relevant audit event cannot be written;
- strict validation of the Remote Desktop target field;
- no requested administrator elevation;
- no custom network listener or hidden background control service;
- `/W4`, SDL checks, `/GS`, Control Flow Guard, ASLR and DEP/NX on both architectures;
- CET-compatible linking on x64;
- automated Debug/Release builds and self-tests for x86 and x64;
- build-independent CodeQL security analysis of the modern C++ source.

See [`docs/SECURE_REDESIGN.md`](docs/SECURE_REDESIGN.md) and [`SECURITY.md`](SECURITY.md) for the wider security requirements.

## Audit log

Security-relevant session events are written to:

```text
%LOCALAPPDATA%\WindowsRemoteSupport\audit.log
```

Records include events such as `consent_granted`, `consent_denied`, `rdp_launch_requested`, `rdp_client_started`, `rdp_launch_failed` and `session_ended`.

## Build from source

### Requirements

Use a supported Windows development machine with:

- Visual Studio 2026 with **Desktop development with C++**;
- a current Windows SDK;
- CMake 3.30 or newer;
- Git.

Clone the repository:

```powershell
git clone https://github.com/reginalluna/Windows-Remote-Support.git
cd Windows-Remote-Support
```

### Build x64

```powershell
cmake -S modern -B build-modern-x64 -A x64
cmake --build build-modern-x64 --config Release --parallel
.\build-modern-x64\Release\RemoteSupport.exe
```

### Build x86 / Win32

```powershell
cmake -S modern -B build-modern-x86 -A Win32
cmake --build build-modern-x86 --config Release --parallel
.\build-modern-x86\Release\RemoteSupport.exe
```

The x86 build runs natively on 32-bit Windows and through WoW64 on compatible x64 Windows installations. The x64 build is the preferred target for current 64-bit Windows PCs.

## Automated self-test

Either build can run the non-interactive baseline test:

```powershell
.\build-modern-x64\Release\RemoteSupport.exe --self-test
.\build-modern-x86\Release\RemoteSupport.exe --self-test
```

A successful self-test exits with code `0`.

## Release artefacts

The release pipeline builds both Release targets and publishes standalone executables as well as ZIP archives:

```text
Windows-Remote-Support-x86.exe
Windows-Remote-Support-x64.exe
Windows-Remote-Support-x86.zip
Windows-Remote-Support-x64.zip
```

The `.exe` files are encapsulated single-file builds. The MSVC runtime is linked statically, so no separate Visual C++ redistributable files are packaged with them; they depend only on standard Windows system components used by the application.

The preview is not code-signed; production distribution should add a trusted Windows code-signing certificate and signed update metadata.

## Repository layout

```text
Windows-Remote-Support/
├── modern/
│   ├── CMakeLists.txt
│   └── RemoteSupport/
│       └── main.cpp            # consent UI + audited Windows RDP hand-off
├── .github/workflows/
│   ├── codeql.yml
│   ├── modern-windows.yml      # x86/x64 Debug/Release builds + self-tests
│   └── release.yml             # standalone EXEs + ZIP release packaging
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

## Contributing

Keep security-sensitive changes small and reviewable. Do not weaken consent, auditing or fail-closed behaviour to preserve compatibility with the historical protocol.

Do not commit local build products, IDE caches, credentials, signing keys or generated secrets.
