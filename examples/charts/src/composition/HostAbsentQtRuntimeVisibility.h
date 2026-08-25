#pragma once

#include <filesystem>

namespace cw_api3d::composition
{

  void ensureHostAbsentQtRuntimeVisible() noexcept;
  void addDirectoryToDllSearchPath(const std::filesystem::path& dir);

} // namespace cw_api3d::composition
