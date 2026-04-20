#pragma once

namespace baresprite
{
class Settings
{
  public:
    virtual ~Settings() = default;

    virtual bool Load() = 0;

    virtual void Save() = 0;
};

} // namespace baresprite