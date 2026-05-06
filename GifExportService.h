#pragma once

#include "ExportSequenceData.h"

namespace baresprite
{

class GifExportService
{
  public:
    GifExportService() = delete;
    ~GifExportService() = delete;
    GifExportService(const GifExportService &) = delete;
    GifExportService &operator=(const GifExportService &) = delete;


    static bool ExportGif(const ExportSequenceData &exportData);
};

} // namespace baresprite