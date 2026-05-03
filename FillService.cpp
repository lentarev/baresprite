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
    // 1. Проверка границ кадра
    if (startX < 0 || startX >= frame.width || startY < 0 || startY >= frame.height)
        return;

    // 2. Получаем целевой цвет (тот, который будем заменять)
    uint32_t targetColor = frame.GetPixel(startX, startY);

    // Если цвета совпадают — выходим (нечего заливать)
    if (targetColor == newColor)
        return;

    // 3. Стек для итеративного обхода (безопасно от переполнения)
    std::vector<std::pair<int, int>> stack;
    stack.reserve(1024); // Предвыделение памяти для производительности
    stack.emplace_back(startX, startY);

    while (!stack.empty())
    {
        auto [x, y] = stack.back();
        stack.pop_back();

        // Проверка границ кадра
        if (x < 0 || x >= frame.width || y < 0 || y >= frame.height)
            continue;

        // Проверка границ выделения (если активно)
        if (sel.isActive)
        {
            if (x < sel.x || x >= sel.x + sel.w || y < sel.y || y >= sel.y + sel.h)
                continue;
        }

        // Если пиксель не того цвета, который мы хотим заменить — пропускаем
        if (frame.GetPixel(x, y) != targetColor)
            continue;

        // Закрашиваем пиксель
        frame.SetPixel(x, y, newColor);

        // Добавляем 4 соседей в стек для проверки
        stack.emplace_back(x + 1, y);
        stack.emplace_back(x - 1, y);
        stack.emplace_back(x, y + 1);
        stack.emplace_back(x, y - 1);
    }
}

} // namespace baresprite