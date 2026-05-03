#pragma once

#include <cstdint>
#include <vector>

namespace baresprite
{

struct MoveDragState
{
    bool isDragging = false;
    int startLogX = 0, startLogY = 0;  // Где нажали мышкой (логические координаты)
    int prevDx = 0, prevDy = 0;        // Смещение предыдущей отрисовки
    int currDx = 0, currDy = 0;        // Смещение текущей отрисовки
    std::vector<uint32_t> pixelBuffer; // Копия пикселей выделения
    int w = 0, h = 0;
    int origSelX = 0, origSelY = 0; // Исходная позиция выделения
};

} // namespace baresprite