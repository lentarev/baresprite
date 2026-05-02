#pragma once

#include <Windows.h>
#include "SelectionState.h"

namespace baresprite
{

class SelectionRenderer
{
  public:
    SelectionRenderer();
    ~SelectionRenderer();

    void Render(HDC hdc, const SelectionState &sel, int checkerSize);
};

} // namespace baresprite