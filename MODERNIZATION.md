# Modernisation status

This repository is a legacy Windows/MFC remote-administration codebase. Modernisation is being performed defensively: build/tooling, exploit mitigations, dependency health, code quality and security governance can be improved without extending the operational reach of the legacy remote-control protocol.

## Completed

- Updated the solution metadata for Visual Studio 2022.
- Added modern Visual Studio/MSBuild ignore rules.
- Removed tracked IDE caches, resource-editor caches, build logs and user-specific project settings.
- Added repository-wide MSBuild hardening defaults for current C/C++ projects:
  - `/W4` warnings;
  - SDL checks;
  - `/GS` stack-buffer protection;
  - Control Flow Guard;
  - ASLR-compatible linking;
  - DEP/NX-compatible linking;
  - `asInvoker` execution and UIAccess disabled.
- Added a CodeQL C/C++ workflow for pushes, pull requests and scheduled analysis.
- Added `SECURITY.md` with a modern threat/deployment baseline and vulnerability-reporting guidance.
- Updated the README with build, mitigation-verification and security guidance.

## Compatibility blockers still present

### Dependency age

The tree vendors zlib 1.1.4-era headers/libraries. They should not be treated as a current dependency. Replacement should use a maintained zlib release and must be verified against the existing compressed data format before the historical files are removed.

### 32-bit assumptions

The source contains pointer/integer conversions and Win32-only project configuration. Do not enable x64 until these are audited and corrected.

### Character encoding

The server project uses the Multi-Byte character set. A Unicode conversion should be done separately because changing it globally can alter Windows API calls, resource handling and data formats.

### Legacy project files

The client still contains Visual C++ 6-era project files. They are retained because deleting them without a verified replacement would reduce reproducibility and auditability.

### Legacy protocol security

The historical remote protocol has no verified modern mutual authentication, encrypted transport, role-based authorisation, explicit user consent or tamper-evident audit model.

Compiler/linker mitigations reduce exploitability of some implementation defects; they do not solve protocol-level trust problems.

## Security boundary

The source includes remote screen control, shell access, file management, process/window management, registry/service management, audio/video capture and network-control components.

Modernising those functions into a production-ready remote administration stack would require a new architecture with authenticated encrypted transport, operator/device identity, least privilege, explicit consent, signed updates, protected credentials, audit logging and abuse controls. That redesign should be treated as a separate security-engineering project rather than a superficial retrofit to the legacy command protocol.

## Recommended next defensive work

1. Establish a consistently green Visual Studio 2022 Win32 build under CodeQL/CI.
2. Replace the obsolete zlib dependency and document the maintained version.
3. Fix compiler warnings and memory/type-safety findings instead of suppressing them.
4. Audit pointer-sized arithmetic and only then consider x64.
5. Migrate text handling to Unicode as a separate tested change.
6. Remove or redesign privileged legacy operations rather than carrying them forward unchanged.
7. Design a new authentication/authorisation/consent boundary before considering any production remote-support use.
