#pragma once
#include "Settings.h"
#include "Project.h"

namespace baresprite
{

class ProjectSettings : public Settings
{
  public:
    ProjectSettings(Project &projectData);
    ~ProjectSettings() override;

    bool Load() override;
    void Save() override;

  private:
    Project &_projectData;
};

} // namespace baresprite

