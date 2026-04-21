#pragma once

#include <Windows.h>
#include "Project.h"

namespace baresprite
{

class ChessBackground
{
  public:
    ChessBackground(Project &projectData);

    ~ChessBackground();

    void Render(const PAINTSTRUCT &ps, HDC hdc) const;

  private:
    HBRUSH _hBrushLight;
    HBRUSH _hBrushDark;

    // Link to the project data structure.
    Project &_projectData;
};

} // namespace baresprite