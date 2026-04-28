#include "FrameService.h"
#include "Frame.h"

namespace baresprite
{

bool FrameService::NewFrame(AppState &appState)
{
    try
    {
        // Создаём кадр
        Frame newFrame(appState.imageSize, appState.imageSize);
        newFrame.tag = L"";

        // Вставляем после текущего
        auto it = appState.frames.begin() + appState.currentFrameIndex + 1;
        appState.frames.insert(it, std::move(newFrame));

        // Переключаемся на новый
        ++appState.currentFrameIndex;
        return true;
    }
    catch (const std::bad_alloc &)
    {
        return false;
    }
}

} // namespace baresprite