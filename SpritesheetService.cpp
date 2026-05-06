
#include "SpritesheetService.h"
#include "Frame.h"
#include "stb_image_write.h"
#include <cmath>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

namespace fs = std::filesystem;

namespace baresprite
{

bool SpritesheetService::BuildSpritesheet(ExportSequenceData &exportData)
{
    // Filtering frames by tag
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
    int frameW = exportData.appState->imageSize;
    int frameH = exportData.appState->imageSize;
    int cols = exportData.columns;
    int padding = exportData.padding;
    int count = static_cast<int>(filteredFrames.size());

    // Rows
    int rows = (count + cols - 1) / cols;

    // Atlas size
    int atlasW = cols * frameW + (cols - 1) * padding;
    int atlasH = rows * frameH + (rows - 1) * padding;

    // Creating a buffer for the atlas (RGBA bytes)
    std::vector<unsigned char> atlasBuffer(atlasW * atlasH * 4, 0); // 0 = completely transparent

    // Copying frames to the atlas
    for (int i = 0; i < count; ++i)
    {
        const Frame *frame = filteredFrames[i];

        // Position in the grid
        int col = i % cols;
        int row = i / cols;

        // Atlas coordinates (in pixels)
        int dstX = col * (frameW + padding);
        int dstY = row * (frameH + padding);

        // Copying pixels
        for (int y = 0; y < frameH; ++y)
        {
            for (int x = 0; x < frameW; ++x)
            {
                uint32_t pixel = frame->GetPixel(x, y);

                // Extracting components from 0xAARRGGBB
                unsigned char a = (pixel >> 24) & 0xFF;
                unsigned char r = (pixel >> 16) & 0xFF;
                unsigned char g = (pixel >> 8) & 0xFF;
                unsigned char b = pixel & 0xFF;

                // Position in the atlas buffer
                int atlasIdx = ((dstY + y) * atlasW + (dstX + x)) * 4;

                atlasBuffer[atlasIdx + 0] = r;
                atlasBuffer[atlasIdx + 1] = g;
                atlasBuffer[atlasIdx + 2] = b;
                atlasBuffer[atlasIdx + 3] = a;
            }
        }
    }

    // Save as PNG
    std::wstring pngPath = exportData.outputFolder + L"\\" + exportData.fileName + L".png";

    // Converting a path to UTF-8 for stbi
    int size = WideCharToMultiByte(CP_UTF8, 0, pngPath.c_str(), -1, nullptr, 0, nullptr, nullptr);
    std::vector<char> utf8Path(size);
    WideCharToMultiByte(CP_UTF8, 0, pngPath.c_str(), -1, utf8Path.data(), size, nullptr, nullptr);

    if (!stbi_write_png(utf8Path.data(), atlasW, atlasH, 4, atlasBuffer.data(), 0))
    {
        return false;
    }

    // Generate JSON (if needed)
    if (exportData.generateJson)
    {
        std::wstring jsonPath = exportData.outputFolder + L"\\" + exportData.fileName + L".json";
        std::wofstream jsonFile(jsonPath);

        if (jsonFile.is_open())
        {
            jsonFile << L"{\n";
            jsonFile << L"  \"meta\": {\n";
            jsonFile << L"    \"app\": \"BareSprite\",\n";
            jsonFile << L"    \"version\": \"1.0\",\n";
            jsonFile << L"    \"image\": \"" << exportData.fileName << L".png\",\n";
            jsonFile << L"    \"size\": { \"w\": " << atlasW << L", \"h\": " << atlasH << L" }\n";
            jsonFile << L"  },\n";
            jsonFile << L"  \"frames\": {\n";

            for (int i = 0; i < count; ++i)
            {
                int col = i % cols;
                int row = i / cols;
                int frameX = col * (frameW + padding);
                int frameY = row * (frameH + padding);

                // Frame name in JSON
                std::wstring frameName = exportData.fileName + L"_" + std::to_wstring(i);

                jsonFile << L"    \"" << frameName << L"\": {\n";
                jsonFile << L"      \"frame\": { \"x\": " << frameX << L", \"y\": " << frameY << L", \"w\": " << frameW << L", \"h\": " << frameH << L" },\n";
                jsonFile << L"      \"duration\": 100,\n";
                jsonFile << L"      \"tag\": \"" << filteredFrames[i]->tag << L"\"\n";
                jsonFile << L"    }";

                if (i < count - 1)
                    jsonFile << L",";
                jsonFile << L"\n";
            }

            jsonFile << L"  }\n";
            jsonFile << L"}\n";
            jsonFile.close();
        }
    }

    return true;
}

} // namespace baresprite
