#include "HistoryService.h"
#include <cstring>
#include <utility>

namespace baresprite
{

// Внутренняя функция копирования одного кадра
Frame HistoryService::DeepCopyFrame(const Frame &src)
{
    Frame dst(src.width, src.height);
    dst.duration = src.duration;
    dst.isVisible = src.isVisible;
    dst.tag = src.tag;

    if (src.pixels && dst.pixels && src.width > 0 && src.height > 0)
    {
        size_t count = static_cast<size_t>(src.width) * src.height;
        std::memcpy(dst.pixels.get(), src.pixels.get(), count * sizeof(uint32_t));
    }
    return dst;
}

void HistoryService::DeepCopyFrames(const std::vector<Frame> &src, std::vector<Frame> &dst)
{
    dst.clear();
    dst.reserve(src.size());

    for (const auto &f : src)
    {
        // Явно перемещаем результат в вектор-приёмник
        dst.push_back(std::move(DeepCopyFrame(f)));
    }
}

} // namespace baresprite