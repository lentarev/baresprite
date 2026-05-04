#define _CRT_SECURE_NO_WARNINGS

#include "ExportService.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include <vector>
#include <Windows.h>

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

} // namespace baresprite