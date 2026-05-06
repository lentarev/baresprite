#pragma once

#include "AppState.h"

namespace baresprite
{

class RotateService
{
  public:
    RotateService() = delete;
    ~RotateService() = delete;
    RotateService(const RotateService &) = delete;
    RotateService &operator=(const RotateService &) = delete;

    static void RotateSelection90R(AppState &appState, HWND hCanvas);
    static void RotateSelection90L(AppState &appState, HWND hCanvas);

  private:
    static constexpr float PI = 3.14159265358979323846f;
};

} // namespace baresprite