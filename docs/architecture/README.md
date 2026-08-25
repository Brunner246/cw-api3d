# Architecture

`cw-api3d` is a teaching monorepo for cadwork 3D plugins. The **kit** is the hexagonal starter (load, log, query plugin path). Each **example** is its own plugin DLL that links the kit and adds one teaching point.

Configure the repo root (`cmake --preset local-relwithdebinfo`). CLion opens the root and you pick a DLL target.

| Layer | Path | Artefact |
|---|---|---|
| Kit | `kit/` | static/interface libraries (`cw_api3d_ports`, `cw_api3d_composition`, …) — no DLL |
| Hello | `examples/hello-plugin/` | `cw_api3d_hello.dll` → `API.x64/cw_api3d_hello/` |
| Charts | `examples/charts/` | `cw_api3d_charts.dll` → `API.x64/cw_api3d_charts/` |

**Coupling rule.** Kit application and ports never include Qt or CwAPI3D. Composition (kit or example) is the only module that names both sides. Example ports (`IElementCatalog`, `IElementActivation`) live with the example that owns their snapshot types — they are not kit.

Charts hexagonal map and D4 coupling contrast: [`examples/charts/docs/architecture/`](../../examples/charts/docs/architecture/README.md).
