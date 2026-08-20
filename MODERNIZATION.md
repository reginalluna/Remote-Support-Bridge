# Modernization assessment

This repository is a legacy Windows/MFC C++ codebase dating from the Visual Studio 2010 era. The safest modernization path is incremental: first make the project auditable and reproducible, then replace obsolete dependencies and unsafe 32-bit assumptions before changing behaviour.

## Current state

- The server solution metadata targets Visual Studio 2010.
- The server project is Win32-only and uses MFC statically.
- The project uses the Multi-Byte character set rather than Unicode.
- A bundled zlib 1.1.4 header/library is present. zlib 1.1.4 dates from 2002 and should not be used in a modern build.
- Several source files use Windows-specific integer and pointer assumptions that need review before adding x64 configurations.
- The repository includes generated/IDE state files and prebuilt binaries from old Visual C++ toolchains.
- The codebase contains remote administration features including screen, file, registry, services, shell, audio/video, and network-control components. These should be treated as security-sensitive code and audited before any functional modernization.

## Recommended migration order

1. **Toolchain metadata**
   - Open and save the solution with Visual Studio 2022.
   - Keep Win32 initially; do not add x64 until pointer/integer truncation issues are fixed.
   - Use the current MSVC v143 toolset and Windows 10/11 SDK.

2. **Dependency cleanup**
   - Remove the vendored zlib 1.1.4 headers/library.
   - Replace it with a maintained zlib release obtained through a normal dependency mechanism (for example vcpkg or a documented system package).
   - Do not mix old static libraries produced by legacy Visual C++ toolchains with current MSVC builds.

3. **Compiler hardening**
   - Raise warnings gradually (`/W4`) and fix warnings rather than globally suppressing them.
   - Enable SDL checks and standard-conformance mode where compatible.
   - Keep warnings-as-errors off until the existing warning backlog is understood.

4. **64-bit readiness**
   - Audit pointer-to-integer conversions and arithmetic that stores pointers in `ULONG`, `DWORD`, `LONG`, or `int`.
   - Replace pointer-sized arithmetic with `size_t`, `uintptr_t`, `ULONG_PTR`, or `DWORD_PTR` as appropriate.
   - Only then introduce x64 build configurations.

5. **Text/Unicode**
   - Migrate from Multi-Byte to Unicode in a separate change.
   - Replace narrow Win32 API assumptions with `W` APIs or TCHAR-independent modern C++ strings.
   - Verify resource files and Chinese text encoding during the migration.

6. **Repository hygiene**
   - Stop tracking Visual Studio user/cache outputs such as `.ncb`, `.opt`, `.plg`, `.aps`, and `.vcxproj.user` unless there is a specific reproducibility reason.
   - Do not commit newly built `.exe`, `.dll`, `.obj`, `.pdb`, or intermediate output.
   - Add a `.gitignore` appropriate for Visual Studio/C++.

7. **Security review before behaviour changes**
   - Treat networking, remote shell, screen capture/control, file transfer, registry manipulation, service manipulation, audio/video capture, and persistence-related code as security-sensitive.
   - Document authentication, authorization, transport security, consent, logging, and threat model before modernizing those behaviours.
   - Prefer removing unused remote-control features rather than carrying them forward automatically.

## Intentionally not changed in this pass

This pass does **not** modernize or extend remote-control behaviour. It does not add new command execution, persistence, capture, evasion, deployment, or remote-management capabilities. It also does not add x64 yet, because the source contains legacy 32-bit assumptions that must be audited first.

## Suggested next safe change

The next low-risk change is repository hygiene: add a Visual Studio `.gitignore`, remove obsolete IDE-generated files from tracking, and document a clean Visual Studio 2022 build environment. After that, replace the obsolete zlib dependency without changing application protocol or remote-control behaviour.
