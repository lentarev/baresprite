#pragma once
#include "AppState.h"
#include "Frame.h"
#include "HistoryService.h"
#include <memory>
#include <vector>

namespace baresprite
{

struct SelectionSnapshot
{
    int x = 0, y = 0, w = 0, h = 0;
    bool isActive = false;
};

struct HistorySnapshot
{
    std::unique_ptr<std::vector<Frame>> frames;
    SelectionSnapshot selection;
};

struct HistoryState
{
    std::vector<HistorySnapshot> undoStack;
    std::vector<HistorySnapshot> redoStack;
    static constexpr int MAX_STEPS = 32;

    // Accepts selection fields individually
    void Commit(std::vector<Frame> &currentFrames, int selX, int selY, int selW, int selH, bool selActive)
    {
        HistorySnapshot snapshot;
        snapshot.frames = std::make_unique<std::vector<Frame>>();
        HistoryService::DeepCopyFrames(currentFrames, *snapshot.frames);

        // Save selection parameters
        snapshot.selection = {selX, selY, selW, selH, selActive};

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

    // We restore both frames and selections
    void Undo(std::vector<Frame> &currentFrames, int &selX, int &selY, int &selW, int &selH, bool &selActive)
    {
        if (undoStack.empty())
            return;

        // Current state in redo
        HistorySnapshot redoSnap;
        redoSnap.frames = std::make_unique<std::vector<Frame>>(std::move(currentFrames));
        redoSnap.selection = {selX, selY, selW, selH, selActive};
        redoStack.push_back(std::move(redoSnap));

        auto &prev = undoStack.back();
        currentFrames.swap(*prev.frames);

        // Restore selection fields
        selX = prev.selection.x;
        selY = prev.selection.y;
        selW = prev.selection.w;
        selH = prev.selection.h;
        selActive = prev.selection.isActive;

        undoStack.pop_back();
    }

    void Redo(std::vector<Frame> &currentFrames, int &selX, int &selY, int &selW, int &selH, bool &selActive)
    {
        if (redoStack.empty())
            return;

        HistorySnapshot undoSnap;
        undoSnap.frames = std::make_unique<std::vector<Frame>>(std::move(currentFrames));
        undoSnap.selection = {selX, selY, selW, selH, selActive};
        undoStack.push_back(std::move(undoSnap));

        auto &next = redoStack.back();
        currentFrames.swap(*next.frames);

        // Restore selection fields
        selX = next.selection.x;
        selY = next.selection.y;
        selW = next.selection.w;
        selH = next.selection.h;
        selActive = next.selection.isActive;

        redoStack.pop_back();
    }

    void Clear()
    {
        undoStack.clear();
        redoStack.clear();
    }
};
} // namespace baresprite