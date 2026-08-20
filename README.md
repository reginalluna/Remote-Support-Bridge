# Remote

Legacy Windows/MFC remote-administration codebase preserved for maintenance, documentation and defensive modernisation.

> **Important:** this repository contains historical remote shell, screen-control, file-management, process/window-management, registry/service-management, audio/video and networking functionality. The original protocol predates modern authentication, encryption, consent and audit requirements. Use the code only on systems you own or are explicitly authorised to test, and do not expose the legacy protocol directly to the public Internet.

## Current status

The repository has now received a broader defensive modernisation pass covering repository hygiene, Visual Studio 2022 solution metadata, compiler/linker exploit mitigations, security policy documentation and automated CodeQL analysis.

The application code itself is still largely legacy code. It is **not** a production-ready remote-support product.

Known limitations still include:

- Win32-only project configuration;
- Multi-Byte rather than Unicode text handling;
- 32-bit pointer/integer assumptions in parts of the source;
- bundled zlib 1.1.4-era files and libraries;
- Visual C++ 6-era project material in the client tree;
- no verified modern mutual authentication, encrypted transport, role-based authorisation, consent UI or tamper-evident session audit model for the legacy remote protocol.

See [`MODERNIZATION.md`](MODERNIZATION.md) for compatibility notes and [`SECURITY.md`](SECURITY.md) for the current security policy and production-security requirements.

## Defensive hardening now applied

MSBuild-based C/C++ projects inherit hardening from [`Directory.Build.targets`](Directory.Build.targets):

- `/W4` warning level;
- SDL security checks;
- `/GS` stack-buffer protection;
- Control Flow Guard for compiler and linker output;
- ASLR-compatible linking;
- DEP/NX-compatible linking;
- `asInvoker` execution to avoid requesting elevation by default;
- UIAccess disabled.

These mitigations reduce exploitability of some memory-corruption defects. They do **not** make the legacy command protocol safe for production use.

The repository also includes a scheduled and pull-request-triggered CodeQL workflow at [`.github/workflows/codeql.yml`](.github/workflows/codeql.yml).

## Project layout

```text
Remote/
├── .github/
│   └── workflows/
│       └── codeql.yml
├── client/
│   ├── ClientDll/             # legacy client-side implementation
│   └── ClientExe/             # legacy client executable project/resources
├── server/
│   ├── 2015Remote.sln         # server solution with VS 2022 metadata
│   └── 2015Remote/            # MFC server application
├── Directory.Build.targets    # defensive MSVC hardening defaults
├── MODERNIZATION.md
├── SECURITY.md
└── README.md
```

## Requirements

For the server project, use a current Windows development machine with:

- Windows 10 or Windows 11;
- Visual Studio 2022;
- the **Desktop development with C++** workload;
- MFC/ATL support for the installed MSVC toolset;
- a current Windows SDK installed through Visual Studio.

The existing project remains configured for **Win32**. Do not switch it to x64 until the pointer/integer assumptions documented in `MODERNIZATION.md` have been audited.

## Getting the source

```powershell
git clone https://github.com/reginalluna/Remote.git
cd Remote
```

## Opening the server project

Open:

```text
server/2015Remote.sln
```

in Visual Studio 2022.

If Visual Studio offers to retarget the project to an installed Windows SDK/toolset, review the proposed changes before accepting them. The solution metadata and defensive build defaults are modernised, but the source still contains legacy assumptions.

## Building for maintenance and defensive testing

1. Select **Win32** as the platform.
2. Start with the **Debug** configuration.
3. Build with **Build → Build Solution**.
4. Treat warnings as migration findings rather than suppressing them globally.
5. Run only in an isolated test environment or on explicitly authorised systems.

Command-line equivalent from a Visual Studio Developer PowerShell is:

```powershell
msbuild server\2015Remote.sln /m /p:Configuration=Debug /p:Platform=Win32
```

A clean build on every current Visual Studio installation is **not yet guaranteed**, primarily because the repository still carries an obsolete zlib dependency and historical project assumptions.

## Verifying Windows exploit mitigations

After a successful build, inspect the executable headers from a Visual Studio Developer Command Prompt:

```text
dumpbin /headers path\to\2015Remote.exe
```

Check that the resulting image reports modern mitigation characteristics such as dynamic-base/ASLR, NX compatibility and Guard/CFG information. The exact text varies by linker version.

## Client source

The client tree contains Visual C++ 6-era project material and pre-existing legacy source. Those files are retained for reproducibility, audit and historical review.

There is currently no supported modern production client build procedure. Migrating the client safely requires a separate source-level review rather than accepting an automatic toolchain conversion wholesale.

## Supported use of the repository today

The supported use is **maintenance, code review, compatibility work, defensive analysis and controlled laboratory testing**:

1. clone the repository;
2. open the server solution in Visual Studio 2022;
3. build/debug the Win32 project in an isolated environment;
4. review compiler and CodeQL findings;
5. replace obsolete dependencies and unsafe legacy assumptions in small pull requests;
6. verify security mitigations on produced binaries;
7. test only between machines or virtual machines you control and have explicitly authorised.

The repository does **not** provide a supported production deployment procedure for its legacy remote-administration protocol.

## What a modern production design would require

A current remote-support product should be redesigned around modern security properties rather than treating the historical protocol as production-ready. At minimum, that design should include:

- mutually authenticated TLS using a maintained cryptographic stack;
- strong operator and device identity;
- role-based authorisation and least privilege;
- explicit visible user consent for sensitive operations;
- session expiry and replay protection;
- protected credential/key storage;
- tamper-evident security logging and audit trails;
- signed binaries and signed update metadata;
- secure update/rollback handling;
- rate limiting and connection-abuse controls;
- dependency inventory and automated vulnerability scanning;
- reproducible CI builds and security gates;
- incident-response and key-rotation procedures.

These controls require an architectural redesign. Adding encryption around the existing unauthenticated command model alone would not be sufficient.

## Recommended modernisation order

1. Replace the obsolete zlib dependency with a maintained release and verify data-format compatibility.
2. Resolve compiler warnings and unsafe memory/type assumptions.
3. Audit pointer-sized arithmetic before introducing x64.
4. Migrate text handling to Unicode in a separate, tested change.
5. Remove or redesign legacy privileged operations rather than carrying them forward unchanged.
6. Design authentication, authorisation, consent and encrypted transport as a new security boundary.
7. Make CodeQL/build checks required only after the build baseline is consistently green.

## Security reporting

Please follow [`SECURITY.md`](SECURITY.md). Do not place credentials, live targets or weaponised proof-of-concept material in a public issue.

## Contributing

Keep changes small and reviewable. Prefer one migration concern per pull request, such as dependency cleanup, warning fixes, memory-safety fixes, x64 readiness, Unicode migration or defensive build hardening.

Do not commit Visual Studio caches, local build products or user-specific project settings; the repository `.gitignore` covers the common cases.

## Disclaimer

This is legacy software with security-sensitive capabilities. Only use it where you have explicit permission.
