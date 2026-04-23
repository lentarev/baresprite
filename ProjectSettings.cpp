#include "ProjectSettings.h"
#include <Windows.h>
#include <cctype>
#include <fstream>
#include <locale>
#include <sstream>

namespace baresprite
{

ProjectSettings::ProjectSettings(AppState &appState) : _appState(appState)
{
}

ProjectSettings::~ProjectSettings() = default;

/// <summary>
/// Load
/// </summary>
/// <returns></returns>
bool ProjectSettings::Load()
{
    if (_appState.projectPath.empty())
    {
        MessageBox(nullptr, L"Project path is empty", L"Error", MB_ICONERROR);
        return false;
    }

    std::wstring fullPath = _appState.projectPath + L"\\" + _configProjectFileName;

    std::wifstream file(fullPath);
    file.imbue(std::locale(""));

    if (!file.is_open())
    {
        MessageBox(nullptr, L"Failed to load project config.", L"Error", MB_ICONERROR);
        return false;
    }

    std::wstring line;
    std::wstring currentSection;

    while (std::getline(file, line))
    {
        line = Trim(line);

        // Пропускаем пустые строки и комментарии
        if (line.empty() || line[0] == L';' || line[0] == L'#')
        {
            continue;
        }

        // Определяем секцию
        if (line[0] == L'[')
        {
            size_t end = line.find(L']');
            if (end != std::wstring::npos)
            {
                currentSection = Trim(line.substr(1, end - 1));
            }
            continue;
        }

        // Парсим ключ=значение
        size_t eqPos = line.find(L'=');
        if (eqPos == std::wstring::npos)
        {
            continue; // Пропускаем битые строки
        }

        std::wstring key = Trim(line.substr(0, eqPos));
        std::wstring value = Trim(line.substr(eqPos + 1));

        try
        {
            // === СЕКЦИЯ [PROJECT] ===
            if (currentSection == L"PROJECT")
            {
                if (key == L"name")
                {
                    _appState.name = value;
                }
                else if (key == L"imageSize")
                {
                    _appState.imageSize = std::stoi(value);
                }
                else if (key == L"checkerSize")
                {
                    _appState.checkerSize = std::stoi(value);
                }
            }
            // === СЕКЦИЯ [PALETTE] ===
            else if (currentSection == L"PALETTE")
            {
                if (key.find(L"Color") == 0)
                {
                    // Извлекаем индекс из "Color0", "Color1", ...
                    int idx = std::stoi(key.substr(5));

                    if (idx >= 0 && idx < 25)
                    {
                        // Парсим HEX (RRGGBB) → unsigned long
                        unsigned long hex = std::stoul(value, nullptr, 16);

                        // Извлекаем компоненты: 0xRRGGBB → R, G, B
                        unsigned char r = (hex >> 16) & 0xFF;
                        unsigned char g = (hex >> 8) & 0xFF;
                        unsigned char b = hex & 0xFF;

                        // COLORREF хранится как 0x00BBGGRR → используем макрос RGB()
                        _appState.palette.colors[idx] = RGB(r, g, b);
                    }
                }
            }
            // === СЕКЦИЯ [PALETTE_LAST_INDEX] ===
            else if (currentSection == L"PALETTE_LAST_INDEX")
            {
                if (key == L"SelectedColorIndex")
                {
                    _appState.palette.index = std::stoi(value);
                }
            }
        }
        catch (...)
        {

            // Можно добавить логирование для отладки

            return false;
        }
    }

    file.close();
    return true;
}

/// <summary>
/// Save
/// </summary>
void ProjectSettings::Save()
{
    if (_appState.projectPath.empty())
    {
        MessageBox(nullptr, L"Project path is empty", L"Error", MB_ICONERROR);
        return;
    }

    std::wstring fullPath = _appState.projectPath + L"\\" + _configProjectFileName;

    std::wofstream file(fullPath);
    file.imbue(std::locale("")); // Системная локаль

    if (!file.is_open())
    {
        MessageBox(nullptr, L"Failed to save project.", L"Error", MB_ICONERROR);
        return;
    }

    // Section [PROJECT]
    file << L"[PROJECT]\n";
    file << L"name=" << _appState.name << L"\n";
    file << L"imageSize=" << _appState.imageSize << L"\n";
    file << L"checkerSize=" << _appState.checkerSize << L"\n";

    if (!_appState.palette.colors.empty())
    {
        // Section [PALETTE]
        file << L"\n[PALETTE]\n";
        for (int i = 0; i < 25; ++i)
        {
            if (i < static_cast<int>(_appState.palette.colors.size()))
            {
                COLORREF c = _appState.palette.colors[i];

                // Extract the components in the correct order
                unsigned char r = GetRValue(c);
                unsigned char g = GetGValue(c);
                unsigned char b = GetBValue(c);

                wchar_t key[25], val[25];
                swprintf_s(key, L"Color%d", i);
                swprintf_s(val, L"%02X%02X%02X", r, g, b); // RRGGBB

                file << key << L"=" << val << L"\n";
            }
        }

        // Секция [PALETTE_LAST_INDEX]
        file << L"\n[PALETTE_LAST_INDEX]\n";
        wchar_t idxStr[8];
        swprintf_s(idxStr, L"%d", _appState.palette.index);
        file << L"SelectedColorIndex=" << idxStr << L"\n";
    }

    file.close();
}

} // namespace baresprite