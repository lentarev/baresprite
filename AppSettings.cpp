#include "AppSettings.h"

#include "framework.h"

#include <fstream>
#include <locale>
#include <shlwapi.h>

namespace baresprite
{
AppSettings::AppSettings(AppState &appState) : _appState(appState)
{

    // Get App path
    wchar_t exePath[MAX_PATH];
    GetModuleFileNameW(nullptr, exePath, MAX_PATH);
    PathRemoveFileSpecW(exePath);

    // Forming the path to config.ini
    _appState.configPath = std::wstring(exePath) + L"\\config.ini";
}

AppSettings::~AppSettings() = default;

/// <summary>
/// Load
/// </summary>
/// <returns></returns>
bool AppSettings::Load()
{
    // Check is config path not empty
    if (_appState.configPath.empty())
        return false;

    // Reading config
    std::wifstream file(_appState.configPath);
    file.imbue(std::locale("")); // Cyrillic support

    // Check is exist app config file
    if (!file.is_open())
        return false;

    wchar_t buffer[MAX_PATH];

    // We read the final path to the project.
    GetPrivateProfileStringW(L"General",                     // Section
                             L"LastProjectPath",             // Key
                             L"",                            // Value by default
                             buffer,                         // Buffer for the result
                             MAX_PATH,                       // Buffer size
                             _appState.configPath.c_str() // Path to config.ini
    );

    _appState.projectPath = buffer;
    _appState.name = GetProjectNameFromPath(buffer);

    return true;
}

/// <summary>
/// Save
/// </summary>
void AppSettings::Save()
{
    WritePrivateProfileStringW(L"General", L"LastProjectPath", _appState.projectPath.c_str(), _appState.configPath.c_str());

    _appState.name = GetProjectNameFromPath(_appState.projectPath.c_str());
}

/// <summary>
/// Check that the file exists and it is not a folder
/// </summary>
/// <param name="fullPath"></param>
/// <returns></returns>
bool AppSettings::IsProjectExist(const std::wstring &projectPath)
{
    std::wstring fullPath = projectPath + L"\\" + _configProjectFileName;

    DWORD attribs = GetFileAttributesW(fullPath.c_str());

    // We check that the file exists and it is not a folder.
    return (attribs != INVALID_FILE_ATTRIBUTES) && !(attribs & FILE_ATTRIBUTE_DIRECTORY);
}

} // namespace baresprite
