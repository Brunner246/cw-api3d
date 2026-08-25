#include "src/composition/HostAbsentQtRuntimeVisibility.h"

#include <gtest/gtest.h>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#include <filesystem>
#include <string>

namespace cw_api3d::tests::composition
{

  [[nodiscard]] std::wstring currentPath()
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

  TEST(HostAbsentQtRuntimeVisibilityTests, AddDirectoryToDllSearchPathPrependsPath)
  {
    const auto tempDir =
      std::filesystem::temp_directory_path() / "cw_api3d_host_absent_qt_runtime_visibility";
    std::filesystem::create_directories(tempDir);

    cw_api3d::composition::addDirectoryToDllSearchPath(tempDir);

    const std::wstring path = currentPath();
    EXPECT_NE(path.find(tempDir.wstring()), std::wstring::npos);
  }

} // namespace cw_api3d::tests::composition
