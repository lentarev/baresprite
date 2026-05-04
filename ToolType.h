#pragma once

namespace baresprite
{
enum class ToolType
{

    Brush,
    Eraser,
    Select,
    Fill,
    MirrorV,
    MirrorH,
    RotateL,
    RotateR,
    Move,

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

    case ToolType::MirrorV:
        return L"Mirror V";

    case ToolType::MirrorH:
        return L"Mirror H";

    case ToolType::RotateL:
        return L"<- 90";

    case ToolType::RotateR:
        return L"90 ->";

    case ToolType::Move:
        return L"Move";

    default:
        return L"Unknown";
    }
}

constexpr int ToolTypeCount()
{
    return static_cast<int>(ToolType::Count);
}
} // namespace baresprite
