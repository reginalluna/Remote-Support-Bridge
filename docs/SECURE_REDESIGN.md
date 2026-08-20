# Secure redesign direction

The historical protocol in this repository should not be promoted directly into a production remote-support protocol. A modern implementation should place a new security boundary around every sensitive action instead of adding isolated checks to individual legacy commands.

## Implemented baseline

The new [`modern/`](../modern/) application now establishes the first security boundary independently of the historical command protocol:

- native x64, Unicode and C++20 build;
- explicit visible consent before local session initialisation;
- 128-bit session identifiers generated with Windows CNG;
- local UTC audit records with fail-closed behaviour if an approved session cannot be recorded;
- no administrator elevation request;
- no network listener and no privileged remote action in the initial baseline;
- modern MSVC exploit mitigations and automated x64 Debug/Release builds with a non-interactive self-test.

This baseline is intentionally small. It gives subsequent authentication, transport and capability work a reviewable place to attach without inheriting the legacy protocol's trust assumptions.

## Security goals

A replacement design should provide:

- authenticated operator and device identity;
- encrypted, integrity-protected transport;
- explicit authorisation for every sensitive capability;
- visible user consent where appropriate;
- least privilege and privilege separation;
- short-lived sessions and replay resistance;
- tamper-evident audit records;
- signed software/update artefacts;
- dependency and build provenance;
- revocation, incident-response and key-rotation mechanisms.

## Suggested architecture

### 1. Identity plane

Use a dedicated identity provider or device-enrolment authority. Operators and endpoints should have distinct identities. Do not use a shared static password or a secret embedded in the executable.

Prefer short-lived credentials and explicit revocation over long-lived bearer secrets.

### 2. Transport plane

Use a maintained TLS implementation with certificate validation and mutual authentication where appropriate. Transport security must include hostname/device identity checks, modern protocol versions, key rotation and certificate expiry handling.

Do not design a custom cryptographic protocol.

### 3. Authorisation plane

Define capabilities such as view-screen, control-input, transfer-file or inspect-system as separate permissions. Grant the minimum required permissions to each operator/session.

High-impact operations should require stronger policy and, where appropriate, a fresh user confirmation.

### 4. Consent and user visibility

Sensitive support activity should be visible to the endpoint user. A modern client should provide a clear consent prompt, active-session indicator and a reliable way to terminate the session.

Do not rely on hidden background execution as the normal operating model.

### 5. Privilege separation

Run the normal client without administrative privileges. If a narrowly scoped privileged operation is genuinely required, isolate it behind a small broker with explicit policy rather than running the entire remote-support process elevated.

### 6. Audit and monitoring

Record security-relevant events such as authentication, authorisation decisions, session start/end, sensitive capability use and update installation. Logs should avoid storing unnecessary secrets while remaining suitable for incident investigation.

For high-assurance deployments, protect audit records against tampering and centralise them outside the endpoint.

### 7. Update and supply-chain security

Require signed releases and signed update metadata. Maintain an inventory of third-party dependencies and scan them continuously. Builds should be reproducible enough to support provenance and release verification.

### 8. Abuse resistance

Apply connection limits, request limits, lockout/back-off behaviour and anomaly monitoring at the service boundary. Deny by default when identity or policy checks cannot be completed.

## Migration strategy

1. Keep the historical implementation separate for comparison and defensive review.
2. Build the new x64 session shell independently of legacy command tokens. **Completed.**
3. Add authenticated encrypted transport and operator/device identity to the new application.
4. Add session expiry, replay resistance and key rotation.
5. Make audit records tamper-evident and suitable for central collection.
6. Add individual support capabilities only after their policy, consent and audit requirements are defined and tested.
7. Require security review and threat-model updates for every newly enabled capability.

## Non-goal

The redesign should not aim to make the historical unauthenticated command protocol more deployable. The security boundary should be new, explicit and independently reviewable.
