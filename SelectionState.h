#pragma once

namespace baresprite
{

struct SelectionState
{
    bool isActive = false;   // Выделение активно (показываем рамку)
    bool isDragging = false; // Пользователь тянет прямоугольник

    // Координаты в логических пикселях кадра
    int x = 0, y = 0; // Левый верхний угол
    int w = 0, h = 0; // Ширина и высота (могут быть отрицательными при драге)

    float rotationAngle = 0.0f;

    // Сброс выделения
    void Clear()
    {
        isActive = false;
        isDragging = false;
        x = y = w = h = 0;
        rotationAngle = 0.0f;
    }

    // Для драга: фиксируем точку старта
    int startX, startY; 

    // Нормализация: гарантирует положительные w/h и корректные x/y
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
            w = 1; // Минимальный размер 1x1
        }

        if (h == 0)
        {
            h = 1;
        }
    }
};

} // namespace baresprite