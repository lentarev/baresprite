#pragma once

#include <cstdint>
#include "Frame.h"
#include "SelectionState.h"


namespace baresprite
{

class FillService
{
  public:
    FillService() = delete;
    ~FillService() = delete;
    FillService(const FillService &) = delete;
    FillService &operator=(const FillService &) = delete;

    /// <summary>
    /// Flood Fill
    /// </summary>
    /// <param name="frame"></param>
    /// <param name="sel"></param>
    /// <param name="startX"></param>
    /// <param name="startY"></param>
    /// <param name="newColor"></param>
    static void PerformFill(Frame &frame, const SelectionState &sel, int startX, int startY, uint32_t newColor);
};

} // namespace baresprite
