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
    if (appState.frames.empty())
    {
        return false;
    }

    const std::wstring &filter = appState.currentFilterTag;
    int startIndex = appState.currentFrameIndex;

    int newIndex = startIndex;
    bool wrapped = false;

    // Если фильтр активен — ищем предыдущий подходящий кадр с зацикливанием
    if (!filter.empty())
    {
        do
        {
            // Двигаемся назад
            newIndex--;
            if (newIndex < 0)
            {
                newIndex = static_cast<int>(appState.frames.size()) - 1; // Зацикливание: начало → конец
                if (wrapped)
                {
                    break;
                }

                wrapped = true;
            }

            // Проверка на соответствие фильтру
            if (MatchesFilter(appState.frames[newIndex], filter))
            {
                appState.currentFrameIndex = newIndex;

                return true;
            }
        } while (newIndex != startIndex);

        // Если ни один кадр не подошёл — остаёмся на текущем
        return false;
    }
    else
    {
        // Фильтр выключен — старая логика
        if (appState.currentFrameIndex <= 0)
        {
            appState.currentFrameIndex = static_cast<int>(appState.frames.size()) - 1;
        }

        else
        {
            --appState.currentFrameIndex;
        }

        return true;
    }
}

/// <summary>
/// Next Frame
/// </summary>
/// <param name="appState"></param>
/// <returns></returns>
bool FrameService::NextFrame(AppState &appState)
{
    if (appState.frames.empty())
    {
        return false;
    }

    const std::wstring &filter = appState.currentFilterTag;
    int startIndex = appState.currentFrameIndex;

    int newIndex = startIndex;
    bool wrapped = false; // Флаг, чтобы не зациклиться бесконечно

    // Если фильтр активен — ищем следующий подходящий кадр с зацикливанием
    if (!filter.empty())
    {
        do
        {
            // Двигаемся вперёд
            newIndex++;

            if (newIndex >= static_cast<int>(appState.frames.size()))
            {
                newIndex = 0; // Зацикливание: конец → начало

                if (wrapped)
                {
                    break; // Уже прошли полный круг → стоп
                }

                wrapped = true;
            }

            // Проверка на соответствие фильтру
            if (MatchesFilter(appState.frames[newIndex], filter))
            {
                appState.currentFrameIndex = newIndex;

                return true;
            }

        } while (newIndex != startIndex); // Пока не вернулись к старту

        // Если ни один кадр не подошёл — остаёмся на текущем
        return false;
    }
    else
    {
        // Фильтр выключен — старая логика (простой цикл)
        int lastIndex = static_cast<int>(appState.frames.size()) - 1;

        if (appState.currentFrameIndex >= lastIndex)
        {
            appState.currentFrameIndex = 0;
        }

        else
        {
            ++appState.currentFrameIndex;
        }

        return true;
    }
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

bool FrameService::MatchesFilter(const Frame &frame, const std::wstring &filterTag)
{
    if (filterTag.empty())
    {
        return true; // <All> = всё подходит
    }

    if (filterTag == L"None")
    {
        return frame.tag.empty(); // None = только кадры без тега
    }

    return frame.tag == filterTag; // Обычный тег = точное совпадение
}

} // namespace baresprite