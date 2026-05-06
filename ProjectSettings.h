#pragma once
#include "AppState.h"
#include "Settings.h"

namespace baresprite
{

class ProjectSettings : public Settings
{
  public:
    ProjectSettings(AppState &appState);
    ~ProjectSettings() override;

    bool Load() override;
    void Save() override;

  private:
    AppState &_appState;
};

} // namespace baresprite
