# Modernisation status

The codebase is being moved to a 2026 Windows/MSVC baseline while preserving controlled compatibility with the existing Win32 application output.

## Completed

- Updated solution metadata for Visual Studio 2026.
- Pinned the supported compiler toolset to MSVC `v145`.
- Pinned the Windows 11 SDK to `10.0.28000.2526`.
- Enabled C++20, conformance mode and x64-hosted compiler tools repository-wide.
- Added vcpkg manifest mode and moved the server project from the bundled zlib 1.1.4-era files to maintained `zlib` restored through vcpkg.
- Removed the obsolete server-side `zlib.h`, `zconf.h` and `zlib.lib` copies.
- Fixed the shared server/client buffer implementation so pointer subtraction is pointer-sized and buffer growth rejects arithmetic overflow.
- Added modern Visual Studio/MSBuild ignore rules, including `vcpkg_installed/`.
- Added repository-wide MSBuild hardening:
  - `/W4` warnings;
  - SDL checks;
  - `/GS` stack-buffer protection;
  - Control Flow Guard;
  - ASLR-compatible linking;
  - DEP/NX-compatible linking;
  - `asInvoker` execution and UIAccess disabled;
  - CFG-compatible Program Database debug information.
- Added Debug/Release Windows build CI using `actions/checkout@v7` and Visual Studio discovery through `vswhere`.
- Added CodeQL C/C++ v4 analysis for pushes, pull requests and scheduled scans.
- Added `SECURITY.md` and secure-redesign documentation.
- Updated `README.md` for the current August 2026 development baseline.

## Remaining compatibility work

### x64 output

The compiler host now uses x64 tools, but the application output is still Win32. Additional pointer/integer assumptions and third-party/client build assumptions must be audited before adding an x64 output configuration.

### Unicode

The server project still uses the Multi-Byte character set. A global Unicode switch would currently break ANSI-specific casts and API assumptions. Convert call sites deliberately, then change the project character set once the source is clean.

### Client project

The client still contains Visual C++ 6-era project files and historical prebuilt artefacts. A current client build needs a new reviewed project definition rather than an automatic conversion of the old DSP/DSW files.

### Privileged command architecture

The historical command set contains remote shell, screen-control, file-management, process/window-management, registry/service-management and audio/video functions. These paths predate current authentication, encrypted transport, authorisation, consent and audit requirements.

For current use, redesign privileged remote operations around a new security boundary rather than assuming the historical protocol is safe because the compiler and dependencies are current.

## Next migration order

1. Keep Debug and Release Win32 CI green on the maintained zlib/vcpkg dependency.
2. Work through remaining compiler and CodeQL findings, prioritising memory ownership, bounds checks and pointer-sized types.
3. Convert ANSI/Multi-Byte call sites to Unicode-safe Win32/MFC usage.
4. Add x64 output only after pointer/integer assumptions are clean and CI can test both architectures.
5. Replace the Visual C++ 6-era client project with a reviewed current build definition after removing historical prebuilt artefacts.
6. Convert raw Win32 resource ownership to RAII where it reduces leak/use-after-free risk.
7. Keep authentication, authorisation, consent, encrypted transport, session expiry and audit logging as explicit security boundaries for any future remote-support implementation.
