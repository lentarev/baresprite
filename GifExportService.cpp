#define NOMINMAX

#include "GifExportService.h"
#include <algorithm>
#include <gif_lib.h>
#include <map>
#include <vector>

namespace baresprite
{

// Distance between colors (to find the closest one in the palette)
inline int ColorDist(uint32_t c1, uint32_t c2)
{
    int r1 = (c1 >> 16) & 0xFF, g1 = (c1 >> 8) & 0xFF, b1 = c1 & 0xFF;
    int r2 = (c2 >> 16) & 0xFF, g2 = (c2 >> 8) & 0xFF, b2 = c2 & 0xFF;
    return (r1 - r2) * (r1 - r2) + (g1 - g2) * (g1 - g2) + (b1 - b2) * (b1 - b2);
}

// Building a global palette (256 colors, index 0 = transparent)
struct PaletteBuilder
{
    std::vector<uint32_t> palette;
    std::vector<int> lut; // Quick mapping: RGB -> index

    void Build(const std::vector<const Frame *> &frames, int w, int h, int scale)
    {
        std::map<uint32_t, int> freq;
        for (const auto *frame : frames)
        {
            for (int y = 0; y < h; ++y)
            {
                for (int x = 0; x < w; ++x)
                {
                    int srcX = x / scale, srcY = y / scale;
                    uint32_t px = frame->GetPixel(srcX, srcY);

                    // We only count opaque pixels
                    if (((px >> 24) & 0xFF) >= 128)
                    {

                        ++freq[px & 0x00FFFFFF];
                    }
                }
            }
        }

        // Sort by frequency
        std::vector<std::pair<uint32_t, int>> sorted(freq.begin(), freq.end());
        std::sort(sorted.begin(), sorted.end(), [](auto &a, auto &b) { return a.second > b.second; });

        // We form a palette: index 0 = transparent, then up to 255 colors
        palette.reserve(256);
        palette.push_back(0); // Прозрачный
        for (size_t i = 0; i < sorted.size() && palette.size() < 256; ++i)
        {
            palette.push_back(sorted[i].first | 0xFF000000);
        }

        // We'll pad to 256 (giflib requires)
        while (palette.size() < 256)
        {
            palette.push_back(0x00000000);
        }

        // Building a LUT for fast mapping
        lut.assign(1 << 24, -1);

        for (size_t i = 0; i < palette.size(); ++i)
        {
            lut[palette[i] & 0x00FFFFFF] = static_cast<int>(i);
        }
    }

