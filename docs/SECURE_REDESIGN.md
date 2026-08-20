# Secure redesign direction

Remote Support Bridge is the maintained redesign in this repository. It keeps the supported controller separate from the historical Windows/MFC command protocol and uses explicit consent, local auditing and established remote-access clients as its current security boundary.

## Implemented baseline

The maintained application now includes:

- native Windows x86/Win32 and x64 C++20 controller builds under [`modern/`](../modern/);
- packaged macOS ARM64 and x86_64 controller applications and an Ubuntu/Linux x86_64 AppImage under [`portable/`](../portable/);
- explicit visible consent before a local support session is enabled;
- random 128-bit session identifiers;
- UTC audit logging with fail-closed checks before connection hand-offs;
- validated host/IP input;
- no stored remote passwords;
- RDP, SSH/SFTP and VNC hand-offs through established local clients;
- no requested administrator elevation in the controller;
- Windows exploit mitigations including `/GS`, Control Flow Guard, ASLR, DEP/NX and x64 CET-compatible linking;
- CI self-tests, CodeQL analysis and release packaging for the supported platforms.

The controller does not implement a new privileged remote-control listener. Authentication, encryption and remote-side authorisation remain the responsibility of the RDP, SSH/SFTP or VNC implementation selected by the user.

## Security goals

The project should continue to provide:

- clear operator and endpoint identity where a protocol supports it;
- encrypted, integrity-protected transport supplied by maintained protocol implementations;
- explicit local approval before sensitive support activity;
- least-privilege execution;
- short-lived local session state;
- auditable session start/end and protocol hand-offs;
- signed release artefacts and provenance as distribution matures;
- dependency and build-health monitoring;
- a reliable user-visible way to end the local support session.

## Architecture

### 1. Controller boundary

Remote Support Bridge owns local consent, target validation, session state and audit logging. These controls must succeed before a remote-access client is opened.

### 2. Protocol boundary

RDP, SSH/SFTP and VNC are delegated to installed operating-system clients or registered handlers. The project should prefer maintained platform implementations rather than designing a new cryptographic or remote-control protocol.

### 3. Consent and visibility

Connection controls remain unavailable until the local user approves a support session. Session termination and protocol hand-offs are security-relevant events and should remain visible and auditable.

### 4. Credential handling

Remote credentials are entered into the selected protocol client rather than stored by Remote Support Bridge. Do not add shared static passwords or secrets embedded in release binaries.

### 5. Privilege separation

The normal controller should continue to run without administrative privileges. If a future feature genuinely requires elevation, keep it narrowly scoped and separate from the main controller process.

### 6. Audit and monitoring

Record consent decisions, session start/end and protocol hand-offs without storing unnecessary secrets. Higher-assurance deployments can centralise or make these records tamper-evident without changing the local consent boundary.

### 7. Release and supply-chain security

Add trusted Windows signing, Apple Developer ID signing/notarisation and release provenance before treating public distribution as mature. Keep third-party build dependencies current and monitored.

## Historical source boundary

The `client/` and `server/` trees are historical reference material. They should remain separate from the maintained controller and must not become an implicit dependency of the supported release path.

The historical protocol predates modern mutual authentication, encrypted transport, explicit consent and role-based authorisation. The redesign should not attempt to make that protocol the production transport for Remote Support Bridge.

## Current migration status

1. Separate maintained controller code from the historical command protocol. **Completed.**
2. Provide native Windows x86/x64 consent and audit UI. **Completed.**
3. Provide packaged macOS and Linux controllers with the same local consent/audit model. **Completed.**
4. Delegate remote sessions to established RDP, SSH/SFTP and VNC clients. **Completed.**
5. Add trusted cross-platform release signing and notarisation. **Pending.**
6. Improve protocol-handler discovery and error reporting. **Pending.**
7. Extend the threat model whenever a new capability changes the trust boundary. **Ongoing.**

## Non-goal

The redesign is not intended to make the historical unauthenticated privileged command protocol more deployable. The maintained security boundary should remain explicit, reviewable and separate from that code.
