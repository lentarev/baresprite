#pragma once

#include <filesystem>
#include <string>

namespace baresprite
{
class Settings
{
  public:
    virtual ~Settings() = default;

    virtual bool Load() = 0;

    virtual void Save() = 0;

  protected:
    /// <summary>
    /// Get project name from path
    /// </summary>
    /// <param name="fullPath"></param>
    /// <returns></returns>
    std::wstring GetProjectNameFromPath(const std::wstring &fullPath)
    {
        namespace fs = std::filesystem;

        fs::path path(fullPath);

        return path.filename().wstring();
    }
};

} // namespace baresprite