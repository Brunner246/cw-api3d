# cw-api3d

Dockable model statistics for cadwork 3D — a C++ plugin built on Ports & Adapters (Hexagonal) architecture.

The plugin loads inside cadwork, reads the element catalogue through a driven port, aggregates a snapshot into chart series, and shows them in a dockable Qt panel. It builds one DLL: `cw_api3d_charts.dll`, deployed to `API.x64/cw_api3d_charts/`.

## Structure

Architecture: [docs/architecture/README.md](docs/architecture/README.md).

- `src/ports/` — `ILogger`, `IUtilityProvider`, `IElementCatalog`, `IElementActivation` — plain virtual interfaces, no parallel concepts.
- `src/application/` — `QueryPluginPathUseCase`, `FetchElementSnapshotUseCase`, `ActivateElementsUseCase`, `ElementStatisticsAggregator`. No Qt, no CwAPI3D.
- `src/adapters/driven/` — `SpdLogLogger`; `CadworkUtilityAdapter`, `CadworkElementCatalogAdapter`, `CadworkElementActivationAdapter`.
- `src/adapters/driving/statistics/` — Qt view models, the dock widget, and `StatisticsPanel.qml`.
- `src/composition/` — `PluginBootstrapper` / `bootstrapPlugin` (no UI), `bootstrapChartsPlugin`, `PluginUiSession`, and the `PluginEntry.cpp` host exports.
- `tests/` — GoogleTest suites with doubles under `tests/doubles/`.
- `tests/e2e/` — Python end-to-end harness inside live cadwork 3D. Override the plugin name with `CW_API3D_PLUGIN_NAME` if you rename the target.
- `cmake/` — Shared CMake and post-build deploy (`cadwork_deploy.cmake`). Deploy folder is `API.x64/<target_name>/`.

---

## Testing

For detailed instructions on building and running the test suites, see [tests/README.md](tests/README.md).

### Quick Commands

Invoke wraps CMake / CTest / pytest and loads MSVC toolset 14.44 automatically. Default preset is `local-relwithdebinfo`.

```powershell
uv sync
uv run invoke --list
uv run invoke build
uv run invoke test
uv run invoke e2e
```

```powershell
uv run invoke build --preset local-debug
uv run invoke test --filter CompositionRoot
uv run invoke e2e --host-only          # path/staging tests, no cadwork launch
uv run invoke e2e --no-build           # skip the C++ rebuild
uv run invoke e2e --args "-k test_cadwork_paths_resolution"
uv run invoke rebuild
```

See [tests/README.md](tests/README.md) for the underlying cmake / ctest / pytest commands.

---

## Parallel Worktrees

Worktrees are a local workflow (short path, private cadwork userprofil). `new-worktree.cmd -Key
<name>` creates a ready-to-build checkout at `D:\wt\cw-api3d\<name>` on branch `wt-<name>`. It
carries over what git never does — the gitignored `CMakeUserPresets.json` and the git-excluded
`CLAUDE.md` / `AGENTS.md` — and gives the worktree its **own** cadwork userprofil
(`D:\cadwork\userprofil_<year>_<name>`), so two checkouts do not overwrite each other's DLL and a
build no longer fails because a running cadwork holds it open.

```powershell
.\new-worktree.cmd -Key feature-x
.\new-worktree.cmd -Key feature-x -Base origin/main -Root D:\wt\cw-api3d
```

The profile path is recorded in the worktree's gitignored `.cw-userprofile` marker, which both
consumers read, so there is no environment variable to remember:

| Reader | Precedence |
|---|---|
| `cmake/cadwork_deploy.cmake` (post-build deploy) | marker → `CADWORK_USERPROFILE_DIR` → `CADWORK_USP`/`CISTART_USP` env → registry |
| `tests/e2e/cadwork_paths.py` (`userprofil_dir()`) | `CADWORK_USP` env → marker → registry |

An existing checkout gets its own profile the same way, without a worktree:

```powershell
.\build-scripts\new-local-profile.ps1 -Name <name>
```

Nothing here touches cadwork's machine-wide active profile (`CADWORK_USP` in the registry) — the
E2E harness passes the marker's profile to the cadwork it launches via the environment. Pass
`-NoLocalProfile` to skip the private profile and deploy into the machine-wide one.

Removing a worktree: `git worktree remove D:\wt\cw-api3d\<name>` (its profile directory under
`D:\cadwork` is left behind — delete it by hand).
