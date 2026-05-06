#include "FillService.h"
#include <vector>

namespace baresprite
{

/// <summary>
/// Flood Fill
/// </summary>
/// <param name="frame"></param>
/// <param name="sel"></param>
/// <param name="startX"></param>
/// <param name="startY"></param>
/// <param name="newColor"></param>
void FillService::PerformFill(Frame &frame, const SelectionState &sel, int startX, int startY, uint32_t newColor)
{
    // Checking frame boundaries
    if (startX < 0 || startX >= frame.width || startY < 0 || startY >= frame.height)
        return;

    // We get the target color (the one we will replace)
    uint32_t targetColor = frame.GetPixel(startX, startY);

    // If the colors match, exit (nothing to fill in)
    if (targetColor == newColor)
        return;

    // Stack for iterative traversal (overflow safe)
    std::vector<std::pair<int, int>> stack;
    stack.reserve(1024); // Pre-allocating memory for performance
    stack.emplace_back(startX, startY);

    while (!stack.empty())
    {
        auto [x, y] = stack.back();
        stack.pop_back();

        // Checking frame boundaries
        if (x < 0 || x >= frame.width || y < 0 || y >= frame.height)
            continue;

        // Check selection boundaries (if active)
        if (sel.isActive)
        {
            if (x < sel.x || x >= sel.x + sel.w || y < sel.y || y >= sel.y + sel.h)
                continue;
        }

        // If the pixel is not the color we want to replace, we skip it.
        if (frame.GetPixel(x, y) != targetColor)
            continue;

        // Filling in a pixel
        frame.SetPixel(x, y, newColor);

        // Add 4 neighbors to the stack for testing
        stack.emplace_back(x + 1, y);
        stack.emplace_back(x - 1, y);
        stack.emplace_back(x, y + 1);
        stack.emplace_back(x, y - 1);
    }
}

} // namespace baresprite