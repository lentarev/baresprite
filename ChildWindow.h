#pragma once

namespace baresprite
{
class ChildWindow
{
  public:
    virtual ~ChildWindow() = default;

    virtual void OnSize(int width, int height) = 0;

    virtual bool OnCommand(int commandId) = 0;
};

} // namespace baresprite