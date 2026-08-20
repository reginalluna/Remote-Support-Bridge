# Modernization status

This repository is a legacy Windows/MFC remote-administration codebase. The modernization work in this branch is intentionally limited to non-functional build metadata, repository hygiene, and documentation.

## Completed

- Updated the solution metadata for Visual Studio 2022.
- Added modern Visual Studio/MSBuild ignore rules.
- Removed tracked IDE caches, resource-editor caches, build logs, and user-specific project settings.
- Documented the major compatibility blockers below.

## Compatibility blockers still present

### Dependency age

The tree vendors zlib 1.1.4 (2002). It should not be treated as a current dependency. Replacing it requires a protocol-compatibility and security review before changing deployed behaviour.

### 32-bit assumptions

The source contains pointer/integer conversions and Win32-only project configuration. Do not enable x64 until these are audited and corrected.

### Character encoding

The server project uses the Multi-Byte character set. A Unicode conversion should be done separately because changing it globally can alter Windows API call behaviour and data handling.

### Legacy project files

The client still contains Visual C++ 6-era project files. They are retained because deleting them without a verified replacement would reduce reproducibility.

## Security boundary

The source includes remote screen control, shell access, file management, registry/service management, audio/video capture, and network-control components. This branch does not optimize, extend, port, accelerate, or otherwise strengthen those capabilities.

For a legitimate modern remote-support application, prefer a current maintained remote-support stack with explicit authentication, encryption, consent UI, session logging, least-privilege operation, and automatic security updates rather than extending this legacy implementation.
