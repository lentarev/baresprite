#include "AppSettings.h"

#include "framework.h"
#include <shlwapi.h>
#include <fstream>
#include <locale>

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
    GetPrivateProfileStringW(L"General",         // Section
                             L"LastProjectPath", // Key
                             L"",                // Value by default
                             buffer,             // Buffer for the result
                             MAX_PATH,           // Buffer size
                             _projectData.configPath.c_str() // Path to config.ini
    );


    _projectData.projectPath = buffer;

    return true;
}

void AppSettings::Save()
{
}

} // namespace baresprite
