#include <windows.h>
#include <bcrypt.h>
#include <shellapi.h>
#include <shlobj.h>

#include <array>
#include <cstdio>
#include <cwctype>
#include <string>

namespace
{
constexpr wchar_t kAppName[] = L"Windows Remote Support";
constexpr wchar_t kWindowClass[] = L"WindowsRemoteSupportMainWindow";
constexpr int kStartSessionButton = 1001;
constexpr int kEndSessionButton = 1002;
constexpr int kOpenAuditButton = 1003;
constexpr int kAboutButton = 1004;
constexpr int kTargetEdit = 1005;
constexpr int kConnectRdpButton = 1006;
constexpr int kConnectSshButton = 1007;
constexpr int kConnectSftpButton = 1008;
constexpr int kConnectVncButton = 1009;

HWND gMainWindow = nullptr;
HWND gStatusText = nullptr;
HWND gSessionText = nullptr;
HWND gEndSessionButton = nullptr;
HWND gTargetEdit = nullptr;
HWND gConnectRdpButton = nullptr;
HWND gConnectSshButton = nullptr;
HWND gConnectSftpButton = nullptr;
HWND gConnectVncButton = nullptr;
std::string gSessionId;
std::wstring gAuditPath;

std::string GenerateSessionId()
{
    std::array<unsigned char, 16> bytes{};
    if (BCryptGenRandom(nullptr, bytes.data(), static_cast<ULONG>(bytes.size()), BCRYPT_USE_SYSTEM_PREFERRED_RNG) != 0)
    {
        return {};
    }

    constexpr char hex[] = "0123456789abcdef";
    std::string id;
    id.reserve(bytes.size() * 2);
    for (const unsigned char value : bytes)
    {
        id.push_back(hex[value >> 4]);
        id.push_back(hex[value & 0x0f]);
    }
    return id;
}

std::string UtcTimestamp()
{
    SYSTEMTIME now{};
    GetSystemTime(&now);

    char buffer[32]{};
    sprintf_s(
        buffer,
        "%04u-%02u-%02uT%02u:%02u:%02uZ",
        now.wYear,
        now.wMonth,
        now.wDay,
        now.wHour,
        now.wMinute,
        now.wSecond);
    return buffer;
}

std::wstring AuditLogPath()
{
    PWSTR localAppData = nullptr;
    if (FAILED(SHGetKnownFolderPath(FOLDERID_LocalAppData, KF_FLAG_CREATE, nullptr, &localAppData)))
    {
        return {};
    }

    std::wstring directory(localAppData);
    CoTaskMemFree(localAppData);
    directory += L"\\WindowsRemoteSupport";

    if (!CreateDirectoryW(directory.c_str(), nullptr) && GetLastError() != ERROR_ALREADY_EXISTS)
    {
        return {};
    }

    return directory + L"\\audit.log";
}

bool WriteAudit(const std::wstring& path, const std::string& sessionId, const char* eventName)
{
    const HANDLE file = CreateFileW(
        path.c_str(),
        FILE_APPEND_DATA,
        FILE_SHARE_READ,
        nullptr,
        OPEN_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        nullptr);

    if (file == INVALID_HANDLE_VALUE)
    {
        return false;
    }

    const std::string record = UtcTimestamp() + " session=" + sessionId + " event=" + eventName + "\r\n";
    DWORD written = 0;
    const BOOL ok = WriteFile(file, record.data(), static_cast<DWORD>(record.size()), &written, nullptr);
    CloseHandle(file);
    return ok && written == static_cast<DWORD>(record.size());
}

std::wstring ArchitectureName()
{
    return sizeof(void*) == 8 ? L"x64 (64-bit)" : L"x86 (32-bit)";
}

std::wstring SessionDisplayName()
{
    if (gSessionId.empty())
    {
        return L"Session: none";
    }

    return L"Session: " + std::wstring(gSessionId.begin(), gSessionId.end());
}

bool IsSafeTarget(const std::wstring& target)
{
    if (target.empty() || target.size() > 255)
    {
        return false;
    }

    for (const wchar_t value : target)
    {
        if (!iswalnum(value) && value != L'.' && value != L'-' && value != L':' && value != L'[' && value != L']')
        {
            return false;
        }
    }
    return true;
}

std::wstring ReadControlText(HWND control)
{
    const int length = GetWindowTextLengthW(control);
    if (length <= 0)
    {
        return {};
    }

    std::wstring text(static_cast<size_t>(length) + 1, L'\0');
    const int copied = GetWindowTextW(control, text.data(), length + 1);
    if (copied <= 0)
    {
        return {};
    }

    text.resize(static_cast<size_t>(copied));
    return text;
}

void RefreshSessionUi()
{
    SetWindowTextW(gStatusText, gSessionId.empty() ? L"Status: ready - no active support session" : L"Status: consent granted - support session active");
    const std::wstring session = SessionDisplayName();
    SetWindowTextW(gSessionText, session.c_str());

    const BOOL enabled = gSessionId.empty() ? FALSE : TRUE;
    EnableWindow(gEndSessionButton, enabled);
    EnableWindow(gConnectRdpButton, enabled);
    EnableWindow(gConnectSshButton, enabled);
    EnableWindow(gConnectSftpButton, enabled);
    EnableWindow(gConnectVncButton, enabled);
}

void StartLocalSession(HWND owner)
{
    if (!gSessionId.empty())
    {
        MessageBoxW(owner, L"A support session is already active. End it before starting another one.", kAppName, MB_OK | MB_ICONINFORMATION);
        return;
    }

    const std::string sessionId = GenerateSessionId();
    if (sessionId.empty())
    {
        MessageBoxW(owner, L"A cryptographically secure session identifier could not be generated.", kAppName, MB_OK | MB_ICONERROR);
        return;
    }

    if (gAuditPath.empty())
    {
        gAuditPath = AuditLogPath();
    }
    if (gAuditPath.empty())
    {
        MessageBoxW(owner, L"The audit-log location could not be initialised, so the session will not start.", kAppName, MB_OK | MB_ICONERROR);
        return;
    }

    const std::wstring sessionWide(sessionId.begin(), sessionId.end());
    const std::wstring prompt =
        L"Start an authorised support session on this computer?\n\nSession: " + sessionWide +
        L"\n\nAfter consent, this application can hand off to Windows RDP, OpenSSH/SFTP, or a registered VNC viewer. Those established clients remain responsible for remote authentication and authorisation.";

    const int choice = MessageBoxW(
        owner,
        prompt.c_str(),
        kAppName,
        MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2 | MB_SETFOREGROUND);

    if (choice != IDYES)
    {
        WriteAudit(gAuditPath, sessionId, "consent_denied");
        return;
    }

    if (!WriteAudit(gAuditPath, sessionId, "consent_granted"))
    {
        MessageBoxW(owner, L"Consent was granted, but the audit event could not be recorded. The session will not continue.", kAppName, MB_OK | MB_ICONERROR);
        return;
    }

    gSessionId = sessionId;
    RefreshSessionUi();
}

void EndLocalSession(HWND owner, const char* eventName)
{
    if (gSessionId.empty())
    {
        return;
    }

    if (!gAuditPath.empty() && !WriteAudit(gAuditPath, gSessionId, eventName))
    {
        MessageBoxW(owner, L"The session has been ended, but its final audit event could not be written.", kAppName, MB_OK | MB_ICONWARNING);
    }

    gSessionId.clear();
    RefreshSessionUi();
}

bool GetAuthorisedTarget(HWND owner, std::wstring& target)
{
    if (gSessionId.empty())
    {
        MessageBoxW(owner, L"Start and approve a support session before opening a remote client.", kAppName, MB_OK | MB_ICONINFORMATION);
        return false;
    }

    target = ReadControlText(gTargetEdit);
    if (!IsSafeTarget(target))
    {
        MessageBoxW(owner, L"Enter a valid computer name or IP address. Only letters, numbers, dots, hyphens, colons and IPv6 brackets are accepted.", kAppName, MB_OK | MB_ICONWARNING);
        return false;
    }

    return true;
}

void LaunchRemoteDesktop(HWND owner)
{
    std::wstring target;
    if (!GetAuthorisedTarget(owner, target))
    {
        return;
    }

    if (gAuditPath.empty() || !WriteAudit(gAuditPath, gSessionId, "rdp_launch_requested"))
    {
        MessageBoxW(owner, L"The RDP hand-off was blocked because its audit event could not be recorded.", kAppName, MB_OK | MB_ICONERROR);
        return;
    }

    const std::wstring arguments = L"/v:" + target;
    const HINSTANCE result = ShellExecuteW(owner, L"open", L"mstsc.exe", arguments.c_str(), nullptr, SW_SHOWNORMAL);
    if (reinterpret_cast<INT_PTR>(result) <= 32)
    {
        WriteAudit(gAuditPath, gSessionId, "rdp_launch_failed");
        MessageBoxW(owner, L"Windows Remote Desktop could not be started on this computer.", kAppName, MB_OK | MB_ICONERROR);
        return;
    }

    WriteAudit(gAuditPath, gSessionId, "rdp_client_started");
}

void LaunchOpenSshClient(HWND owner, const wchar_t* executable, const char* requestedEvent, const char* startedEvent, const char* failedEvent, const wchar_t* displayName)
{
    std::wstring target;
    if (!GetAuthorisedTarget(owner, target))
    {
        return;
    }

    if (gAuditPath.empty() || !WriteAudit(gAuditPath, gSessionId, requestedEvent))
    {
        MessageBoxW(owner, L"The client hand-off was blocked because its audit event could not be recorded.", kAppName, MB_OK | MB_ICONERROR);
        return;
    }

    const HINSTANCE result = ShellExecuteW(owner, L"open", executable, target.c_str(), nullptr, SW_SHOWNORMAL);
    if (reinterpret_cast<INT_PTR>(result) <= 32)
    {
        WriteAudit(gAuditPath, gSessionId, failedEvent);
        const std::wstring message = std::wstring(displayName) + L" could not be started. Install or enable the Windows OpenSSH Client feature, then try again.";
        MessageBoxW(owner, message.c_str(), kAppName, MB_OK | MB_ICONERROR);
        return;
    }

    WriteAudit(gAuditPath, gSessionId, startedEvent);
}

void LaunchVnc(HWND owner)
{
    std::wstring target;
    if (!GetAuthorisedTarget(owner, target))
    {
        return;
    }

    if (gAuditPath.empty() || !WriteAudit(gAuditPath, gSessionId, "vnc_launch_requested"))
    {
        MessageBoxW(owner, L"The VNC hand-off was blocked because its audit event could not be recorded.", kAppName, MB_OK | MB_ICONERROR);
        return;
    }

    const std::wstring uri = L"vnc://" + target;
    const HINSTANCE result = ShellExecuteW(owner, L"open", uri.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
    if (reinterpret_cast<INT_PTR>(result) <= 32)
    {
        WriteAudit(gAuditPath, gSessionId, "vnc_launch_failed");
        MessageBoxW(
            owner,
            L"No VNC viewer is registered for vnc:// links on this Windows computer. Install a trusted VNC viewer and enable Screen Sharing/VNC on the remote macOS or Linux computer.",
            kAppName,
            MB_OK | MB_ICONERROR);
        return;
    }

    WriteAudit(gAuditPath, gSessionId, "vnc_client_started");
}

void OpenAuditLog(HWND owner)
{
    if (gAuditPath.empty())
    {
        gAuditPath = AuditLogPath();
    }

    if (gAuditPath.empty() || GetFileAttributesW(gAuditPath.c_str()) == INVALID_FILE_ATTRIBUTES)
    {
        MessageBoxW(owner, L"There are no audit records to display yet.", kAppName, MB_OK | MB_ICONINFORMATION);
        return;
    }

    const HINSTANCE result = ShellExecuteW(owner, L"open", gAuditPath.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
    if (reinterpret_cast<INT_PTR>(result) <= 32)
    {
        MessageBoxW(owner, L"Windows could not open the audit log.", kAppName, MB_OK | MB_ICONERROR);
    }
}

void ShowAbout(HWND owner)
{
    const std::wstring message =
        L"Windows Remote Support\n\n"
        L"Architecture: " + ArchitectureName() +
        L"\n\nCross-platform target hand-offs:\n"
        L"- RDP: Windows and RDP-enabled Ubuntu desktops\n"
        L"- SSH/SFTP: macOS, Ubuntu and other SSH servers\n"
        L"- VNC: macOS Screen Sharing and VNC-enabled Linux desktops\n\n"
        L"The application requires explicit local consent and audits each hand-off. Authentication remains with the selected remote-access client.";
    MessageBoxW(owner, message.c_str(), L"About Windows Remote Support", MB_OK | MB_ICONINFORMATION);
}

bool IsSelfTestRequested()
{
    int argc = 0;
    LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);
    if (argv == nullptr)
    {
        return false;
    }

    const bool requested = argc == 2 && wcscmp(argv[1], L"--self-test") == 0;
    LocalFree(argv);
    return requested;
}

int RunSelfTest()
{
    const std::string first = GenerateSessionId();
    const std::string second = GenerateSessionId();
    const bool validCharacters = first.find_first_not_of("0123456789abcdef") == std::string::npos;
    const bool validArchitecture = ArchitectureName() == L"x64 (64-bit)" || ArchitectureName() == L"x86 (32-bit)";
    const bool validTargets = IsSafeTarget(L"lab-mac.local") && IsSafeTarget(L"192.168.10.20") && IsSafeTarget(L"[2001:db8::10]") && !IsSafeTarget(L"host /admin");
    return first.size() == 32 && second.size() == 32 && first != second && validCharacters && validArchitecture && validTargets ? 0 : 1;
}

void ApplyDefaultFont(HWND control)
{
    SendMessageW(control, WM_SETFONT, reinterpret_cast<WPARAM>(GetStockObject(DEFAULT_GUI_FONT)), TRUE);
}

HWND AddControl(
    DWORD exStyle,
    const wchar_t* className,
    const wchar_t* text,
    DWORD style,
    int x,
    int y,
    int width,
    int height,
    HWND parent,
    int id)
{
    HWND control = CreateWindowExW(
        exStyle,
        className,
        text,
        style,
        x,
        y,
        width,
        height,
        parent,
        reinterpret_cast<HMENU>(static_cast<INT_PTR>(id)),
        GetModuleHandleW(nullptr),
        nullptr);
    if (control != nullptr)
    {
        ApplyDefaultFont(control);
    }
    return control;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_CREATE:
    {
        HWND title = AddControl(0, L"STATIC", L"Windows Remote Support", WS_CHILD | WS_VISIBLE, 28, 24, 620, 32, hwnd, 0);
        ApplyDefaultFont(title);

        AddControl(
            0,
            L"STATIC",
            L"Consent-first Windows controller with audited hand-offs to standard remote-access clients.",
            WS_CHILD | WS_VISIBLE,
            28,
            62,
            620,
            42,
            hwnd,
            0);

        gStatusText = AddControl(WS_EX_CLIENTEDGE, L"STATIC", L"", WS_CHILD | WS_VISIBLE | SS_LEFT, 28, 122, 620, 42, hwnd, 0);
        gSessionText = AddControl(0, L"STATIC", L"", WS_CHILD | WS_VISIBLE, 28, 178, 620, 24, hwnd, 0);

        const std::wstring architecture = L"Application architecture: " + ArchitectureName();
        AddControl(0, L"STATIC", architecture.c_str(), WS_CHILD | WS_VISIBLE, 28, 206, 620, 24, hwnd, 0);

        AddControl(0, L"STATIC", L"Remote computer name or IP address:", WS_CHILD | WS_VISIBLE, 28, 242, 300, 24, hwnd, 0);
        gTargetEdit = AddControl(WS_EX_CLIENTEDGE, L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL, 28, 268, 620, 30, hwnd, kTargetEdit);

        gConnectRdpButton = AddControl(0, L"BUTTON", L"RDP desktop", WS_CHILD | WS_VISIBLE | WS_TABSTOP, 28, 312, 142, 34, hwnd, kConnectRdpButton);
        gConnectSshButton = AddControl(0, L"BUTTON", L"SSH terminal", WS_CHILD | WS_VISIBLE | WS_TABSTOP, 184, 312, 142, 34, hwnd, kConnectSshButton);
        gConnectSftpButton = AddControl(0, L"BUTTON", L"SFTP files", WS_CHILD | WS_VISIBLE | WS_TABSTOP, 340, 312, 142, 34, hwnd, kConnectSftpButton);
        gConnectVncButton = AddControl(0, L"BUTTON", L"VNC desktop", WS_CHILD | WS_VISIBLE | WS_TABSTOP, 496, 312, 152, 34, hwnd, kConnectVncButton);

        AddControl(0, L"BUTTON", L"Start consented session", WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_DEFPUSHBUTTON, 28, 370, 190, 42, hwnd, kStartSessionButton);
        gEndSessionButton = AddControl(0, L"BUTTON", L"End session", WS_CHILD | WS_VISIBLE | WS_TABSTOP, 232, 370, 130, 42, hwnd, kEndSessionButton);
        AddControl(0, L"BUTTON", L"Open audit log", WS_CHILD | WS_VISIBLE | WS_TABSTOP, 376, 370, 130, 42, hwnd, kOpenAuditButton);
        AddControl(0, L"BUTTON", L"About", WS_CHILD | WS_VISIBLE | WS_TABSTOP, 520, 370, 128, 42, hwnd, kAboutButton);

        AddControl(
            0,
            L"STATIC",
            L"macOS: enable Remote Login for SSH/SFTP or Screen Sharing for VNC. Ubuntu: enable SSH/SFTP, GNOME Remote Desktop/RDP, or a VNC server. A matching client/viewer must be available on this Windows PC.",
            WS_CHILD | WS_VISIBLE,
            28,
            438,
            620,
            70,
            hwnd,
            0);

        RefreshSessionUi();
        return 0;
    }
    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case kStartSessionButton:
            StartLocalSession(hwnd);
            return 0;
        case kEndSessionButton:
            EndLocalSession(hwnd, "session_ended");
            return 0;
        case kOpenAuditButton:
            OpenAuditLog(hwnd);
            return 0;
        case kAboutButton:
            ShowAbout(hwnd);
            return 0;
        case kConnectRdpButton:
            LaunchRemoteDesktop(hwnd);
            return 0;
        case kConnectSshButton:
            LaunchOpenSshClient(hwnd, L"ssh.exe", "ssh_launch_requested", "ssh_client_started", "ssh_launch_failed", L"SSH");
            return 0;
        case kConnectSftpButton:
            LaunchOpenSshClient(hwnd, L"sftp.exe", "sftp_launch_requested", "sftp_client_started", "sftp_launch_failed", L"SFTP");
            return 0;
        case kConnectVncButton:
            LaunchVnc(hwnd);
            return 0;
        default:
            break;
        }
        break;
    case WM_CLOSE:
        EndLocalSession(hwnd, "session_ended_app_exit");
        DestroyWindow(hwnd);
        return 0;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    default:
        break;
    }

