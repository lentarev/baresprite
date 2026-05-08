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

        // Skipping blank lines and comments
        if (line.empty() || line[0] == L';' || line[0] == L'#')
        {
            continue;
        }

        // Defining a section
        if (line[0] == L'[')
        {
            size_t end = line.find(L']');
            if (end != std::wstring::npos)
            {
                currentSection = Trim(line.substr(1, end - 1));
            }
            continue;
        }

        // Parse key=value
        size_t eqPos = line.find(L'=');
        if (eqPos == std::wstring::npos)
        {
            continue; // Skipping broken lines
        }

        std::wstring key = Trim(line.substr(0, eqPos));
        std::wstring value = Trim(line.substr(eqPos + 1));

        try
        {
            // === SECTION [PROJECT] ===
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
                else if (key == L"currentFrameIndex")
                {
                    _appState.currentFrameIndex = std::stoi(value);
                }
            }
            // === SECTION [PALETTE] ===
            else if (currentSection == L"PALETTE")
            {
                if (key.find(L"Color") == 0)
                {
                    // Extract the index fromз "Color0", "Color1", ...
                    int idx = std::stoi(key.substr(5));

                    if (idx >= 0 && idx < 25)
                    {
                        // Parse HEX (RRGGBB) -> unsigned long
                        unsigned long hex = std::stoul(value, nullptr, 16);

                        // Extracting the components: 0xRRGGBB → R, G, B
                        unsigned char r = (hex >> 16) & 0xFF;
                        unsigned char g = (hex >> 8) & 0xFF;
                        unsigned char b = hex & 0xFF;

                        // COLORREF is stored as 0x00BBGGRR → use the RGB() macro
                        _appState.palette.colors[idx] = RGB(r, g, b);
                    }
                }
            }
            // === SECTION [PALETTE_LAST_INDEX] ===
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
            // === SECTION [ONION_SKIN] ===
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

            // === SECTION [ANIMATION] ===
            else if (currentSection == L"ANIMATION")
            {
                if (key == L"playbackFPS")
                {
                    _appState.playbackFPS = std::stoi(value);
                }
            }

            // === SECTION [TAGS] ===
            else if (currentSection == L"TAGS")
            {
                if (key.find(L"Tag") == 0) // Tag0, Tag1, ...
                {
                    _appState.availableTags.push_back(value);
                }
            }

            // === SECTION [FRAMES] ===
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
            // === SECTION [FRAME_0], [FRAME_1], ... ===
            else if (currentSection.find(L"FRAME_") == 0)
            {
                // Extract the frame index from "FRAME_0"
                int frameIdx = std::stoi(currentSection.substr(6));

                // We guarantee that the vector is large enough
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
                    // Reallocate the pixel buffer when resizing
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
                    // Decoding \n and \r back
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
                    // Parse the HEX string back into pixels
                    size_t pixelCount = static_cast<size_t>(frame.width) * frame.height;
                    if (value.length() >= pixelCount * 8)
                    { // 8 characters per pixel
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

            // Logging can be added for debugging

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
    file.imbue(std::locale("")); // System locale

    if (!file.is_open())
    {
        MessageBox(nullptr, L"Failed to save project.", L"Error", MB_ICONERROR);
        return;
    }

    // SECTION [PROJECT]
    file << L"[PROJECT]\n";
    file << L"name=" << _appState.name << L"\n";
    file << L"imageSize=" << _appState.imageSize << L"\n";
    file << L"checkerSize=" << _appState.checkerSize << L"\n";
    file << L"filterTag=" << _appState.currentFilterTag << L"\n";
    file << L"currentFrameIndex=" << _appState.currentFrameIndex << L"\n";

    if (!_appState.palette.colors.empty())
    {
        // SECTION [PALETTE]
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

        // SECTION [PALETTE_LAST_INDEX]
        file << L"\n[PALETTE_LAST_INDEX]\n";
        wchar_t idxStr[8];
        swprintf_s(idxStr, L"%d", _appState.palette.index);
        file << L"SelectedColorIndex=" << idxStr << L"\n";
    }

    // SECTION [ONION_SKIN]
    file << L"\n[ONION_SKIN]\n";
    file << L"onionSkinEnabled=" << _appState.onionSkinEnabled << L"\n";
    file << L"onionSkinPrevFrames=" << _appState.onionSkinPrevFrames << L"\n";
    file << L"onionSkinNextFrames=" << _appState.onionSkinNextFrames << L"\n";
    file << L"onionSkinOpacity=" << static_cast<int>(_appState.onionSkinOpacity * 100) << L"\n";

    // SECTION [ANIMATION]
    file << L"\n[ANIMATION]\n";
    file << L"playbackFPS=" << _appState.playbackFPS << L"\n";

    // === SECTION [TAGS] ===
    if (!_appState.availableTags.empty())
    {
        file << L"\n[TAGS]\n";
        file << L"Count=" << _appState.availableTags.size() << L"\n";

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

        // Escape tags (replace \n and \r with \\n and \\r)
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

        // Pixels in HEX (format 0xAARRGGBB → 8 characters per pixel)
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