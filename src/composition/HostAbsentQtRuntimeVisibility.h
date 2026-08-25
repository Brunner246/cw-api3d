#pragma once

#include <filesystem>

namespace cw_api3d::composition
{

  /// Cadwork ships the Qt Graphs QML plugin (`pclib.x64/QtQml/QtGraphs/graphsplugin.dll`)
  /// but not `Qt6Graphs.dll` nor its Quick3D/QuickShapes/ShaderTools closure. The post-build
  /// copier deploys those next to the plugin DLL (`cmake/copy_non_qt_runtime_dlls.cmake`), yet
  /// that folder is not on the search path the host uses to load `graphsplugin.dll`. This makes
  /// it reachable before the dock's QML (`import QtGraphs`) is loaded.
  ///
  /// The directory is the loaded module's own (`GetModuleFileNameW`), deliberately not
  /// `IUtilityProvider::getPluginPath()`: the host value is a configured plugin path rather than
  /// the image cadwork actually mapped, and the deploy folder is a per-target subdirectory whose
  /// name `CW_API3D_PLUGIN_NAME` can override.
  ///
  /// The `PATH` prepend is the load-bearing half — `AddDllDirectory` only affects loads passing
  /// `LOAD_LIBRARY_SEARCH_*` flags, and this process never calls `SetDefaultDllDirectories`
  /// (which is process-wide and can break cadwork). See `docs/architecture/deploy-qtgraphs-runtime.md`
  /// §3.2 for the rejected alternatives.
  void ensureHostAbsentQtRuntimeVisible() noexcept;

  /// Test seam for the above; not intended for direct use in production wiring.
  void addDirectoryToDllSearchPath(const std::filesystem::path& dir);

} // namespace cw_api3d::composition
