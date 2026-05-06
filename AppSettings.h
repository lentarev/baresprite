#pragma once
#include "AppState.h"
#include "Settings.h"


namespace baresprite
{

class AppSettings : public Settings
{
  public:
    AppSettings(AppState &appState);
    ~AppSettings() override;

    bool Load() override;
    void Save() override;

    bool IsProjectExist(const std::wstring &projectPath);

  private:
    AppState &_appState;
};

} // namespace baresprite