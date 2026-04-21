#pragma once

namespace baresprite
{
class ChildWindow
{
  public:
    virtual ~ChildWindow() = default;

    virtual void OnSize(int clientW, int clientH) = 0;

    virtual bool OnCommand(int commandId) = 0;
};

} // namespace baresprite