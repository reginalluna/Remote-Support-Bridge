# Remote

Windows/MFC remote-support codebase being updated for a 2026 Windows development environment.

> Use this software only on systems you own or are explicitly authorised to administer. The historical command protocol contains privileged remote-management functions, so it must not be exposed directly to untrusted networks.

## 2026 development baseline

The repository now targets the current Microsoft C++ toolchain generation while retaining a compatibility fallback for CI:

- **Visual Studio 2026 18.6 or newer** is the preferred IDE;
- **MSVC v145 / 14.51 or newer** is preferred when Visual Studio 2026 is available;
- **MSVC v143** remains an automatic fallback for Visual Studio 2022-based CI environments;
- **Windows 11 SDK 10.0.28000.2526 or newer supported SDK** is recommended;
- **C++20** language mode and MSVC conformance mode are enabled repository-wide;
- the compiler host architecture is set to **x64**;
- dependencies are managed with **vcpkg manifest mode**;
- the server no longer uses the bundled 2002-era zlib headers/library and instead restores maintained `zlib` through `vcpkg`.

The server application still produces a **Win32** binary at this stage. Moving the output itself to x64 and converting the application globally to Unicode require additional source-level changes because the historical code contains 32-bit and ANSI assumptions.

## Security hardening

MSBuild C/C++ projects inherit the defensive defaults in [`Directory.Build.targets`](Directory.Build.targets):

- `/W4` warning level;
- SDL security checks;
- `/GS` stack-buffer protection;
- Control Flow Guard;
- ASLR-compatible linking;
- DEP/NX-compatible linking;
- `asInvoker` execution;
- UIAccess disabled.

The buffer implementation has also been updated to avoid 32-bit pointer truncation and to reject arithmetic overflow when expanding receive/send buffers.

Automated checks are provided by:

- [`.github/workflows/build.yml`](.github/workflows/build.yml) — Debug and Release Windows builds;
- [`.github/workflows/codeql.yml`](.github/workflows/codeql.yml) — C/C++ CodeQL analysis using the repository security query configuration.

## Project layout

```text
Remote/
├── .github/
│   ├── codeql/
│   └── workflows/
│       ├── build.yml
│       └── codeql.yml
├── client/
│   ├── ClientDll/             # historical client-side source
│   └── ClientExe/             # historical client launcher/resources
├── server/
│   ├── 2015Remote.sln         # Visual Studio 2026 solution metadata
│   └── 2015Remote/            # MFC controller application
├── Directory.Build.props      # toolchain, C++20 and vcpkg defaults
├── Directory.Build.targets    # compiler/linker hardening
├── vcpkg.json                 # maintained third-party dependencies
├── MODERNIZATION.md
├── SECURITY.md
└── README.md
```

## Prerequisites

Install on a supported 64-bit Windows system:

1. Visual Studio 2026 with **Desktop development with C++**.
2. MFC/ATL support for the installed MSVC toolset.
3. A supported Windows 11 SDK; the July 2026 `10.0.28000.2526` SDK or later is recommended.
4. Git.
5. vcpkg integration for MSBuild.

Visual Studio 2022 can still be used as a compatibility fallback if the v143 toolset is installed.

## Clone the repository

```powershell
git clone https://github.com/reginalluna/Remote.git
cd Remote
```

## Enable vcpkg for MSBuild

From a Developer PowerShell where `vcpkg` is available:

```powershell
vcpkg integrate install
```

The root [`vcpkg.json`](vcpkg.json) manifest declares `zlib`. With manifest integration enabled, Visual Studio/MSBuild restores the dependency automatically into `vcpkg_installed/` during the build.

## Open and build

Open:

```text
server/2015Remote.sln
```

Prefer Visual Studio 2026. Select **Debug | Win32** for the first build, then test **Release | Win32**.

Command-line builds from a Developer PowerShell:

```powershell
msbuild server\2015Remote.sln /m /p:Configuration=Debug /p:Platform=Win32
msbuild server\2015Remote.sln /m /p:Configuration=Release /p:Platform=Win32
```

## Verify exploit mitigations

After a successful build, inspect the PE headers:

```text
dumpbin /headers path\to\2015Remote.exe
```

Confirm the produced image reports the expected modern mitigation characteristics, including dynamic-base/ASLR, NX compatibility and Guard/CFG information.

## Current modernisation status

Completed or in place:

- Visual Studio 2026 solution metadata;
- automatic v145/v143 toolset selection;
- current supported Windows SDK selection through `10.0` MSBuild targeting;
- C++20 and conformance mode;
- x64-hosted compiler tools;
- maintained zlib dependency for the MSBuild server project through vcpkg;
- removal of the obsolete server-side vendored zlib binary/header set;
- checked buffer arithmetic and pointer-safe buffer-length calculation;
- compiler/linker exploit mitigations;
- reproducible Debug/Release CI build jobs;
- CodeQL security analysis;
- security and redesign documentation.

Still requiring source-level migration:

- native **x64 output** for the application;
- complete **Unicode** conversion;
- replacement of Visual C++ 6-era client project files with a current build project;
- removal of remaining prebuilt historical client artefacts;
- broader RAII/modern C++ conversion of raw Win32 handles, buffers and ownership;
- retirement or redesign of historical privileged command paths;
- a production security boundary for identity, authentication, authorisation, consent, encrypted transport, session expiry and audit logging.

See [`MODERNIZATION.md`](MODERNIZATION.md), [`SECURITY.md`](SECURITY.md) and [`docs/SECURE_REDESIGN.md`](docs/SECURE_REDESIGN.md) for the migration and security architecture notes.

## Development workflow

For each modernisation change:

1. create a short-lived branch;
2. keep the change focused;
3. build Debug and Release;
4. fix new compiler warnings rather than suppressing them globally;
5. run CodeQL/CI;
6. verify security-sensitive behaviour in an isolated authorised test environment;
7. merge only after the build and analysis checks pass.

## Security reporting

Follow [`SECURITY.md`](SECURITY.md). Do not publish credentials, live targets or weaponised proof-of-concept material in a public issue.

## Contributing

Prefer small, reviewable changes. Good migration units include dependency updates, memory-safety fixes, x64 readiness, Unicode conversion, RAII adoption, build reproducibility and defensive security hardening.

Do not commit Visual Studio caches, local build products, `vcpkg_installed/`, generated binaries or user-specific project settings.

## Licence and authorised use

Use the code only where you have permission to do so, and comply with applicable law, organisational policy and the licences of included dependencies.
