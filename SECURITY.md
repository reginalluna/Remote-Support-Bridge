# Security policy

## Supported application

Remote Support Bridge is the maintained application in this repository. Supported controller code lives in:

- `modern/` for native Windows x86/x64 builds;
- `portable/` for packaged macOS and Ubuntu/Linux builds.

The historical Windows/MFC material under `client/` and `server/` is retained for comparison and defensive analysis. It is not part of the supported controller and should not be deployed as a production remote-support service.

## Security model

Remote Support Bridge requires visible local consent before enabling connection hand-offs. It generates a random session identifier, validates the requested target, records security-relevant events and does not store remote passwords.

The controller does not embed a custom privileged remote-control listener. Instead, it opens established RDP, SSH/SFTP or VNC clients available on the local operating system. Authentication, encryption and remote-side authorisation are provided by those protocol implementations and their configuration.

Supported security practices include:

- explicit local session approval;
- least-privilege execution;
- UTC audit logging;
- fail-closed behaviour when an approved hand-off cannot be audited;
- validated host/IP input;
- no stored remote credentials;
- current CI and CodeQL checks;
- Windows exploit mitigations such as `/GS`, Control Flow Guard, ASLR and DEP/NX.

## Platform packaging

The first public release provides native Windows executables, macOS DMGs and a Linux x86_64 AppImage. The initial release is not signed with a production Windows certificate or Apple Developer ID. Users should obtain release files only from this repository and verify their source before execution.

Production distribution should add:

- Windows Authenticode signing;
- Apple Developer ID signing and notarisation;
- signed update metadata if automatic updates are introduced;
- reproducible release provenance and dependency inventory.

## Historical source boundary

The retained legacy source contains remote shell, screen control, file management, process/window management, registry/service management, audio/video capture and network-control functionality. It also contains obsolete dependencies and historical trust assumptions.

Changes to the maintained application must not silently activate or link the historical command protocol. Any migration work should preserve a clear architectural separation and require explicit review of identity, consent, authorisation and audit behaviour.

## Vulnerability reporting

Do not publish credentials, live targets, weaponised proof-of-concept material or sensitive exploitation details in a public issue.

Use GitHub private vulnerability reporting/security advisories when available. If private reporting is unavailable, open a minimal issue stating that you have a security concern and request a private contact channel without including exploitation details.

## Security review boundary

Changes that improve consent, authentication design, auditability, dependency health, memory safety, build hardening, least privilege or vulnerability detection are welcome.

Changes whose primary effect would be to make the historical privileged command implementation more stealthy, persistent, scalable or easier to deploy against third-party systems are outside the supported project scope.
