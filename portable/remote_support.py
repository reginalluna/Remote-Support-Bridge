#!/usr/bin/env python3

import datetime as dt
import os
import platform
import re
import secrets
import shutil
import subprocess
import sys
from pathlib import Path
import tkinter as tk
from tkinter import messagebox

APP_NAME = "Windows Remote Support"
SAFE_TARGET = re.compile(r"^[A-Za-z0-9._:\-\[\]]{1,255}$")


def generate_session_id() -> str:
    return secrets.token_hex(16)


def is_safe_target(target: str) -> bool:
    return bool(SAFE_TARGET.fullmatch(target.strip()))


def audit_path() -> Path:
    system = platform.system()
    if system == "Darwin":
        base = Path.home() / "Library" / "Logs" / "WindowsRemoteSupport"
    else:
        state_home = Path(os.environ.get("XDG_STATE_HOME", Path.home() / ".local" / "state"))
        base = state_home / "windows-remote-support"
    base.mkdir(parents=True, exist_ok=True)
    return base / "audit.log"


def write_audit(session_id: str, event: str, target: str = "") -> bool:
    try:
        stamp = dt.datetime.now(dt.timezone.utc).replace(microsecond=0).isoformat().replace("+00:00", "Z")
        suffix = f" target={target}" if target else ""
        with audit_path().open("a", encoding="utf-8") as handle:
            handle.write(f"{stamp} session={session_id} event={event}{suffix}\n")
        return True
    except OSError:
        return False


def open_uri(uri: str) -> bool:
    system = platform.system()
    try:
        if system == "Darwin":
            subprocess.Popen(["open", uri], close_fds=True)
        elif system == "Linux":
            opener = shutil.which("xdg-open")
            if not opener:
                return False
            subprocess.Popen([opener, uri], close_fds=True)
        else:
            return False
        return True
    except OSError:
        return False


def launch_ssh(target: str) -> bool:
    system = platform.system()
    try:
        if system == "Darwin":
            return open_uri(f"ssh://{target}")
        if system == "Linux":
            terminal = shutil.which("x-terminal-emulator")
            ssh = shutil.which("ssh")
            if terminal and ssh:
                subprocess.Popen([terminal, "-e", ssh, target], close_fds=True)
                return True
            return open_uri(f"ssh://{target}")
    except OSError:
        return False
    return False


def launch_protocol(protocol: str, target: str) -> bool:
    if protocol == "ssh":
        return launch_ssh(target)
    return open_uri(f"{protocol}://{target}")


def run_self_test() -> int:
    first = generate_session_id()
    second = generate_session_id()
    checks = [
        len(first) == 32,
        len(second) == 32,
        first != second,
        all(ch in "0123456789abcdef" for ch in first),
        is_safe_target("lab-host-01"),
        is_safe_target("192.168.10.25"),
        is_safe_target("[2001:db8::10]"),
        not is_safe_target("host name"),
        not is_safe_target("host;rm"),
    ]
    return 0 if all(checks) else 1


