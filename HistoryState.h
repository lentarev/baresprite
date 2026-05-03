#pragma once
#include "Frame.h"
#include "HistoryService.h"
#include <memory>
#include <vector>

namespace baresprite
{
struct HistoryState
{
    // Храним указатели на векторы, а не сами векторы
    std::vector<std::unique_ptr<std::vector<Frame>>> undoStack;
    std::vector<std::unique_ptr<std::vector<Frame>>> redoStack;
    static constexpr int MAX_STEPS = 32;

    // Сохраняет снимок текущего состояния
    void Commit(std::vector<Frame> &currentFrames)
    {
        auto snapshot = std::make_unique<std::vector<Frame>>();
        HistoryService::DeepCopyFrames(currentFrames, *snapshot);

        undoStack.push_back(std::move(snapshot));

        if (undoStack.size() > MAX_STEPS)
            undoStack.erase(undoStack.begin());

        redoStack.clear();
    }

    bool CanUndo() const
    {
        return !undoStack.empty();
    }
    bool CanRedo() const
    {
        return !redoStack.empty();
    }

    // Восстанавливает состояние через SWAP (без копирования)
    void Undo(std::vector<Frame> &currentFrames)
    {
        if (undoStack.empty())
            return;

        // Текущее состояние уходит в Redo
        redoStack.push_back(std::make_unique<std::vector<Frame>>(std::move(currentFrames)));

        // Предыдущее состояние становится текущим
        currentFrames.swap(*undoStack.back());
        undoStack.pop_back();
    }

    void Redo(std::vector<Frame> &currentFrames)
    {
        if (redoStack.empty())
            return;

        undoStack.push_back(std::make_unique<std::vector<Frame>>(std::move(currentFrames)));
        currentFrames.swap(*redoStack.back());
        redoStack.pop_back();
    }

    void Clear()
    {
        undoStack.clear();
        redoStack.clear();
    }
};
} // namespace baresprite