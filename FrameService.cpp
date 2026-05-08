#include "FrameService.h"
#include "Frame.h"
#include <iostream>

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
        // Create frame
        Frame newFrame(appState.imageSize, appState.imageSize);
        newFrame.tag = appState.selectedTag;

        std::wcout << appState.selectedTag << std::endl;

        // Insert after the current one
        auto it = appState.frames.begin() + appState.currentFrameIndex + 1;
        appState.frames.insert(it, std::move(newFrame));

        // Switching to a new one
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

    // If the filter is active, we search for the previous suitable frame with a loop
    if (!filter.empty())
    {
        do
        {
            // Moving back
            newIndex--;
            if (newIndex < 0)
            {
                newIndex = static_cast<int>(appState.frames.size()) - 1; // Looping: start -> end
                if (wrapped)
                {
                    break;
                }

                wrapped = true;
            }

            // Checking for filter compliance
            if (MatchesFilter(appState.frames[newIndex], filter))
            {
                appState.currentFrameIndex = newIndex;

                return true;
            }
        } while (newIndex != startIndex);

        // If none of the frames are suitable, we stay with the current one
        return false;
    }
    else
    {
        // Filter off - old logic
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
    bool wrapped = false; // A flag to avoid endless looping

    // If the filter is active, we search for the next suitable frame with a loop
    if (!filter.empty())
    {
        do
        {
            // Moving forward
            newIndex++;

            if (newIndex >= static_cast<int>(appState.frames.size()))
            {
                newIndex = 0; // Looping: end -> start

                if (wrapped)
                {
                    break; // We've already come full circle -> stop
                }

                wrapped = true;
            }

            // Checking for filter compliance
            if (MatchesFilter(appState.frames[newIndex], filter))
            {
                appState.currentFrameIndex = newIndex;

                return true;
            }

        } while (newIndex != startIndex); // Until we get back to the start

        // If none of the frames are suitable, we stay with the current one
        return false;
    }
    else
    {
        // Filter off - old logic (simple loop)
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
        return false; // There is nothing to clone
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
    if (appState.frames.size() <= 1)
        return false;

    appState.frames.erase(appState.frames.begin() + appState.currentFrameIndex);

    if (appState.currentFrameIndex >= static_cast<int>(appState.frames.size()))
    {
        --appState.currentFrameIndex;
    }

    if (!appState.currentFilterTag.empty())
    {
        int count = GetNumberFramesByTag(appState);

        if (count == 0)
        {
            appState.currentFilterTag = L"";

            appState.startIndexByTag = FindFirstMatchingFrame(appState);
            appState.numberFramesByTag = GetNumberFramesByTag(appState);

            if (!appState.frames.empty() && (appState.currentFrameIndex < 0 || appState.currentFrameIndex >= static_cast<int>(appState.frames.size())))
            {
                appState.currentFrameIndex = 0;
            }
        }
    }

    appState.isDirty = true;
    return true;
}

Frame &FrameService::GetCurrentFrame(AppState &state)
{
    // Simple protection (if there are no frames)
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
    if (filterTag.empty() || filterTag == L"All")
        return true;

    if (filterTag == L"None")
        return frame.tag.empty() || frame.tag == L"None";

    return frame.tag == filterTag;
}

/// <summary>
/// Finds the index of the first frame matching the filter
/// Returns -1 if there are no suitable frames.
/// </summary>
/// <param name="appState"></param>
/// <returns></returns>
int FrameService::FindFirstMatchingFrame(AppState &appState)
{
    const std::wstring &filter = appState.currentFilterTag;

    if (filter.empty())
        return 0;

    for (size_t i = 0; i < appState.frames.size(); ++i)
    {
        const std::wstring &frameTag = appState.frames[i].tag;

        if (filter == L"None")
        {

            if (frameTag.empty() || frameTag == L"None")
                return static_cast<int>(i);
        }
        else
        {
            if (frameTag == filter)
                return static_cast<int>(i);
        }
    }
    return -1;
}

int FrameService::GetNumberFramesByTag(AppState &appState)
{
    const std::wstring &filter = appState.currentFilterTag;

    int numberFrames = 0;

    if (filter.empty())
        return 0;

    for (size_t i = 0; i < appState.frames.size(); ++i)
    {
        const std::wstring &frameTag = appState.frames[i].tag;

        if (filter == L"None")
        {
            if (frameTag.empty() || frameTag == L"None")
                numberFrames++;
        }
        else
        {

            if (frameTag == filter)
                numberFrames++;
        }
    }

    return numberFrames;
}

} // namespace baresprite