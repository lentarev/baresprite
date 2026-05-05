#pragma once

#include "ExportSequenceData.h"

namespace baresprite
{

class SpritesheetService
{
  public:
    SpritesheetService() = delete;
    ~SpritesheetService() = delete;
    SpritesheetService(const SpritesheetService &) = delete;
    SpritesheetService &operator=(const SpritesheetService &) = delete;

    static bool BuildSpritesheet(ExportSequenceData &exportData);
};

} // namespace baresprite