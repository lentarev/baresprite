#pragma once

namespace baresprite
{
enum class ToolType
{

    Brush,
    Eraser,
    Select,
    Fill,
    Move,
    Mirror,
    RotateL,
    RotateR,

    Count
};

inline const wchar_t *GetToolLabel(ToolType tool)
{
    switch (tool)
    {
    case ToolType::Brush:
        return L"Brush";

    case ToolType::Eraser:
        return L"Eraser";

    case ToolType::Select:
        return L"Select";

    case ToolType::Fill:
        return L"Fill";

    case ToolType::Move:
        return L"Move";

    case ToolType::Mirror:
        return L"Mirror";

    case ToolType::RotateL:
        return L"<- 90";

    case ToolType::RotateR:
        return L"90 ->";

    default:
        return L"Unknown";
    }
}

constexpr int ToolTypeCount()
{
    return static_cast<int>(ToolType::Count);
}
} // namespace baresprite
