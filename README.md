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
