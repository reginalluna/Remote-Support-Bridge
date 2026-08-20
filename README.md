# Windows Remote Support

A Windows remote-support research project being redesigned around a modern **native x86/x64**, consent-first security model.

The current application under [`modern/`](modern/) is a Windows desktop programme with an interactive UI for starting and ending explicitly consented local support sessions, viewing session state and opening the local audit log. The historical Windows/MFC implementation remains under `client/` and `server/` for academic comparison and migration analysis; the new application does **not** link to or activate that historical command protocol.

> Use the software only on systems you own or are explicitly authorised to administer or test.

## Current release

Version **0.1.0** is the first desktop-UI preview of the redesigned application. Release builds are produced for:

- **x86 / Win32** — native 32-bit Windows executable;
- **x64** — native 64-bit Windows executable.

Both architectures are built from the same C++20 source and are continuously compiled and self-tested in GitHub Actions.

## Desktop interface

When `RemoteSupport.exe` starts, the main window shows:

- current session status;
- the architecture of the running build (`x86` or `x64`);
- **Start consented session** — generates a cryptographically random session identifier and asks the local user for explicit approval;
- **End session** — terminates the active local session and records the event;
- **Open audit log** — opens the local security-event log;
- **About** — shows the architecture and current security-baseline information.

A session is not activated unless the local user explicitly approves it. If an approved session cannot be recorded in the audit log, the application fails closed and does not continue the session.

The current preview intentionally has **no network listener, no administrator-elevation request, no hidden background session and no privileged remote-control actions**. Those capabilities are not inherited from the historical protocol.

## Security baseline

The redesigned application currently provides:

- native **x86 (Win32)** and **x64** output;
- Unicode Windows APIs;
- C++20;
- visible user consent before session activation;
- 128-bit random session identifiers from Windows CNG (`BCryptGenRandom`);
- explicit session termination;
- local UTC audit records;
- no requested administrator elevation;
- `/W4`, SDL checks, `/GS`, Control Flow Guard, ASLR and DEP/NX on both architectures;
- CET-compatible linking on x64;
- automated Debug/Release builds and self-tests for x86 and x64;
- CodeQL security analysis of both modern targets.

See [`docs/SECURE_REDESIGN.md`](docs/SECURE_REDESIGN.md) and [`SECURITY.md`](SECURITY.md) for the wider security requirements.

## Audit log

Security-relevant session events are written to:

```text
%LOCALAPPDATA%\WindowsRemoteSupport\audit.log
```

Each record contains a UTC timestamp, a random session identifier and an event name such as `consent_granted`, `consent_denied` or `session_ended`.

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

The release pipeline builds both Release targets and publishes:

```text
Windows-Remote-Support-x86.zip
Windows-Remote-Support-x64.zip
SHA256SUMS.txt
```

The checksum file can be used to verify that a downloaded archive matches the artefact produced by the release workflow. The initial preview is not code-signed; production distribution should add a trusted Windows code-signing certificate and signed update metadata.

## Security architecture

The redesign follows these rules:

1. **Identity before capability** — operator and device identity must be established before support capabilities are enabled.
2. **Encrypted transport only** — future network transport must use a maintained authenticated TLS implementation with certificate validation and replay-resistant sessions.
3. **Consent by default** — sensitive capabilities require an explicit and visible local approval path.
4. **Least privilege** — the programme runs as the signed-in user by default and does not request elevation merely to start.
5. **Capability allow-listing** — support functions are individually authorised instead of exposing a general command channel.
6. **Session expiry** — authentication and consent are scoped to bounded sessions rather than permanent trust grants.
7. **Auditability** — security-relevant session events must be recorded and security failures fail closed.
8. **Signed distribution** — production releases should use code signing and signed update metadata.

## Next implementation milestones

The next safe milestones for the new application are authenticated encrypted transport, device/operator identity, session expiry and replay protection, tamper-evident auditing, and then individually reviewed support capabilities behind authentication, authorisation and explicit consent.

The historical remote shell, screen-control, file-management, registry/service, audio/video and related command paths are **not** automatically carried into the redesigned application.

## Repository layout

```text
Windows-Remote-Support/
├── modern/
│   ├── CMakeLists.txt
│   └── RemoteSupport/
│       └── main.cpp            # consent-first Windows desktop application
├── .github/workflows/
│   ├── codeql.yml
│   ├── modern-windows.yml      # x86/x64 Debug/Release builds + self-tests
│   └── release.yml             # x86/x64 release packaging
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

Keep security-sensitive changes small and reviewable. Do not weaken authentication, consent, auditing or fail-closed behaviour to preserve compatibility with the historical protocol.

Do not commit local build products, IDE caches, credentials, signing keys or generated secrets.
