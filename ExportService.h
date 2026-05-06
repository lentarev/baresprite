#pragma once

#include "Frame.h"
#include <string>
#include <vector>

namespace baresprite
{

class ExportService
{
  public:
    ExportService() = delete;
    ~ExportService() = delete;
    ExportService(const ExportService &) = delete;
    ExportService &operator=(const ExportService &) = delete;

    // Exporting a single frame to PNG
    static bool ExportFrameToPNG(const Frame &frame, const std::wstring &filePath);

    // Export sequence
    static bool ExportSequence(const std::vector<Frame> &frames, const std::wstring &tagFilter, const std::wstring &outputFolder, std::wstring &outMessage);
};

} // namespace baresprite