#pragma once
#include "Project.h"
#include "Settings.h"

namespace baresprite
{

class AppSettings : public Settings
{
  public:
    AppSettings(Project &projectData);
    ~AppSettings() override;

    bool Load() override;
    void Save() override;

  private:
    Project &_projectData;
};

} // namespace baresprite