    return DefWindowProcW(hwnd, message, wParam, lParam);
}
}

int WINAPI wWinMain(HINSTANCE instance, HINSTANCE, PWSTR, int showCommand)
{
    if (IsSelfTestRequested())
    {
        return RunSelfTest();
    }

    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

    WNDCLASSEXW windowClass{};
    windowClass.cbSize = sizeof(windowClass);
    windowClass.style = CS_HREDRAW | CS_VREDRAW;
    windowClass.lpfnWndProc = WindowProc;
    windowClass.hInstance = instance;
    windowClass.hIcon = LoadIconW(nullptr, IDI_APPLICATION);
    windowClass.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    windowClass.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
    windowClass.lpszClassName = kWindowClass;
    windowClass.hIconSm = LoadIconW(nullptr, IDI_APPLICATION);

    if (RegisterClassExW(&windowClass) == 0)
    {
        return 1;
    }

    gMainWindow = CreateWindowExW(
        0,
        kWindowClass,
        kAppName,
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        700,
        570,
        nullptr,
        nullptr,
        instance,
        nullptr);

    if (gMainWindow == nullptr)
    {
        return 1;
    }

    ShowWindow(gMainWindow, showCommand);
    UpdateWindow(gMainWindow);

    MSG message{};
    while (GetMessageW(&message, nullptr, 0, 0) > 0)
    {
        TranslateMessage(&message);
        DispatchMessageW(&message);
    }

    return static_cast<int>(message.wParam);
}
