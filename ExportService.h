#pragma once

#include "Frame.h"
#include <string>

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
};

} // namespace baresprite