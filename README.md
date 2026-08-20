# Remote

A Windows remote-support research project being redesigned around a modern **native x64**, consent-first security model.

The repository still contains the historical Windows/MFC implementation under `client/` and `server/` for reference and migration work. The new implementation lives under [`modern/`](modern/) and does **not** link to or activate the historical remote-command protocol.

> Use the software only on systems you own or are explicitly authorised to administer or test.

## Native x64 redesign

The new `RemoteSupport` application is a clean x64 baseline for current Windows PCs. It uses:

- native **x64** output;
- Unicode Windows APIs;
- C++20;
- explicit visible consent before a session can initialise;
- cryptographically secure 128-bit session identifiers from Windows CNG (`BCryptGenRandom`);
- local audit records under `%LOCALAPPDATA%\RemoteSupport\audit.log`;
- no requested administrator elevation;
- `/W4`, SDL checks, `/GS`, Control Flow Guard, ASLR, DEP/NX and CET-compatible linking;
- an automated Debug/Release x64 build and self-test in GitHub Actions.

The modern binary deliberately starts with **no privileged remote-control actions and no network listener**. Authentication, encrypted transport and individual support capabilities are added only behind the new security boundary rather than inherited from the legacy protocol.

## Build the modern x64 application

### Requirements

Use a 64-bit Windows 11 or supported Windows 10 development machine with:

- Visual Studio 2026 with **Desktop development with C++**;
- a current Windows SDK;
- CMake 3.30 or newer;
- Git.

Clone the repository:

```powershell
git clone https://github.com/reginalluna/Remote.git
cd Remote
```

Configure a native x64 build:

```powershell
cmake -S modern -B build-modern -A x64
```

Build Release:

```powershell
cmake --build build-modern --config Release --parallel
```

Run:

```powershell
.\build-modern\Release\RemoteSupport.exe
```

The application displays a visible consent prompt, creates a random session identifier, and records the consent decision locally.

Run the non-interactive security-baseline self-test:

```powershell
.\build-modern\Release\RemoteSupport.exe --self-test
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

The safe migration order for the new x64 application is:

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
│       └── main.cpp            # new native x64 consent-first application
├── .github/workflows/
│   ├── codeql.yml
│   └── modern-x64.yml          # Debug/Release x64 build + self-test
├── client/                     # historical client source
├── server/                     # historical MFC controller source
├── docs/SECURE_REDESIGN.md
├── SECURITY.md
├── MODERNIZATION.md
└── README.md
```

## Historical implementation

The legacy `client/` and `server/` trees remain available for academic comparison, migration analysis and defensive review. They are separate from the new x64 application and should not be treated as the security architecture for the redesign.

## Contributing

Keep security-sensitive changes small and reviewable. Do not weaken authentication, consent, audit or fail-closed behaviour to preserve compatibility with the historical protocol.

Do not commit local build products, IDE caches, credentials, signing keys or generated secrets.
