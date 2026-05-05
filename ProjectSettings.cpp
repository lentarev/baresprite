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
                else if (key == L"filterTag")
                {
                    _appState.currentFilterTag = value;
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

                    if (_appState.palette.index >= 0 && _appState.palette.index < static_cast<int>(_appState.palette.colors.size()))
                    {
                        _appState.palette.color = _appState.palette.colors[_appState.palette.index];
                    }
                }
            }
            // === СЕКЦИЯ [ONION_SKIN] ===
            else if (currentSection == L"ONION_SKIN")
            {
                if (key == L"onionSkinEnabled")
                {
                    _appState.onionSkinEnabled = (std::stoi(value) != 0);
                }
                else if (key == L"onionSkinPrevFrames")
                {
                    _appState.onionSkinPrevFrames = std::stoi(value);
                }
                else if (key == L"onionSkinNextFrames")
                {
                    _appState.onionSkinNextFrames = std::stoi(value);
                }
                else if (key == L"onionSkinOpacity")
                {

                    try
                    {
                        int percent = std::stoi(value);
                        _appState.onionSkinOpacity = percent / 100.0f;
                        _appState.onionSkinOpacity = std::clamp(_appState.onionSkinOpacity, 0.0f, 1.0f);
                    }
                    catch (...)
                    {
                        _appState.onionSkinOpacity = 0.35f;
                    }
                }
            }

            // === СЕКЦИЯ [ANIMATION] ===
            else if (currentSection == L"ANIMATION")
            {
                if (key == L"playbackFPS")
                {
                    _appState.playbackFPS = std::stoi(value);
                }
            }

            // === СЕКЦИЯ [TAGS] ===
            else if (currentSection == L"TAGS")
            {
                if (key.find(L"Tag") == 0) // Tag0, Tag1, ...
                {
                    _appState.availableTags.push_back(value);
                }
            }

            // === СЕКЦИЯ [FRAMES] ===
            else if (currentSection == L"FRAMES")
            {
                if (key == L"Count")
                {
                    int frameCount = std::stoi(value);
                    _appState.frames.clear();
                    _appState.frames.reserve(frameCount);
                    _appState.currentFrameIndex = 0;
                }
            }
            // === СЕКЦИИ [FRAME_0], [FRAME_1], ... ===
            else if (currentSection.find(L"FRAME_") == 0)
            {
                // Извлекаем индекс кадра из "FRAME_0"
                int frameIdx = std::stoi(currentSection.substr(6));

                // Гарантируем, что вектор достаточно большой
                if (frameIdx >= static_cast<int>(_appState.frames.size()))
                {
                    _appState.frames.resize(frameIdx + 1);
                }

                Frame &frame = _appState.frames[frameIdx];

                if (key == L"width")
                {
                    frame.width = std::stoi(value);
                }
                else if (key == L"height")
                {
                    frame.height = std::stoi(value);
                    // Перевыделяем буфер пикселей при изменении размера
                    if (frame.pixels || (frame.width * frame.height > 0))
                    {
                        frame.pixels = std::make_unique<uint32_t[]>(static_cast<size_t>(frame.width) * frame.height);
                    }
                }
                else if (key == L"duration")
                {
                    frame.duration = std::stoi(value);
                }
                else if (key == L"isVisible")
                {
                    frame.isVisible = (value == L"1");
                }
                else if (key == L"tag")
                {
                    // Декодируем \n и \r обратно
                    std::wstring tag = value;
                    size_t pos = 0;
                    while ((pos = tag.find(L"\\n", pos)) != std::wstring::npos)
                    {
                        tag.replace(pos, 2, L"\n");
                        pos += 1;
                    }
                    pos = 0;
                    while ((pos = tag.find(L"\\r", pos)) != std::wstring::npos)
                    {
                        tag.replace(pos, 2, L"\r");
                        pos += 1;
                    }
                    frame.tag = tag;
                }
                else if (key == L"pixels" && frame.pixels)
                {
                    // Парсим HEX-строку обратно в пиксели
                    size_t pixelCount = static_cast<size_t>(frame.width) * frame.height;
                    if (value.length() >= pixelCount * 8)
                    { // 8 символов на пиксель
                        for (size_t p = 0; p < pixelCount; ++p)
                        {
                            std::wstring hexStr = value.substr(p * 8, 8);
                            frame.pixels[p] = static_cast<uint32_t>(std::wcstoul(hexStr.c_str(), nullptr, 16));
                        }
                    }
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
    file << L"filterTag=" << _appState.currentFilterTag << L"\n";

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

    // Section [ONION_SKIN]
    file << L"\n[ONION_SKIN]\n";
    file << L"onionSkinEnabled=" << _appState.onionSkinEnabled << L"\n";
    file << L"onionSkinPrevFrames=" << _appState.onionSkinPrevFrames << L"\n";
    file << L"onionSkinNextFrames=" << _appState.onionSkinNextFrames << L"\n";
    file << L"onionSkinOpacity=" << static_cast<int>(_appState.onionSkinOpacity * 100) << L"\n";

    // Section [ANIMATION]
    file << L"\n[ANIMATION]\n";
    file << L"playbackFPS=" << _appState.playbackFPS << L"\n";

    // === СЕКЦИЯ [TAGS] ===
    if (!_appState.availableTags.empty())
    {
        file << L"\n[TAGS]\n";
        file << L"Count=" << _appState.availableTags.size() << L"\n"; // Опционально, для надёжности

        for (size_t i = 0; i < _appState.availableTags.size(); ++i)
        {
            wchar_t key[32];
            swprintf_s(key, L"Tag%zu", i); // Tag0, Tag1, Tag2...
            file << key << L"=" << _appState.availableTags[i] << L"\n";
        }
    }

    for (size_t i = 0; i < _appState.frames.size(); ++i)
    {
        const Frame &frame = _appState.frames[i];

        file << L"\n[FRAME_" << i << L"]\n";
        file << L"width=" << frame.width << L"\n";
        file << L"height=" << frame.height << L"\n";
        file << L"duration=" << frame.duration << L"\n";
        file << L"isVisible=" << (frame.isVisible ? 1 : 0) << L"\n";

        // Экранируем теги (заменяем \n и \r на \\n и \\r)
        std::wstring safeTag = frame.tag;
        size_t pos = 0;
        while ((pos = safeTag.find(L'\n', pos)) != std::wstring::npos)
        {
            safeTag.replace(pos, 1, L"\\n");
            pos += 2;
        }
        pos = 0;
        while ((pos = safeTag.find(L'\r', pos)) != std::wstring::npos)
        {
            safeTag.replace(pos, 1, L"\\r");
            pos += 2;
        }
        file << L"tag=" << safeTag << L"\n";

        // Пиксели в HEX (формат 0xAARRGGBB → 8 символов на пиксель)
        file << L"pixels=";
        size_t pixelCount = static_cast<size_t>(frame.width) * frame.height;
        for (size_t p = 0; p < pixelCount; ++p)
        {
            wchar_t hex[9];
            swprintf_s(hex, L"%08X", frame.pixels[p]); // AARRGGBB
            file << hex;
        }
        file << L"\n";
    }

    file.close();
}

} // namespace baresprite