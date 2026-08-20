# Security policy

## Project status

This repository contains a legacy Windows/MFC remote-administration codebase. It includes functionality for remote shell access, screen control, file management, process/window management, registry/service management, audio/video capture and network communication.

The code predates current security expectations and must **not** be treated as a production-ready remote-support product.

## Supported security posture

The supported posture of the current repository is:

- source review and maintenance;
- controlled compatibility work;
- defensive static analysis;
- isolated laboratory testing on systems you own or are explicitly authorised to test;
- least-privilege local execution;
- no direct exposure of the legacy protocol to the public Internet.

The repository now applies MSVC hardening defaults to MSBuild-based C/C++ projects, including higher warning levels, SDL checks, stack-buffer protection, Control Flow Guard, ASLR, DEP and `asInvoker` execution.

These mitigations reduce exploitability of memory-safety defects but do **not** make the legacy protocol secure.

## Production security requirements

A genuinely modern remote-support product built from this historical code would require an architectural redesign before deployment. At minimum it would need:

- mutually authenticated encrypted transport using a maintained TLS implementation;
- strong device and operator identity;
- explicit authorisation and role-based access control;
- visible end-user consent for sensitive operations;
- least-privilege service design and privilege separation;
- cryptographically signed updates and binaries;
- protected credential storage;
- replay protection and session expiry;
- tamper-evident security logging and audit trails;
- secure defaults with dangerous functions disabled unless explicitly authorised;
- rate limiting, connection limits and abuse controls;
- dependency inventory and automated vulnerability scanning;
- reproducible builds and CI security checks;
- documented incident-response and key-rotation procedures.

Do not retrofit these controls superficially around the existing unauthenticated command protocol and assume that the result is secure.

## Dependency risk

The repository still contains an obsolete zlib 1.1.4-era dependency. Treat it as technical debt that must be replaced with a currently maintained release before any production consideration. Replacement must be verified against the existing compression/data format before removing the historical files.

## 32-bit and Unicode limitations

The codebase still contains Win32 assumptions and Multi-Byte text handling. Do not enable x64 or global Unicode conversion blindly; both require source-level auditing and regression testing.

## Vulnerability reporting

If you find a vulnerability, avoid publishing working exploitation details, credentials, live targets or weaponised proof-of-concept material in a public issue.

Use GitHub private vulnerability reporting/security advisories if enabled for this repository. If private reporting is unavailable, open a minimal issue stating that you have a security concern and request a private contact channel without including exploit details.

## Security review boundary

Changes that improve defensive build hardening, memory safety, dependency health, authentication design, consent, logging, least privilege or vulnerability detection are welcome.

Changes whose primary effect is to make the legacy remote-control capabilities more deployable, stealthy, persistent, scalable or easier to operate against third-party systems are outside the supported modernisation scope.
