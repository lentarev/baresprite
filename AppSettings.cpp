#include "AppSettings.h"

#include "framework.h"

#include <fstream>
#include <locale>
#include <shlwapi.h>

namespace baresprite
{
AppSettings::AppSettings(Project &projectData) : _projectData(projectData)
{

    // Get App path
    wchar_t exePath[MAX_PATH];
    GetModuleFileNameW(nullptr, exePath, MAX_PATH);
    PathRemoveFileSpecW(exePath);

    // Forming the path to config.ini
    _projectData.configPath = std::wstring(exePath) + L"\\config.ini";
}

AppSettings::~AppSettings() = default;

/// <summary>
/// Load
/// </summary>
/// <returns></returns>
bool AppSettings::Load()
{
    // Check is config path not empty
    if (_projectData.configPath.empty())
        return false;

    // Reading config
    std::wifstream file(_projectData.configPath);
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
                             _projectData.configPath.c_str() // Path to config.ini
    );

    _projectData.projectPath = buffer;
    _projectData.name = GetProjectNameFromPath(buffer);

    return true;
}

/// <summary>
/// Save
/// </summary>
void AppSettings::Save()
{
    WritePrivateProfileStringW(L"General", L"LastProjectPath", _projectData.projectPath.c_str(), _projectData.configPath.c_str());

    _projectData.name = GetProjectNameFromPath(_projectData.projectPath.c_str());
}

} // namespace baresprite