    int MapPixel(uint32_t pixel) const
    {
        // Transparent pixel
        if (((pixel >> 24) & 0xFF) < 128)
        {
            return 0;
        }

        uint32_t rgb = pixel & 0x00FFFFFF;
        if (lut[rgb] != -1)
        {
            return lut[rgb];
        }

        // Fallback: closest color (rarely works)
        int bestIdx = 1, minDist = ColorDist(rgb, palette[1]);
        for (size_t i = 2; i < palette.size(); ++i)
        {
            int d = ColorDist(rgb, palette[i]);
            if (d < minDist)
            {
                minDist = d;
                bestIdx = static_cast<int>(i);
            }
        }

        return bestIdx;
    }
};

bool GifExportService::ExportGif(const ExportSequenceData &exportData)
{
    // Filter frames by tag
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

    // Parameters
    int frameW = exportData.appState->imageSize * exportData.scale;
    int frameH = exportData.appState->imageSize * exportData.scale;

    // File path
    std::wstring gifPath = exportData.outputFolder + L"\\" + exportData.fileName + L".gif";
    int size = WideCharToMultiByte(CP_UTF8, 0, gifPath.c_str(), -1, nullptr, 0, nullptr, nullptr);
    std::vector<char> utf8Path(size);

    WideCharToMultiByte(CP_UTF8, 0, gifPath.c_str(), -1, utf8Path.data(), size, nullptr, nullptr);

    // Building a global palette
    PaletteBuilder pb;
    pb.Build(filteredFrames, frameW, frameH, exportData.scale);

    // Preparing a palette for giflib
    std::vector<GifColorType> gifColors(256);
    for (size_t i = 0; i < 256; ++i)
    {
        uint32_t c = pb.palette[i];
        gifColors[i].Red = (c >> 16) & 0xFF;
        gifColors[i].Green = (c >> 8) & 0xFF;
        gifColors[i].Blue = c & 0xFF;
    }

    ColorMapObject *ColorMap = GifMakeMapObject(256, gifColors.data());
    if (!ColorMap)
    {
        return false;
    }

    // Opening a file
    int errorCode = 0;
    GifFileType *Gif = EGifOpenFileName(utf8Path.data(), false, &errorCode);

    if (!Gif)
    {
        GifFreeMapObject(ColorMap);
        return false;
    }

    bool success = true;

    // Screen Title
    if (EGifPutScreenDesc(Gif, frameW, frameH, 8, 0, ColorMap) == GIF_ERROR)
    {
        success = false;
    }

    // Loop: Infinite
    if (success && exportData.infiniteLoop)
    {
        // Netscape 2.0 Application Extension
        const unsigned char appExt[] = {'N', 'E', 'T', 'S', 'C', 'A', 'P', 'E', '2', '.', '0'};
        const unsigned char loopData[] = {0x01, 0x00, 0x00}; // 0x0000 = infinite loop

        //  Leader/Block/Trailer
        if (EGifPutExtensionLeader(Gif, APPLICATION_EXT_FUNC_CODE) == GIF_ERROR)
        {
            success = false;
        }

        else if (EGifPutExtensionBlock(Gif, 11, appExt) == GIF_ERROR)
        {
            success = false;
        }

        else if (EGifPutExtensionBlock(Gif, 3, loopData) == GIF_ERROR)
        {
            success = false;
        }

        else if (EGifPutExtensionTrailer(Gif) == GIF_ERROR)
        {
            success = false;
        }
    }

    // Recording frames
    if (success)
    {
        int duration = 100 / exportData.appState->playbackFPS;

        if (duration < 1)
        {
            duration = 1;
        }

        for (const auto *frame : filteredFrames)
        {
            // Indexing the frame into the palette
            std::vector<GifByteType> indexed(frameW * frameH);

            if (exportData.scale == 1)
            {
                for (int y = 0; y < exportData.appState->imageSize; ++y)
                {
                    for (int x = 0; x < exportData.appState->imageSize; ++x)
                    {
                        indexed[y * frameW + x] = static_cast<GifByteType>(pb.MapPixel(frame->GetPixel(x, y)));
                    }
                }
            }
            else
            {
                for (int sy = 0; sy < frameH; ++sy)
                {
                    for (int sx = 0; sx < frameW; ++sx)
                    {
                        int srcX = sx / exportData.scale;
                        int srcY = sy / exportData.scale;
                        indexed[sy * frameW + sx] = static_cast<GifByteType>(pb.MapPixel(frame->GetPixel(srcX, srcY)));
                    }
                }
            }

            // Graphic Control Extension (delay + transparency)
            unsigned char gce[4] = {
                0x00, // Packed: 00000000 (no disposal, transparency by index 0)
                static_cast<unsigned char>(duration & 0xFF), static_cast<unsigned char>((duration >> 8) & 0xFF),
                0 // Transparent Color Index = 0
            };

            if (EGifPutExtension(Gif, GRAPHICS_EXT_FUNC_CODE, 4, gce) == GIF_ERROR)
            {
                success = false;
                break;
            }

            if (EGifPutImageDesc(Gif, 0, 0, frameW, frameH, false, nullptr) == GIF_ERROR)
            {
                success = false;
                break;
            }

            // We write line by line
            for (int y = 0; y < frameH; ++y)
            {
                if (EGifPutLine(Gif, indexed.data() + y * frameW, frameW) == GIF_ERROR)
                {
                    success = false;
                    break;
                }
            }
            if (!success)
            {
                break;
            }
            
        }
    }

    // Conclusion
    GifFreeMapObject(ColorMap);

    int closeResult = EGifCloseFile(Gif, &errorCode);

    // We return success only if everything went without errors.
    return success && (closeResult == GIF_OK);
}

} // namespace baresprite