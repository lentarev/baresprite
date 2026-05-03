#pragma once

#include "Frame.h"
#include <vector>


namespace baresprite
{

class HistoryService
{
  public:
    HistoryService() = delete;
    ~HistoryService() = delete;
    HistoryService(const HistoryService &) = delete;
    HistoryService &operator=(const HistoryService &) = delete;

    static Frame DeepCopyFrame(const Frame &src);
    static void DeepCopyFrames(const std::vector<Frame> &src, std::vector<Frame> &dst);
};

} // namespace baresprite