#include <windows.h>
#include <bcrypt.h>
#include <shellapi.h>
#include <shlobj.h>

#include <array>
#include <cstdio>
#include <string>

namespace
{
constexpr wchar_t kAppName[] = L"Remote Support";

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
    directory += L"\\RemoteSupport";

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
    return ok && written == record.size();
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
    return first.size() == 32 && second.size() == 32 && first != second && validCharacters ? 0 : 1;
}
}

int WINAPI wWinMain(HINSTANCE, HINSTANCE, PWSTR, int)
{
    if (IsSelfTestRequested())
    {
        return RunSelfTest();
    }

    const std::string sessionId = GenerateSessionId();
    const std::wstring auditPath = AuditLogPath();
    if (sessionId.empty() || auditPath.empty())
    {
        MessageBoxW(nullptr, L"The secure local session could not be initialised.", kAppName, MB_OK | MB_ICONERROR);
        return 1;
    }

    const std::wstring sessionWide(sessionId.begin(), sessionId.end());
    const std::wstring prompt =
        L"An authorised support session is requesting access.\n\nSession: " + sessionWide +
        L"\n\nNo remote-control capability is enabled in this redesign baseline. "
        L"Select Yes to record explicit consent and initialise the local session, or No to deny it.";

    const int choice = MessageBoxW(
        nullptr,
        prompt.c_str(),
        kAppName,
        MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2 | MB_SETFOREGROUND);

    if (choice != IDYES)
    {
        WriteAudit(auditPath, sessionId, "consent_denied");
        return 0;
    }

    if (!WriteAudit(auditPath, sessionId, "consent_granted"))
    {
        MessageBoxW(nullptr, L"Consent was granted, but the audit record could not be written. The session will not continue.", kAppName, MB_OK | MB_ICONERROR);
        return 1;
    }

    MessageBoxW(
        nullptr,
        L"Consent recorded. This native x64 baseline intentionally leaves network transport and privileged remote actions disabled until mutual authentication, encrypted transport, per-capability consent, expiry and auditing are implemented.",
        kAppName,
        MB_OK | MB_ICONINFORMATION);

    return 0;
}
