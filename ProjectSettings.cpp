#include "ProjectSettings.h"
#include <Windows.h>
#include <fstream>
#include <locale>
#include <sstream>

namespace baresprite
{

ProjectSettings::ProjectSettings(Project &projectData) : _projectData(projectData)
{
}

ProjectSettings::~ProjectSettings() = default;

bool ProjectSettings::Load()
{
    return true;
}

void ProjectSettings::Save()
{
    if (_projectData.projectPath.empty())
    {
        MessageBox(nullptr, L"Path to project is empty", L"Error", MB_ICONERROR);
        return;
    }

    std::wstring fullPath = _projectData.projectPath + L"\\baresprite.ini";

    std::wofstream file(fullPath);
    file.imbue(std::locale("")); // Системная локаль

    if (!file.is_open())
    {
        MessageBox(nullptr, L"Failed to save project.", L"Error", MB_ICONERROR);
        return;
    }

    // Section [PROJECT]
    file << L"[PROJECT]\n";
    file << L"name=" << _projectData.name << L"\n";
    file << L"imageSize=" << _projectData.imageSize << L"\n";
    file << L"checkerSize=" << _projectData.checkerSize << L"\n";

    // Section [PALETTE]
    file << L"\n[PALETTE]\n";
    for (int i = 0; i < 25; ++i)
    {
        wchar_t key[25];
        swprintf_s(key, L"Color%d", i);

        wchar_t val[25];
        swprintf_s(val, L"%06X", _projectData.palette.colors[i] & 0xFFFFFF);
        file << key << L"=" << val << L"\n";
    }

    // Секция [PALETTE_LAST_INDEX]
    file << L"\n[PALETTE_LAST_INDEX]\n";
    wchar_t idxStr[8];
    swprintf_s(idxStr, L"%d", _projectData.palette.index);
    file << L"SelectedColorIndex=" << idxStr << L"\n";

    file.close();
}

} // namespace baresprite