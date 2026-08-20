# cw-api3d

C++ CAD plugin and SDK integration library for cadwork 3D, implementing Ports & Adapters (Hexagonal) architecture.

## Structure

- `src/ports/` — Port interfaces (`ILogger`, `IUtilityProvider`) and C++20 concepts (`concepts::Logger`, `concepts::UtilityProvider`).
- `src/application/` — Application use cases (`IQueryPluginPathUseCase`, `QueryPluginPathUseCase`) orchestrating core logic via driven port interfaces.
- `src/adapters/driven/` — Driven adapters:
  - `logging/` (`SpdLogLogger` wrapping spdlog).
  - `cadwork/` (`CadworkUtilityAdapter` wrapping `ICwAPI3DUtilityController`).
- `src/composition/` — Composition root (`PluginBootstrapper`) and C-safe DLL plugin entry point (`PluginEntry.cpp`, `bootstrapPlugin`).
- `cmake/` — CMake build logic and post-build automated plugin deployment (`cadwork_deploy.cmake`).
- `tests/` — Test suites:
  - `tests/ports/`, `tests/application/`, `tests/adapters/`, `tests/composition/` — C++ GoogleTest unit/integration tests with test doubles (`tests/doubles/`).
  - `tests/e2e/` — Python end-to-end test harness running inside live cadwork 3D.

---

## Testing

For detailed instructions on building and running the test suites, see [tests/README.md](tests/README.md).

### Quick Commands

#### 1. C++ Unit Tests (CTest)
```powershell
cmd.exe /c "call ""C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat"" -vcvars_ver=14.44 && cmake --preset local-relwithdebinfo && cmake --build out/build/local-relwithdebinfo && ctest --test-dir out/build/local-relwithdebinfo --output-on-failure"
```

#### 2. Python E2E Tests (uv / pytest)
```bash
# Sync environment & run tests
uv sync
uv run pytest -v
```

---

## Parallel Worktrees

`new-worktree.cmd -Key <name>` creates a ready-to-build worktree at `D:\wt\cw-api3d\<name>` on
branch `wt-<name>`. It carries over what git never does — the gitignored `CMakeUserPresets.json`
and the git-excluded `CLAUDE.md` / `AGENTS.md` — and gives the worktree its **own** cadwork
userprofil (`D:\cadwork\userprofil_<year>_<name>`), so parallel worktrees no longer overwrite each
other's `cw_api3d.dll` and a build no longer fails because a running cadwork holds it open.

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
