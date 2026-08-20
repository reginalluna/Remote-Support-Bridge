# Remote

A Windows remote-support research project being redesigned around a modern **native x86/x64**, consent-first security model.

The repository still contains the historical Windows/MFC implementation under `client/` and `server/` for reference and migration work. The new implementation lives under [`modern/`](modern/) and does **not** link to or activate the historical remote-command protocol.

> Use the software only on systems you own or are explicitly authorised to administer or test.

## Native x86 and x64 redesign

The new `RemoteSupport` application is a clean Windows baseline for both 32-bit and 64-bit PCs. It uses:

- native **x86 (Win32)** and **x64** output from the same source tree;
- Unicode Windows APIs;
- C++20;
- explicit visible consent before a session can initialise;
- cryptographically secure 128-bit session identifiers from Windows CNG (`BCryptGenRandom`);
- local audit records under `%LOCALAPPDATA%\RemoteSupport\audit.log`;
- no requested administrator elevation;
- `/W4`, SDL checks, `/GS`, Control Flow Guard, ASLR and DEP/NX on both architectures;
- CET-compatible linking on x64;
- automated Debug/Release builds and self-tests for both x86 and x64 in GitHub Actions.

The modern binary deliberately starts with **no privileged remote-control actions and no network listener**. Authentication, encrypted transport and individual support capabilities are added only behind the new security boundary rather than inherited from the legacy protocol.

## Build the modern application

### Requirements

Use a supported Windows development machine with:

- Visual Studio 2026 with **Desktop development with C++**;
- a current Windows SDK;
- CMake 3.30 or newer;
- Git.

Clone the repository:

```powershell
git clone https://github.com/reginalluna/Remote.git
cd Remote
```

### Build x64

Configure:

```powershell
cmake -S modern -B build-modern-x64 -A x64
```

Build Release:

```powershell
cmake --build build-modern-x64 --config Release --parallel
```

Run:

```powershell
.\build-modern-x64\Release\RemoteSupport.exe
```

### Build x86 / Win32

Configure:

```powershell
cmake -S modern -B build-modern-x86 -A Win32
```

Build Release:

```powershell
cmake --build build-modern-x86 --config Release --parallel
```

Run:

```powershell
.\build-modern-x86\Release\RemoteSupport.exe
```

The x86 binary is suitable for 32-bit Windows environments and also runs on x64 Windows through WoW64. The x64 binary is the preferred target for current 64-bit Windows PCs.

The application displays a visible consent prompt, creates a random session identifier, and records the consent decision locally.

Run the non-interactive security-baseline self-test with either build:

```powershell
.\build-modern-x64\Release\RemoteSupport.exe --self-test
.\build-modern-x86\Release\RemoteSupport.exe --self-test
```

A successful self-test exits with code `0`.

## Audit log

The redesign records session-consent events at:

```text
%LOCALAPPDATA%\RemoteSupport\audit.log
```

Each record contains a UTC timestamp, random session identifier and event name. If the audit record cannot be written after consent is granted, the session does not continue.

## Security architecture

The redesign follows these rules:

1. **Identity before capability** — operators and devices must be authenticated before any support action is offered.
2. **Encrypted transport only** — future network transport must use a maintained authenticated TLS implementation with certificate validation and replay-resistant sessions.
3. **Consent by default** — sensitive capabilities require an explicit, visible local approval path.
4. **Least privilege** — the application runs as the signed-in user by default and does not request elevation merely to start.
5. **Capability allow-listing** — future support functions are individually authorised rather than exposing a general command channel.
6. **Session expiry** — authentication and consent are scoped to a bounded session and are not permanent trust grants.
7. **Auditability** — security-relevant session events must be recorded and failures must fail closed.
8. **Signed distribution** — production releases should use code signing and signed update metadata before deployment.

See [`docs/SECURE_REDESIGN.md`](docs/SECURE_REDESIGN.md) and [`SECURITY.md`](SECURITY.md) for the wider security requirements.

## Next implementation milestones

The safe migration order for the new cross-architecture application is:

1. add authenticated TLS transport and device/operator identity;
2. add session expiry, replay protection and key rotation;
3. make the audit trail tamper-evident;
4. add one narrowly scoped support capability at a time behind authentication, authorisation and explicit consent;
5. add signed release/update verification;
6. expand automated tests, CodeQL and dependency scanning around the new application.

The historical remote shell, screen-control, file-management, registry/service, audio/video and related command paths are **not** automatically carried into the new application.

## Repository layout

```text
Remote/
├── modern/
│   ├── CMakeLists.txt
│   └── RemoteSupport/
│       └── main.cpp            # new consent-first application
├── .github/workflows/
│   ├── codeql.yml
│   └── modern-windows.yml      # x86/x64 Debug/Release builds + self-tests
├── client/                     # historical client source
├── server/                     # historical MFC controller source
├── docs/SECURE_REDESIGN.md
├── SECURITY.md
├── MODERNIZATION.md
└── README.md
```

## Historical implementation

The legacy `client/` and `server/` trees remain available for academic comparison, migration analysis and defensive review. They are separate from the new application and should not be treated as the security architecture for the redesign.

## Contributing

Keep security-sensitive changes small and reviewable. Do not weaken authentication, consent, audit or fail-closed behaviour to preserve compatibility with the historical protocol.

Do not commit local build products, IDE caches, credentials, signing keys or generated secrets.
