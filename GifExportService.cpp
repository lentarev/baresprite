#define GIF_IMPLEMENTATION
#pragma warning(push)
#pragma warning(disable : 4334)
#include "gif.h"
#pragma warning(pop)

#include "GifExportService.h"

namespace baresprite
{
bool GifExportService::ExportGif(const ExportSequenceData &exportData)
{
    // Фильтрация кадров по тегу
    std::vector<const Frame *> filteredFrames;

    for (const auto &frame : exportData.appState->frames)
    {
        std::wstring frameTag = frame.tag.empty() ? L"None" : frame.tag;
        std::wstring filterTag = exportData.selectedTag.empty() ? L"None" : exportData.selectedTag;

        if (filterTag == L"All" || frameTag == filterTag)
        {
            filteredFrames.push_back(&frame);
        }
    }

    if (filteredFrames.empty())
    {
        return false;
    }

    // Параметры
    int frameW = exportData.appState->imageSize * exportData.scale;
    int frameH = exportData.appState->imageSize * exportData.scale;
    int count = static_cast<int>(filteredFrames.size());

    // Создаём GIF
    std::wstring gifPath = exportData.outputFolder + L"\\" + exportData.fileName + L".gif";

    // Конвертируем путь в UTF-8 для gif.h
    int size = WideCharToMultiByte(CP_UTF8, 0, gifPath.c_str(), -1, nullptr, 0, nullptr, nullptr);
    std::vector<char> utf8Path(size);
    WideCharToMultiByte(CP_UTF8, 0, gifPath.c_str(), -1, utf8Path.data(), size, nullptr, nullptr);

    GifWriter writer = {};

    // Длительность по умолчанию (из первого кадра)
    int defaultDuration = 100 / exportData.appState->playbackFPS;
    
    if (defaultDuration < 1)
    {
        defaultDuration = 1;
    }
    

    if (!GifBegin(&writer, utf8Path.data(), frameW, frameH, defaultDuration, exportData.infiniteLoop))
    {
        return false;
    }

    // Пишем каждый кадр
    for (const auto *frame : filteredFrames)
    {
        // Создаём буфер для масштабированного кадра (RGBA)
        std::vector<uint8_t> rgbaBuffer(frameW * frameH * 4);

        if (exportData.scale == 1)
        {
            // Без масштабирования — просто копируем
            for (int y = 0; y < exportData.appState->imageSize; ++y)
            {
                for (int x = 0; x < exportData.appState->imageSize; ++x)
                {
                    uint32_t pixel = frame->GetPixel(x, y);

                    unsigned char a = (pixel >> 24) & 0xFF;
                    unsigned char r = (pixel >> 16) & 0xFF;
                    unsigned char g = (pixel >> 8) & 0xFF;
                    unsigned char b = pixel & 0xFF;

                    int idx = (y * exportData.appState->imageSize + x) * 4;
                    rgbaBuffer[idx + 0] = r;
                    rgbaBuffer[idx + 1] = g;
                    rgbaBuffer[idx + 2] = b;
                    rgbaBuffer[idx + 3] = a;
                }
            }
        }
        else
        {
            // Масштабирование (nearest neighbor)
            for (int sy = 0; sy < frameH; ++sy)
            {
                for (int sx = 0; sx < frameW; ++sx)
                {
                    // Координаты в исходном кадре
                    int srcX = sx / exportData.scale;
                    int srcY = sy / exportData.scale;

                    uint32_t pixel = frame->GetPixel(srcX, srcY);

                    unsigned char a = (pixel >> 24) & 0xFF;
                    unsigned char r = (pixel >> 16) & 0xFF;
                    unsigned char g = (pixel >> 8) & 0xFF;
                    unsigned char b = pixel & 0xFF;

                    int idx = (sy * frameW + sx) * 4;
                    rgbaBuffer[idx + 0] = r;
                    rgbaBuffer[idx + 1] = g;
                    rgbaBuffer[idx + 2] = b;
                    rgbaBuffer[idx + 3] = a;
                }
            }
        }

        // Длительность кадра в deciseconds (единая для всех кадров)
        int duration = 100 / exportData.appState->playbackFPS;
        if (duration < 1)
            duration = 1; // Минимум 1 decisecond (100ms)

        // Записываем кадр
        GifWriteFrame(&writer, rgbaBuffer.data(), frameW, frameH, duration);
    }

    GifEnd(&writer);

    return true;
}

} // namespace baresprite