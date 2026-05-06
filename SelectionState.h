#pragma once

namespace baresprite
{

struct SelectionState
{
    bool isActive = false;
    bool isDragging = false;

    int x = 0, y = 0;
    int w = 0, h = 0;

    float rotationAngle = 0.0f;

    void Clear()
    {
        isActive = false;
        isDragging = false;
        x = y = w = h = 0;
        rotationAngle = 0.0f;
    }

    // For drag: fix the starting point
    int startX, startY;

    void Normalize()
    {
        if (w < 0)
        {
            x += w;
            w = -w;
        }

        if (h < 0)
        {
            y += h;
            h = -h;
        }

        if (w == 0)
        {
            w = 1;
        }

        if (h == 0)
        {
            h = 1;
        }
    }
};

} // namespace baresprite