# Modernisation status

Remote Support Bridge is the maintained cross-platform controller in this repository. The current application is separate from the historical Windows/MFC command implementation and delegates remote sessions to established RDP, SSH/SFTP and VNC clients.

The historical `client/` and `server/` trees remain available for comparison, compatibility study and defensive review; they are not part of the supported controller build.

## Completed

- Added native Windows x86 and x64 C++20 controller builds under `modern/`.
- Added packaged macOS and Ubuntu/Linux controller builds under `portable/`.
- Added explicit local consent before a support session can begin.
- Added random session identifiers and UTC audit logging.
- Added target validation and fail-closed audit checks before protocol hand-offs.
- Added RDP, SSH, SFTP and VNC hand-offs through installed operating-system clients.
- Added Windows compiler/linker hardening including `/W4`, SDL checks, `/GS`, Control Flow Guard, ASLR, DEP/NX and x64 CET-compatible linking.
- Added CodeQL analysis, Windows build/self-test CI and release packaging.
- Added encapsulated Windows EXE, macOS DMG and Linux AppImage release artefacts.
- Kept the historical privileged command protocol outside the maintained application.

## Current architecture

### Native Windows controller

`modern/` contains the Windows C++20 application. It builds for Win32/x86 and x64 and uses Windows-native security and UI APIs.

### Portable macOS/Linux controller

`portable/remote_support.py` provides the macOS and Linux controller UI. Release builds package the runtime so destination computers do not need a separate Python installation.

### Remote-access hand-offs

Remote Support Bridge does not implement a new background control service. It validates and audits the target, then opens an established local RDP, SSH/SFTP or VNC client. Authentication and encryption are therefore provided by the selected protocol implementation.

## Historical compatibility notes

The retained Windows/MFC source contains old project formats, Win32 assumptions, Multi-Byte text handling and obsolete dependencies. These constraints apply to the historical reference trees, not to the supported Remote Support Bridge controller.

The historical protocol also predates current expectations for mutual authentication, encrypted transport, explicit consent, role-based authorisation and tamper-evident auditing. It should not be treated as the basis of the maintained application.

## Current priorities

1. Keep Windows, macOS and Linux packaging reproducible and self-tested.
2. Add trusted code signing/notarisation for public distribution.
3. Keep dependency and CodeQL checks current.
4. Improve platform-specific protocol-handler detection and user-facing error reporting.
5. Maintain a clear separation between the supported controller and historical reference code.
6. Extend security documentation whenever a new user-visible capability is added.
