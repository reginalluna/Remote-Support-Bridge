# Remote

Legacy Windows/MFC remote-administration codebase preserved for maintenance, documentation and modernisation.

> **Important:** this repository contains legacy remote screen, shell, file-management, registry/service-management, audio/video and networking code. It predates current security expectations. Use it only on systems you own or are explicitly authorised to test, and do not expose the legacy client/server directly to the public Internet.

## Repository status

The repository has received a first modernisation pass for repository hygiene and Visual Studio 2022 solution metadata. The application code itself remains largely legacy code.

Current known limitations include:

- Win32-only project configuration;
- Multi-Byte rather than Unicode text handling;
- 32-bit pointer/integer assumptions in parts of the source;
- bundled zlib 1.1.4-era files and libraries;
- legacy Visual C++ project files in the client tree;
- no verified modern authentication, transport-encryption or consent model for the remote-administration features.

See [`MODERNIZATION.md`](MODERNIZATION.md) for the detailed compatibility and security assessment.

## Project layout

```text
Remote/
├── client/
│   ├── ClientDll/      # legacy client-side implementation
│   └── ClientExe/      # legacy client executable project/resources
├── server/
│   ├── 2015Remote.sln  # server solution, updated to VS 2022 metadata
│   └── 2015Remote/     # MFC server application
├── MODERNIZATION.md
└── README.md
```

## Requirements

For the server project, use a current Windows development machine with:

- Windows 10 or Windows 11;
- Visual Studio 2022;
- **Desktop development with C++** workload;
- MFC/ATL support for the installed MSVC toolset;
- a Windows SDK installed by Visual Studio.

The existing project is configured for **Win32**. Do not switch it to x64 until the pointer/integer assumptions documented in `MODERNIZATION.md` have been audited.

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

If Visual Studio offers to retarget the project to an installed Windows SDK/toolset, review the proposed changes before accepting them. The solution metadata is modernised, but the underlying source and project settings are still legacy.

## Building for maintenance/testing

1. Select **Win32** as the platform.
2. Start with the **Debug** configuration.
3. Build the solution with **Build → Build Solution**.
4. Treat compiler/linker warnings as migration findings rather than suppressing them globally.

A successful build on every current Visual Studio installation is **not yet guaranteed**. In particular, the repository still contains a very old zlib dependency and legacy project assumptions that should be replaced or audited before this is considered a current supported build.

## Client source

The client tree contains Visual C++ 6-era project material and pre-existing legacy source. Those project files are retained for reproducibility and source review.

There is currently no supported modern client build procedure. The recommended approach is to audit and migrate the client project separately rather than attempting to load the old project files into a current compiler and accepting automatic conversions wholesale.

## How to use this repository today

The supported use of the current repository is **source maintenance, code review, compatibility work and controlled testing**:

1. clone the repository;
2. open the server solution in Visual Studio 2022;
3. build/debug the Win32 project in an isolated test environment;
4. review compiler diagnostics and legacy dependencies;
5. make migration changes in small pull requests;
6. test only between machines or virtual machines you control and have explicitly authorised.

The repository does **not** currently provide a supported production deployment procedure for its remote-administration functionality. Before such use, the software would need a separate security redesign covering authentication, authorisation, encrypted transport, explicit user consent, session logging, least privilege and update handling.

## Recommended modernisation order

1. Replace the obsolete zlib dependency without changing protocol behaviour.
2. Resolve compiler warnings and unsafe legacy type assumptions.
3. Audit pointer-sized arithmetic before adding x64.
4. Migrate text handling to Unicode in a separate change.
5. Review authentication, authorisation and transport security before changing remote-control behaviour.
6. Add repeatable CI builds only after the dependency/toolchain baseline is stable.

## Contributing

Keep changes small and reviewable. Prefer one migration concern per pull request, for example dependency cleanup, warning fixes, x64 readiness or Unicode migration.

Do not commit Visual Studio caches, local build products or user-specific project settings; the repository `.gitignore` covers the common cases.

## Disclaimer

This is legacy software with security-sensitive capabilities. Only use it where you have explicit permission. The current repository should be treated as a maintenance and modernisation project, not as a security-reviewed remote-support product.