class RemoteSupportApp:
    def __init__(self, root: tk.Tk) -> None:
        self.root = root
        self.session_id = ""
        self.root.title(APP_NAME)
        self.root.geometry("650x470")
        self.root.minsize(620, 440)

        title = tk.Label(root, text=APP_NAME, font=("TkDefaultFont", 18, "bold"))
        title.pack(anchor="w", padx=24, pady=(22, 6))

        subtitle = tk.Label(
            root,
            text="Consent-first remote-support controller using established RDP, SSH/SFTP and VNC clients.",
            justify="left",
            wraplength=590,
        )
        subtitle.pack(anchor="w", padx=24, pady=(0, 16))

        self.status = tk.StringVar(value="Status: ready - no active support session")
        tk.Label(root, textvariable=self.status, relief="groove", anchor="w", padx=10, pady=10).pack(
            fill="x", padx=24
        )

        self.session = tk.StringVar(value="Session: none")
        tk.Label(root, textvariable=self.session, anchor="w").pack(fill="x", padx=24, pady=(12, 2))
        tk.Label(root, text=f"Controller platform: {platform.system()} {platform.machine()}", anchor="w").pack(
            fill="x", padx=24, pady=(0, 14)
        )

        tk.Label(root, text="Remote computer name or IP address:", anchor="w").pack(fill="x", padx=24)
        self.target = tk.Entry(root)
        self.target.pack(fill="x", padx=24, pady=(4, 14))

        protocol_row = tk.Frame(root)
        protocol_row.pack(fill="x", padx=24)
        self.protocol_buttons = []
        for label, protocol in (
            ("RDP desktop", "rdp"),
            ("SSH terminal", "ssh"),
            ("SFTP files", "sftp"),
            ("VNC desktop", "vnc"),
        ):
            button = tk.Button(protocol_row, text=label, command=lambda p=protocol: self.connect(p), state="disabled")
            button.pack(side="left", expand=True, fill="x", padx=3)
            self.protocol_buttons.append(button)

        session_row = tk.Frame(root)
        session_row.pack(fill="x", padx=24, pady=(22, 0))
        tk.Button(session_row, text="Start consented session", command=self.start_session).pack(
            side="left", expand=True, fill="x", padx=3
        )
        self.end_button = tk.Button(session_row, text="End session", command=self.end_session, state="disabled")
        self.end_button.pack(side="left", expand=True, fill="x", padx=3)
        tk.Button(session_row, text="Open audit log", command=self.open_audit_log).pack(
            side="left", expand=True, fill="x", padx=3
        )
        tk.Button(session_row, text="About", command=self.show_about).pack(side="left", expand=True, fill="x", padx=3)

        help_text = (
            "RDP/VNC/SFTP require a compatible client or URL handler on this computer. "
            "SSH uses the local terminal/OpenSSH where available. Authentication remains in the selected client."
        )
        tk.Label(root, text=help_text, justify="left", wraplength=590).pack(anchor="w", padx=24, pady=(22, 0))

        self.root.protocol("WM_DELETE_WINDOW", self.on_close)

    def refresh(self) -> None:
        active = bool(self.session_id)
        self.status.set(
            "Status: consent granted - support session active" if active else "Status: ready - no active support session"
        )
        self.session.set(f"Session: {self.session_id}" if active else "Session: none")
        state = "normal" if active else "disabled"
        self.end_button.configure(state=state)
        for button in self.protocol_buttons:
            button.configure(state=state)

    def start_session(self) -> None:
        if self.session_id:
            messagebox.showinfo(APP_NAME, "A support session is already active.")
            return
        session_id = generate_session_id()
        approved = messagebox.askyesno(
            APP_NAME,
            f"Start an authorised support session on this computer?\n\nSession: {session_id}\n\n"
            "Remote authentication remains in the selected RDP, SSH/SFTP or VNC client.",
            default="no",
        )
        if not approved:
            write_audit(session_id, "consent_denied")
            return
        if not write_audit(session_id, "consent_granted"):
            messagebox.showerror(APP_NAME, "The audit record could not be written, so the session will not start.")
            return
        self.session_id = session_id
        self.refresh()

    def end_session(self) -> None:
        if self.session_id:
            write_audit(self.session_id, "session_ended")
            self.session_id = ""
            self.refresh()

    def connect(self, protocol: str) -> None:
        if not self.session_id:
            return
        target = self.target.get().strip()
        if not is_safe_target(target):
            messagebox.showwarning(
                APP_NAME,
                "Enter a valid hostname or IP address. Only letters, numbers, dots, hyphens, colons and IPv6 brackets are accepted.",
            )
            return
        if not write_audit(self.session_id, f"{protocol}_launch_requested", target):
            messagebox.showerror(APP_NAME, "The connection was blocked because the audit event could not be written.")
            return
        if not launch_protocol(protocol, target):
            write_audit(self.session_id, f"{protocol}_launch_failed", target)
            messagebox.showerror(
                APP_NAME,
                f"No compatible {protocol.upper()} client or URL handler could be opened on this computer.",
            )
            return
        write_audit(self.session_id, f"{protocol}_client_started", target)

    def open_audit_log(self) -> None:
        path = audit_path()
        if not path.exists():
            messagebox.showinfo(APP_NAME, "There are no audit records to display yet.")
            return
        if platform.system() == "Darwin":
            subprocess.Popen(["open", str(path)], close_fds=True)
        elif platform.system() == "Linux":
            opener = shutil.which("xdg-open")
            if opener:
                subprocess.Popen([opener, str(path)], close_fds=True)
                return
            messagebox.showinfo(APP_NAME, str(path))

    def show_about(self) -> None:
        messagebox.showinfo(
            APP_NAME,
            f"{APP_NAME}\n\n{platform.system()} {platform.machine()} controller\n\n"
            "Consent-first session UI with audited hand-offs to established remote-access clients.\n"
            "The application does not store remote passwords.",
        )

    def on_close(self) -> None:
        if self.session_id:
            write_audit(self.session_id, "session_closed")
        self.root.destroy()


def main() -> int:
    if len(sys.argv) == 2 and sys.argv[1] == "--self-test":
        return run_self_test()
    if platform.system() not in {"Darwin", "Linux"}:
        print("This portable controller build targets macOS and Linux.", file=sys.stderr)
        return 1
    root = tk.Tk()
    RemoteSupportApp(root)
    root.mainloop()
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
