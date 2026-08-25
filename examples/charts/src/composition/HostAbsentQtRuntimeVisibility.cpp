#include "src/composition/HostAbsentQtRuntimeVisibility.h"

#include "src/composition/PluginUiSession.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#include <atomic>
#include <string>
#include <string_view>

namespace cw_api3d::composition
{
  namespace
  {
    void logWarn(const std::string_view message) noexcept
    {
      try
      {
        const auto logger = PluginUiSession::instance().logger();
        if (logger)
        {
          logger->warn(message);
        }
      }
      catch (...)
      {
      }
    }

    [[nodiscard]] std::wstring queryPath()
    {
      const DWORD size = GetEnvironmentVariableW(L"PATH", nullptr, 0);
      if (size == 0)
      {
        return {};
      }
      std::wstring path(size, L'\0');
      const DWORD written = GetEnvironmentVariableW(L"PATH", path.data(), size);
      if (written == 0 || written >= size)
      {
        return {};
      }
      path.resize(written);
      return path;
    }

    [[nodiscard]] std::filesystem::path loadedModuleDirectory()
    {
      HMODULE module = nullptr;
      if (GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                             reinterpret_cast<LPCWSTR>(static_cast<const void*>(&ensureHostAbsentQtRuntimeVisible)),
                             &module)
            == 0
          || module == nullptr)
      {
        return {};
      }

      std::wstring buffer(MAX_PATH, L'\0');
      for (;;)
      {
        const DWORD length = GetModuleFileNameW(module, buffer.data(), static_cast<DWORD>(buffer.size()));
        if (length == 0)
        {
          return {};
        }
        if (length < buffer.size())
        {
          buffer.resize(length);
          break;
        }
        buffer.assign(buffer.size() * 2, L'\0');
      }

      return std::filesystem::path(buffer).parent_path();
    }
  } // namespace

  void addDirectoryToDllSearchPath(const std::filesystem::path& dir)
  {
    const std::wstring dirW = dir.wstring();
    if (dirW.empty())
    {
      return;
    }

    if (AddDllDirectory(dirW.c_str()) == nullptr)
    {
      logWarn("AddDllDirectory failed for host-absent Qt runtime directory");
    }

    std::wstring newPath = dirW;
    const std::wstring currentPath = queryPath();
    if (!currentPath.empty())
    {
      newPath.push_back(L';');
      newPath.append(currentPath);
    }
    if (SetEnvironmentVariableW(L"PATH", newPath.c_str()) == 0)
    {
      logWarn("Failed to prepend plugin directory to PATH");
    }
  }

  void ensureHostAbsentQtRuntimeVisible() noexcept
  {
    try
    {
      static std::atomic<bool> done{false};
      if (done.exchange(true))
      {
        return;
      }

      const std::filesystem::path pluginDir = loadedModuleDirectory();
      if (pluginDir.empty())
      {
        logWarn("Cannot resolve plugin directory for host-absent Qt runtime");
        return;
      }

      addDirectoryToDllSearchPath(pluginDir);
    }
    catch (...)
    {
      logWarn("Failed to make host-absent Qt runtime visible");
    }
  }

} // namespace cw_api3d::composition
