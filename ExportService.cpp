#define _CRT_SECURE_NO_WARNINGS

#include "ExportService.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include <Windows.h>
#include <filesystem>
#include <iostream>
#include <vector>

namespace fs = std::filesystem;

namespace baresprite
{

/// <summary>
/// Exporting a single frame to PNG
/// </summary>
/// <param name="frame"></param>
/// <param name="filePath"></param>
/// <returns></returns>
bool ExportService::ExportFrameToPNG(const Frame &frame, const std::wstring &filePath)
{
    if (frame.width == 0 || frame.height == 0)
        return false;

    // Конвертируем пиксели из 0xAARRGGBB → RGBA байты (stbi требует именно такой порядок)
    std::vector<unsigned char> rgbaBuffer(frame.width * frame.height * 4);

    for (int y = 0; y < frame.height; ++y)
    {
        for (int x = 0; x < frame.width; ++x)
        {
            uint32_t pixel = frame.GetPixel(x, y);

            // Извлекаем компоненты из 0xAARRGGBB
            unsigned char a = (pixel >> 24) & 0xFF;
            unsigned char r = (pixel >> 16) & 0xFF;
            unsigned char g = (pixel >> 8) & 0xFF;
            unsigned char b = pixel & 0xFF;

            // Записываем в буфер в порядке RGBA
            int idx = (y * frame.width + x) * 4;
            rgbaBuffer[idx + 0] = r;
            rgbaBuffer[idx + 1] = g;
            rgbaBuffer[idx + 2] = b;
            rgbaBuffer[idx + 3] = a;
        }
    }

    // Конвертируем путь в UTF-8 для stbi (он не поддерживает wchar_t напрямую)
    int size = WideCharToMultiByte(CP_UTF8, 0, filePath.c_str(), -1, nullptr, 0, nullptr, nullptr);
    std::vector<char> utf8Path(size);
    WideCharToMultiByte(CP_UTF8, 0, filePath.c_str(), -1, utf8Path.data(), size, nullptr, nullptr);

    // Записываем PNG
    // Параметры: путь, ширина, высота, каналов (4=RGBA), данные, stride (0=по умолчанию)
    int result = stbi_write_png(utf8Path.data(), frame.width, frame.height, 4, rgbaBuffer.data(), 0);

    return result != 0;
}

/// <summary>
/// Export sequence
/// </summary>
/// <param name="frames"></param>
/// <param name="tagFilter"></param>
/// <param name="outputFolder"></param>
/// <param name="outMessage"></param>
/// <returns></returns>
bool ExportService::ExportSequence(const std::vector<Frame> &frames, const std::wstring &tagFilter, const std::wstring &outputFolder, std::wstring &outMessage)
{
    if (frames.empty() || outputFolder.empty())
    {
        outMessage = L"Invalid parameters.";
        return false;
    }

    if (!fs::exists(outputFolder) || !fs::is_directory(outputFolder))
    {
        outMessage = L"Output folder does not exist.";
        return false;
    }

    int exportIndex = 0;
    int successCount = 0;

#ifdef _DEBUG
    for (const auto &f : frames)
    {
        std::wcout << L"Frame tag: [" << f.tag << L"]\n";
    }
    std::wcout << L"Filter tag: [" << tagFilter << L"]\n";
#endif

    for (const auto &frame : frames)
    {
        std::wstring frameTag = frame.tag.empty() ? L"None" : frame.tag;
        std::wstring filterTag = tagFilter.empty() ? L"None" : tagFilter;

        // Пропускаем кадр, только если фильтр не "All" и теги не совпадают
        if (filterTag != L"All" && frameTag != filterTag)
        {
            continue;
        }

        wchar_t filename[MAX_PATH];

        // Используем имя выбранного тега как префикс (Idle, Walk, All и т.д.)
        std::wstring tagPrefix = tagFilter;

        // На всякий случай, если фильтр пустой (хотя логика диалога это предотвращает)
        if (tagPrefix.empty())
            tagPrefix = L"Frame";

        // Формат: {Tag}_frame_{Index}.png
        // Примеры: Idle_frame_000.png, Walk_frame_000.png, All_frame_000.png
        swprintf_s(filename, L"%s_frame_%03d.png", tagPrefix.c_str(), exportIndex);

        std::wstring fullPath = outputFolder + L"\\" + filename;

        if (ExportFrameToPNG(frame, fullPath))
        {
            successCount++;
        }

        exportIndex++;
    }

    if (successCount > 0)
    {
        outMessage = L"Successfully exported " + std::to_wstring(successCount) + L" frame(s).";
        return true;
    }
    else
    {
        outMessage = L"No frames matched the selected tag.";
        return false;
    }
}

} // namespace baresprite