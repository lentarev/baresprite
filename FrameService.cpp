#include "FrameService.h"
#include "Frame.h"

namespace baresprite
{

/// <summary>
/// New Frame
/// </summary>
/// <param name="appState"></param>
/// <returns></returns>
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

/// <summary>
/// Prev Frame
/// </summary>
/// <param name="appState"></param>
/// <returns></returns>
bool FrameService::PrevFrame(AppState &appState)
{
    // Если кадров нет или мы уже на первом
    if (appState.frames.empty() || appState.currentFrameIndex <= 0)
    {
        return false;
    }

    // Сдвигаем индекс назад
    --appState.currentFrameIndex;
    return true;
}

/// <summary>
/// Next Frame
/// </summary>
/// <param name="appState"></param>
/// <returns></returns>
bool FrameService::NextFrame(AppState &appState)
{
    // Защита от пустого списка
    if (appState.frames.empty())
    {
        return false;
    }

    // Вычисляем индекс последнего кадра
    int lastIndex = static_cast<int>(appState.frames.size()) - 1;

    // Если мы ещё не в конце, то переключаемся
    if (appState.currentFrameIndex < lastIndex)
    {
        ++appState.currentFrameIndex;
        return true;
    }

    // Уже на последнем кадре, движение невозможно
    return false;
}

/// <summary>
/// Clone Frame
/// </summary>
/// <param name="appState"></param>
/// <returns></returns>
bool FrameService::CloneFrame(AppState &appState)
{
    if (appState.frames.empty())
    {
        return false; // Нечего клонировать
    }

    try
    {
        const Frame &source = appState.frames[appState.currentFrameIndex];

        Frame clone(source.width, source.height);
        size_t pixelCount = static_cast<size_t>(source.width) * source.height;

        std::copy(source.pixels.get(), source.pixels.get() + pixelCount, clone.pixels.get());

        clone.tag = source.tag;
        clone.duration = source.duration;
        clone.isVisible = source.isVisible;

        auto insertPos = appState.frames.begin() + appState.currentFrameIndex + 1;
        appState.frames.insert(insertPos, std::move(clone));

        ++appState.currentFrameIndex;
        return true;
    }
    catch (const std::bad_alloc &)
    {
        return false;
    }
}


/// <summary>
/// Delete Frame
/// </summary>
/// <param name="appState"></param>
/// <returns></returns>
bool FrameService::DeleteFrame(AppState &appState)
{
    // в проекте всегда должен оставаться минимум 1 кадр
    if (appState.frames.size() <= 1)
    {
        return false;
    }

    // Удаляем текущий кадр
    appState.frames.erase(appState.frames.begin() + appState.currentFrameIndex);

    // Корректировка индекса:
    // Если мы удалили последний кадр в списке, currentFrameIndex станет равен size() (out of bounds).
    // Сдвигаем его на предыдущий (теперь последний) кадр.
    if (appState.currentFrameIndex >= static_cast<int>(appState.frames.size()))
    {
        --appState.currentFrameIndex;
    }

    return true; // Кадр успешно удалён
}

Frame &FrameService::GetCurrentFrame(AppState &state)
{
    // Простая защита от дурака (если кадров вдруг нет)
    if (state.frames.empty())
    {

        static Frame dummy;
        return dummy;
    }
    return state.frames[state.currentFrameIndex];
}

const Frame &FrameService::GetCurrentFrame(const AppState &state)
{
    if (state.frames.empty())
    {
        static Frame dummy;
        return dummy;
    }
    return state.frames[state.currentFrameIndex];
}

} // namespace baresprite