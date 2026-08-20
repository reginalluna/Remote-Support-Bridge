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
constexpr int kRdpTargetEdit = 1005;
constexpr int kConnectRdpButton = 1006;

HWND gMainWindow = nullptr;
HWND gStatusText = nullptr;
HWND gSessionText = nullptr;
HWND gEndSessionButton = nullptr;
HWND gRdpTargetEdit = nullptr;
HWND gConnectRdpButton = nullptr;
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

bool IsSafeRdpTarget(const std::wstring& target)
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
    EnableWindow(gEndSessionButton, gSessionId.empty() ? FALSE : TRUE);
    EnableWindow(gConnectRdpButton, gSessionId.empty() ? FALSE : TRUE);
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
        L"\n\nAfter consent, this application can hand off a connection to the built-in Windows Remote Desktop client. Windows remains responsible for remote authentication, authorisation and the RDP connection.";

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

void LaunchRemoteDesktop(HWND owner)
{
    if (gSessionId.empty())
    {
        MessageBoxW(owner, L"Start and approve a support session before opening Remote Desktop.", kAppName, MB_OK | MB_ICONINFORMATION);
        return;
    }

    const std::wstring target = ReadControlText(gRdpTargetEdit);
    if (!IsSafeRdpTarget(target))
    {
        MessageBoxW(owner, L"Enter a valid computer name or IP address. Only letters, numbers, dots, hyphens, colons and IPv6 brackets are accepted.", kAppName, MB_OK | MB_ICONWARNING);
        return;
    }

    if (gAuditPath.empty() || !WriteAudit(gAuditPath, gSessionId, "rdp_launch_requested"))
    {
        MessageBoxW(owner, L"The Remote Desktop hand-off was blocked because its audit event could not be recorded.", kAppName, MB_OK | MB_ICONERROR);
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
        L"\n\nConsent-first Windows support application. Remote screen, keyboard and mouse operation is provided by the authenticated Windows Remote Desktop subsystem rather than a custom remote-control protocol.";
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
    const bool validRdpTarget = IsSafeRdpTarget(L"lab-pc-02:3389") && IsSafeRdpTarget(L"[2001:db8::10]") && !IsSafeRdpTarget(L"/admin");
    return first.size() == 32 && second.size() == 32 && first != second && validCharacters && validArchitecture && validRdpTarget ? 0 : 1;
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
            L"Consent-first Windows support interface with an audited hand-off to Windows Remote Desktop.",
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
        gRdpTargetEdit = AddControl(WS_EX_CLIENTEDGE, L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL, 28, 268, 396, 30, hwnd, kRdpTargetEdit);
        gConnectRdpButton = AddControl(0, L"BUTTON", L"Connect with Windows RDP", WS_CHILD | WS_VISIBLE | WS_TABSTOP, 440, 268, 208, 30, hwnd, kConnectRdpButton);

        AddControl(0, L"BUTTON", L"Start consented session", WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_DEFPUSHBUTTON, 28, 326, 190, 42, hwnd, kStartSessionButton);
        gEndSessionButton = AddControl(0, L"BUTTON", L"End session", WS_CHILD | WS_VISIBLE | WS_TABSTOP, 232, 326, 130, 42, hwnd, kEndSessionButton);
        AddControl(0, L"BUTTON", L"Open audit log", WS_CHILD | WS_VISIBLE | WS_TABSTOP, 376, 326, 130, 42, hwnd, kOpenAuditButton);
        AddControl(0, L"BUTTON", L"About", WS_CHILD | WS_VISIBLE | WS_TABSTOP, 520, 326, 128, 42, hwnd, kAboutButton);

        AddControl(
            0,
            L"STATIC",
            L"Windows RDP provides the network connection, screen, keyboard/mouse and Windows authentication. This application does not install a custom listener or background control service.",
            WS_CHILD | WS_VISIBLE,
            28,
            392,
            620,
            54,
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
        520,
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
