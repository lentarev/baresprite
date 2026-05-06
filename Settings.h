#pragma once

#include <Windows.h>
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

  protected:
    // Config file name
    std::wstring _configProjectFileName = L"baresprite.ini";

    /// <summary>
    /// Removes spaces and line breaks from the ends of a line.
    /// </summary>
    /// <param name="str"></param>
    /// <returns></returns>
    std::wstring Trim(const std::wstring &str)
    {
        size_t start = str.find_first_not_of(L" \t\r\n");
        if (start == std::wstring::npos)
            return L"";
        size_t end = str.find_last_not_of(L" \t\r\n");
        return str.substr(start, end - start + 1);
    }
};

} // namespace baresprite