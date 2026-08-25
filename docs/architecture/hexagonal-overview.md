# Hexagonal overview: project setup, data flow, coupling

Project-wide map of `cw_api3d` as a cadwork 3D plugin. Start with the D4 coupling contrast in [`README.md`](README.md). Feature decisions (aggregator policy, dock chrome, colour) live in [`dockable-model-statistics-charts.md`](dockable-model-statistics-charts.md) and [`statistics-chart-colour-and-layout.md`](statistics-chart-colour-and-layout.md). This file is the coupling and data-flow picture those docs assume.

**Colour.** Green = application + ports (no Qt, no CwAPI3D). Blue = driving adapters (Qt / QML). Amber = driven adapters (cadwork SDK / spdlog). Purple = composition root (the only place both sides are named). Red dashed = illegal dependency.

**Two arrow directions.** Runtime calls travel *out* through ports (use case → `IElementCatalog` → adapter). Compile-time dependencies travel *in* (adapter → port ← application). That inversion is the coupling advantage.

**No `src/domain/`.** Value types (`ElementSnapshot`, `StatisticSeries`) live in `src/application/`. Driven-port headers include those types; CMake stays acyclic (`cw_api3d_application` → `cw_api3d_ports`).

---

## D1. System context

```mermaid
flowchart TB
  modeller["Modeller"]
  host["cadwork 3D process<br/>QMainWindow + ControllerFactory"]
  plugin["cw_api3d.dll<br/>plugin_x64_init returns false — stays loaded"]
  qt["cadlib Qt 6.8.3<br/>not vcpkg"]
  sdk["CwAPI3D controllers<br/>element / attribute / geometry<br/>visualization / utility"]
  log["spdlog"]

  modeller -->|"Fetch, axis, chart, click-through"| host
  host -->|"plugin_x64_init / init_cwapi3d"| plugin
  plugin -->|"addDockWidget, reuse QCoreApplication"| host
  plugin --> qt
  plugin --> sdk
  plugin --> log

  classDef layerHost fill:#2a2a2a,stroke:#888,color:#eee
  classDef layerPlugin fill:#2d1a44,stroke:#c084fc,color:#f3e8ff
  classDef layerTech fill:#3d2a12,stroke:#f0b429,color:#fff6e5
  class modeller,host layerHost
  class plugin layerPlugin
  class qt,sdk,log layerTech
```

**What it shows.** The plugin is a guest DLL inside cadwork, not a standalone Qt application.

**Coupling rule.** Never construct `QApplication`. Reuse `QCoreApplication::instance()`; if it is null, skip UI.

**Where.** `src/composition/PluginEntry.cpp`, `src/composition/StatisticsPanelWiring.cpp`, `src/composition/HostAbsentQtRuntimeVisibility.cpp`.

---

## D2. Hexagonal layer map (runtime)

```mermaid
flowchart LR
  subgraph sgDriving["Driving adapters — Qt"]
    direction TB
    Qml["StatisticsPanel.qml"]
    Dock["StatisticsDockWidget"]
    VM["StatisticsViewModel"]
    Model["BucketListModel"]
    Pal["CategoricalPalette"]
    Qml --> Dock
    Dock --> VM
    VM --> Model
    Model --> Pal
  end

  subgraph sgCore["Application + ports — no Qt, no CwAPI3D"]
    direction TB
    Fetch["IFetchElementSnapshotUseCase"]
    Activate["IActivateElementsUseCase"]
    Path["IQueryPluginPathUseCase"]
    Agg["ElementStatisticsAggregator"]
    Snap["ElementSnapshot / StatisticSeries"]
    CatPort["IElementCatalog"]
    ActPort["IElementActivation"]
    UtilPort["IUtilityProvider"]
    LogPort["ILogger"]
    Fetch --> CatPort
    Fetch --> LogPort
    Activate --> ActPort
    Activate --> LogPort
    Path --> UtilPort
    Path --> LogPort
    Agg --> Snap
  end

  subgraph sgDriven["Driven adapters"]
    direction TB
    CatAd["CadworkElementCatalogAdapter"]
    ActAd["CadworkElementActivationAdapter"]
    UtilAd["CadworkUtilityAdapter"]
    LogAd["SpdLogLogger"]
  end

  subgraph sgComposition["Composition — wires both sides"]
    direction TB
    Entry["PluginEntry"]
    Boot["PluginBootstrapper"]
    Session["PluginUiSession"]
    Wiring["StatisticsPanelWiring<br/>ProductionGraph"]
    Panel["IStatisticsPanel"]
    Entry --> Boot
    Entry --> Session
    Session --> Wiring
    Wiring --> Panel
  end

  VM --> Fetch
  VM --> Activate
  VM --> Agg
  Boot --> Path
  Dock -.-> Panel
  CatPort --> CatAd
  ActPort --> ActAd
  UtilPort --> UtilAd
  LogPort --> LogAd
  Wiring -.-> CatAd
  Wiring -.-> ActAd
  Wiring -.-> VM
  Wiring -.-> Dock
  Boot -.-> UtilAd
  Boot -.-> LogAd

  classDef layerCore fill:#163a2e,stroke:#3dd68c,color:#e8f5ee
  classDef layerDriving fill:#1a2d4a,stroke:#6ea8fe,color:#e8f0ff
  classDef layerDriven fill:#3d2a12,stroke:#f0b429,color:#fff6e5
  classDef layerComposition fill:#2d1a44,stroke:#c084fc,color:#f3e8ff
  class Qml,Dock,VM,Model,Pal layerDriving
  class Fetch,Activate,Path,Agg,Snap,CatPort,ActPort,UtilPort,LogPort layerCore
  class CatAd,ActAd,UtilAd,LogAd layerDriven
  class Entry,Boot,Session,Wiring,Panel layerComposition
```

**What it shows.** Real types around the hexagon. `ElementStatisticsAggregator` is not a port: it has no I/O.

**Coupling rule.** Core names ports, never adapters. Adapters implement ports. Composition is the only module that constructs both.

**Where.** `src/ports/`, `src/application/`, `src/adapters/driving/statistics/`, `src/adapters/driven/`, `src/composition/`.

---

## D3. CMake target graph (compile-time coupling)

```mermaid
flowchart TB
  ports["cw_api3d_ports<br/>INTERFACE"]
  app["cw_api3d_application<br/>INTERFACE"]
  spd["cw_api3d_spdlog_adapter"]
  cad["cw_api3d_cadwork_adapter"]
  drv["cw_api3d_driving<br/>optional: skipped if CUSTOM_QT_PATH empty"]
  comp["cw_api3d_composition"]
  dll["cw_api3d.dll"]

  qt["Qt6 Core/Gui/Widgets/Qml/Quick/Graphs"]
  sdk["CwAPI3D::CwAPI3D"]
  spdlog["spdlog::spdlog"]

  app --> ports
  spd --> ports
  cad --> ports
  drv --> app
  drv --> ports
  drv --> qt
  cad --> sdk
  spd --> spdlog
  comp --> ports
  comp --> app
  comp --> spd
  comp --> cad
  dll --> comp
  dll -.->|"if TARGET cw_api3d_driving"| drv

  classDef layerCore fill:#163a2e,stroke:#3dd68c,color:#e8f5ee
  classDef layerDriving fill:#1a2d4a,stroke:#6ea8fe,color:#e8f0ff
  classDef layerDriven fill:#3d2a12,stroke:#f0b429,color:#fff6e5
  classDef layerComposition fill:#2d1a44,stroke:#c084fc,color:#f3e8ff
  classDef layerExt fill:#2a2a2a,stroke:#888,color:#eee
  class ports,app layerCore
  class drv layerDriving
  class spd,cad layerDriven
  class comp,dll layerComposition
  class qt,sdk,spdlog layerExt
```

Missing edges that a god-object plugin would have — and this tree does not:

| Illegal link | Why it is absent |
|--------------|------------------|
| `cw_api3d_application` → Qt | aggregator and use cases stay header-only C++23 |
| `cw_api3d_application` → `CwAPI3D` | host types never enter the core |
| `cw_api3d_driving` → `cw_api3d_cadwork_adapter` | QML/ViewModel talk to use cases, not the SDK |
| `cw_api3d_cadwork_adapter` → Qt | driven side does not know about the dock |
| `cw_api3d_composition` → Qt | Qt enters the DLL only through optional `cw_api3d_driving` |

**What it shows.** `target_link_libraries` is the architecture, not a comment.

**Coupling rule.** A CwAPI3D header change does not rebuild application. A QML/Qt change does not rebuild cadwork adapters.

**Where.** `src/*/CMakeLists.txt`, root `CMakeLists.txt`. Driving: `src/adapters/driving/CMakeLists.txt` early-returns when `CUSTOM_QT_PATH` is empty.

---

## D4. Coupling contrast — same feature, two shapes

```mermaid
flowchart TB
  subgraph coupled["A. Tightly coupled plugin — what this repo avoids"]
    direction LR
    cQml["QML"] --> cVM["ViewModel"]
    cVM --> cEl["ICwAPI3DElementController"]
    cVM --> cViz["ICwAPI3DVisualizationController"]
    cVM --> cLog["spdlog"]
    cVM --> cColor["QColor / Qt Graphs"]
    cAgg["Aggregator"] --> cEl
  end

  subgraph hex["B. This repo — ports invert the dependencies"]
    direction LR
    hQml["QML"] --> hVM["StatisticsViewModel"]
    hVM --> hFetch["IFetch / IActivate"]
    hVM --> hAgg["ElementStatisticsAggregator<br/>pure, no I/O"]
    hFetch --> hPorts["IElementCatalog<br/>IElementActivation<br/>ILogger"]
    hAd["Cadwork*Adapter"] --> hPorts
    hFake["Fake* in tests"] --> hPorts
  end

  classDef layerBad fill:#4a1c1c,stroke:#e74c3c,color:#fdecea
  classDef layerCore fill:#163a2e,stroke:#3dd68c,color:#e8f5ee
  classDef layerDriving fill:#1a2d4a,stroke:#6ea8fe,color:#e8f0ff
  classDef layerDriven fill:#3d2a12,stroke:#f0b429,color:#fff6e5
  class cQml,cVM,cEl,cViz,cLog,cColor,cAgg layerBad
  class hQml,hVM layerDriving
  class hFetch,hAgg,hPorts layerCore
  class hAd,hFake layerDriven
```

In **A**, UI, host, logging, and charting share includes. Replacing cadwork or unit-testing the aggregator requires the SDK.

In **B**, fan-in is at the ports. `CadworkElementCatalogAdapter` and `FakeElementCatalog` are interchangeable behind `IElementCatalog`. QML never names a controller. The aggregator never names Qt.

**Coupling rule.** Depend on abstractions (`IElementCatalog`, `IFetchElementSnapshotUseCase`), not on cadwork or Qt. Composition, not the view, picks the implementation.

---

## D5. Change blast radius

```mermaid
flowchart TB
  subgraph sdkChange["Change: new CwAPI3D version / extra controller"]
    direction LR
    sCad["HIT: driven/cadwork + composition wiring"]
    sCore["untouched: application + ports"]
    sDrv["untouched: driving / QML"]
    sCad -.->|"does not force rebuild"| sCore
    sCad -.->|"does not force rebuild"| sDrv
  end

  subgraph uiChange["Change: new chart kind / colour palette"]
    direction LR
    uDrv["HIT: CategoricalPalette, BucketListModel, QML"]
    uCore["untouched: application + ports<br/>StatisticBucket has no colour field"]
    uCad["untouched: driven/cadwork"]
    uDrv -.->|"does not leak QColor into"| uCore
    uDrv -.->|"does not touch"| uCad
  end

  subgraph testChange["Change: unit-test aggregator or fetch"]
    direction LR
    tApp["HIT: cw_api3d_application + Fake*"]
    tHost["NOT REQUIRED: cadwork process / SDK mocks"]
    tApp -.->|"no host needed"| tHost
  end

  classDef layerHit fill:#4a1c1c,stroke:#e74c3c,color:#fdecea
  classDef layerSafe fill:#163a2e,stroke:#3dd68c,color:#e8f5ee
  classDef layerSkip fill:#2a2a2a,stroke:#555,color:#888
  class sCad,uDrv,tApp layerHit
  class sCore,sDrv,uCore,uCad layerSafe
  class tHost layerSkip
```

| Change | Hexagonal blast | Coupled blast |
|--------|-----------------|---------------|
| New cadwork SDK | `src/adapters/driven/cadwork/` + `StatisticsPanelWiring.cpp` | every file that included `<cwapi3d/...>` |
| New chart kind / palette | `src/adapters/driving/statistics/` | risk of `QColor` leaking into the aggregator |
| Unit-test aggregator / fetch | `cw_api3d_application` + `tests/doubles/` (no Qt, no SDK) | host process or SDK mocks |

Red is the blast. Green is untouched. Grey is a dependency the coupled design would have required and this one does not.

**What it shows.** The same three changes, with churn confined to the adapter that owns the technology.

**Coupling rule.** Dependency inversion localizes rebuilds and test setup; a coupled plugin would paint every box red.

**Where.** Contrast `src/adapters/driven/cadwork/` vs `src/application/` vs `src/adapters/driving/statistics/`.

---

## D6. Allowed vs forbidden include edges

Allowed (these compile today):

```mermaid
flowchart LR
  App["src/application/"]
  Ports["src/ports/"]
  Driving["src/adapters/driving/"]
  Driven["src/adapters/driven/"]
  Comp["src/composition/"]
  Qt["Qt headers"]
  Sdk["cwapi3d headers"]
  Tests["tests/application + tests/doubles"]

  Driving --> App
  Driving --> Ports
  App --> Ports
  Driven --> Ports
  Comp --> App
  Comp --> Ports
  Comp --> Driven
  Comp --> Driving
  Tests --> App
  Tests --> Ports
  Driving --> Qt
  Driven --> Sdk

  classDef layerCore fill:#163a2e,stroke:#3dd68c,color:#e8f5ee
  classDef layerDriving fill:#1a2d4a,stroke:#6ea8fe,color:#e8f0ff
  classDef layerDriven fill:#3d2a12,stroke:#f0b429,color:#fff6e5
  classDef layerComposition fill:#2d1a44,stroke:#c084fc,color:#f3e8ff
  classDef layerExt fill:#2a2a2a,stroke:#888,color:#eee
  class App,Ports,Tests layerCore
  class Driving layerDriving
  class Driven layerDriven
  class Comp layerComposition
  class Qt,Sdk layerExt
```

Forbidden (CMake and include discipline reject these):

```mermaid
flowchart LR
  App["src/application/"]
  Ports["src/ports/"]
  Driving["src/adapters/driving/"]
  Driven["src/adapters/driven/"]
  Qt["Qt headers"]
  Sdk["cwapi3d headers"]

  App -.->|"illegal"| Qt
  App -.->|"illegal"| Sdk
  Ports -.->|"illegal"| Qt
  Ports -.->|"illegal"| Sdk
  Driving -.->|"illegal"| Driven
  Driven -.->|"illegal"| Qt
  Driven -.->|"illegal"| Driving

  classDef layerBad fill:#4a1c1c,stroke:#e74c3c,color:#fdecea
  class App,Ports,Driving,Driven,Qt,Sdk layerBad
```

**What it shows.** Hexagonal layering is a *link* rule, not a folder naming scheme.

**Coupling rule.** Driving must not include driven (or the UI is welded to cadwork). Application must not include Qt or CwAPI3D (or tests cannot run without the host).

**Where.** Enforced by CMake `target_link_libraries` (D3). `src/ports` and `src/application` must not `#include` Qt.

---

## D7. Bootstrap sequence

```mermaid
sequenceDiagram
  actor Host as cadwork 3D
  participant Entry as PluginEntry
  participant Boot as PluginBootstrapper
  participant Path as QueryPluginPathUseCase
  participant Session as PluginUiSession
  participant Wiring as StatisticsPanelWiring
  participant Dock as StatisticsDockWidget

  Host->>Entry: plugin_x64_init with ControllerFactory
  Entry->>Session: registerStatisticsPanelFactory if CW_API3D_HAS_DRIVING
  Entry->>Boot: bootstrapPlugin factory
  Boot->>Boot: SpdLogLogger plus CadworkUtilityAdapter
  Boot->>Path: execute
  Path-->>Boot: plugin path or unexpected
  Boot->>Session: setLogger plus showOrFocus factory
  Session->>Wiring: PanelFactory factory
  alt no QCoreApplication, HWND, or factory
    Wiring-->>Session: nullptr - skip UI, plugin stays loaded
  else
    Wiring->>Wiring: ensureGraph catalog, activation, use cases, ViewModel
    Wiring->>Dock: new StatisticsDockWidget with ViewModel and host QMainWindow
    Dock-->>Session: IStatisticsPanel pointer
    Session->>Dock: showOrFocus
  end
  Entry-->>Host: false - stay loaded, no host modal
```

**What it shows.** C exports catch all exceptions. UI construction failure logs and skips the dock; the DLL stays resident.

**Coupling rule.** `PluginEntry.cpp` is the host boundary (`noexcept`, catch-all). Only composition sees `ControllerFactory` and `QMainWindow` together.

**Where.** `src/composition/PluginEntry.cpp`, `Bootstrapping.cpp`, `PluginUiSession.cpp`, `StatisticsPanelWiring.cpp`.

---

## D8. Fetch data flow

```mermaid
sequenceDiagram
  actor User
  participant Qml as StatisticsPanel.qml
  participant VM as StatisticsViewModel
  participant Fetch as FetchElementSnapshotUseCase
  participant Cat as IElementCatalog
  participant Ad as CadworkElementCatalogAdapter
  participant Host as CwAPI3D element and attribute and geometry
  participant Agg as ElementStatisticsAggregator
  participant Model as BucketListModel

  User->>Qml: Fetch
  Qml->>VM: fetch
  Note over VM: set busy, defer one event-loop tick so QML paints a frame
  VM->>Fetch: execute universe
  Fetch->>Cat: fetch universe
  Cat->>Ad: production implementation
  Ad->>Host: getActive or getAll identifiable IDs
  Note over Ad,Host: copy count at narrowData - never destroy
  Host-->>Ad: host lists and strings
  Ad-->>Fetch: ElementSnapshot values
  alt catalog error
    Fetch-->>VM: unexpected string
    VM-->>Qml: errorMessage
  else
    Fetch-->>VM: ElementSnapshot
    VM->>Agg: aggregate snapshot by axis
    Agg-->>VM: StatisticSeries
    VM->>Model: setBuckets labels counts colours memberIds
    Model-->>Qml: GraphsView or table fallback
  end
```

Value types that cross the hexagon: `ElementSnapshot` / `ElementRecord` (copied IDs, strings, `ElementKind`, optional dimensions) and `std::expected<T, std::string>`.

**What it shows.** The host walk is behind `IElementCatalog`. QML never names an element controller.

**Coupling rule.** Host lifetime stays on the cadwork heap. The core receives values, so it can be tested with `FakeElementCatalog`.

**Where.** `StatisticsViewModel.cpp` (`fetch` / `runFetch`), `FetchElementSnapshotUseCase.h`, `CadworkElementCatalogAdapter`.

---

## D9. Axis / chart change — no host I/O

```mermaid
sequenceDiagram
  actor User
  participant Qml as StatisticsPanel.qml
  participant VM as StatisticsViewModel
  participant Agg as ElementStatisticsAggregator
  participant Model as BucketListModel

  alt axis change
    User->>Qml: setAxis
    Qml->>VM: setAxis
    VM->>Agg: aggregate cached snapshot by new axis
    Agg-->>VM: StatisticSeries
    VM->>Model: setBuckets
  else chart kind change
    User->>Qml: "setChartKind Bar / Pie"
    Qml->>VM: setChartKind
    Note over Qml,VM: "view concern only - aggregator is not called"
  end
  Model-->>Qml: "rebind GraphsView / table"
```

**What it shows.** A cached `ElementSnapshot` is re-sliced in process. Chart kind (`Bar` / `Pie`) never leaves the driving adapter.

**Coupling rule.** Aggregation is not a port because it has no I/O. Keeping it in application means the UI can re-slice without coupling to cadwork.

**Where.** `StatisticsViewModel::reaggregate`, `ElementStatisticsAggregator`, `StatisticAxis` vs ViewModel `ChartKind`.

---

## D10. Click-through activate

```mermaid
sequenceDiagram
  actor User
  participant Qml as QML / table
  participant VM as StatisticsViewModel
  participant Model as BucketListModel
  participant Act as ActivateElementsUseCase
  participant Port as IElementActivation
  participant Ad as CadworkElementActivationAdapter
  participant Viz as ICwAPI3DVisualizationController

  User->>Qml: click bar or slice or row
  Qml->>VM: selectBucket index
  VM->>Model: memberIdsAt index
  Model-->>VM: span of ElementId
  VM->>Act: execute ids
  Act->>Port: activate ids
  Port->>Ad: production implementation
  Ad->>Viz: createEmptyElementIDList, append, setActive
  Note over Ad,Viz: never destroy the list
  alt host error
    Act-->>VM: "unexpected string"
    VM-->>Qml: "errorMessage - cadwork stays up"
  else
    Viz-->>User: "those elements become active in 3D"
  end
```

**What it shows.** Member IDs travelled with the snapshot through the core. The visualization controller is not visible to QML or the aggregator.

**Coupling rule.** Click-through is one use-case call. Host list construction stays in the driven adapter.

**Where.** `StatisticsViewModel::selectBucket`, `ActivateElementsUseCase.h`, `CadworkElementActivationAdapter`.

---

## D11. Test substitution — the coupling payoff

```mermaid
flowchart LR
  subgraph sgSameCore["Same application — unchanged"]
    Fetch["FetchElementSnapshotUseCase"]
    Activate["ActivateElementsUseCase"]
    Path["QueryPluginPathUseCase"]
    Agg["ElementStatisticsAggregator"]
    CatPort["IElementCatalog"]
    ActPort["IElementActivation"]
    UtilPort["IUtilityProvider"]
    LogPort["ILogger"]
    Fetch --> CatPort
    Fetch --> LogPort
    Activate --> ActPort
    Activate --> LogPort
    Path --> UtilPort
    Path --> LogPort
  end

  subgraph sgProduction["Production — composition"]
    CatAd["CadworkElementCatalogAdapter"]
    ActAd["CadworkElementActivationAdapter"]
    UtilAd["CadworkUtilityAdapter"]
    LogAd["SpdLogLogger"]
    Dock["StatisticsDockWidget"]
  end

  subgraph sgTests["Tests — doubles, no host"]
    FakeCat["FakeElementCatalog"]
    FakeAct["FakeElementActivation"]
    FakeUtil["FakeUtilityProvider"]
    FakeLog["FakeLogger"]
    FakePanel["FakeStatisticsPanel"]
  end

  CatPort --> CatAd
  CatPort --> FakeCat
  ActPort --> ActAd
  ActPort --> FakeAct
  UtilPort --> UtilAd
  UtilPort --> FakeUtil
  LogPort --> LogAd
  LogPort --> FakeLog
  Panel["IStatisticsPanel"]
  Dock --> Panel
  FakePanel --> Panel

  classDef layerCore fill:#163a2e,stroke:#3dd68c,color:#e8f5ee
  classDef layerDriven fill:#3d2a12,stroke:#f0b429,color:#fff6e5
  classDef layerTest fill:#1a2d4a,stroke:#6ea8fe,color:#e8f0ff
  classDef layerComposition fill:#2d1a44,stroke:#c084fc,color:#f3e8ff
  class Fetch,Activate,Path,Agg,CatPort,ActPort,UtilPort,LogPort layerCore
  class CatAd,ActAd,UtilAd,LogAd,Dock layerDriven
  class FakeCat,FakeAct,FakeUtil,FakeLog,FakePanel layerTest
  class Panel layerComposition
```

| Production | Test double | Who uses it |
|------------|-------------|-------------|
| `CadworkElementCatalogAdapter` | `FakeElementCatalog` | `FetchElementSnapshotUseCaseTests` |
| `CadworkElementActivationAdapter` | `FakeElementActivation` | `ActivateElementsUseCaseTests` |
| `SpdLogLogger` | `FakeLogger` | use case + composition tests |
| `CadworkUtilityAdapter` | `FakeUtilityProvider` | path use case / `CompositionRootTests` |
| `StatisticsDockWidget` | `FakeStatisticsPanel` | `CompositionRootTests` — no `QMainWindow` |
| real fetch/activate use cases | fake use cases | `StatisticsViewModelTests` (Qt allowed only on that target) |

Application tests link `cw_api3d_application` + `cw_api3d_ports` only. Adapter tests use concept-sized `FakeHost*` so the template adapter compiles without the SDK. Composition tests inject `IStatisticsPanel` so the session's idempotence does not construct a dock.

**What it shows.** Ports are the test seam. The core under test is the same binary shape as production.

**Coupling rule.** If a unit test needs cadwork or Qt to exercise application logic, a port is missing or an adapter leaked inward.

**Where.** `tests/doubles/`, `tests/application/`, `tests/composition/CompositionRootTests.cpp`, `tests/CMakeLists.txt`.

---

## How to read the coupling advantage

Narrative D4 walkthrough (tight vs hex for fetch, re-slice, click-through): [`README.md`](README.md).

1. **D4 + D6** — what would be welded together without ports.
2. **D3** — CMake already forbids those welds.
3. **D8–D10** — data that *does* cross the boundary is copied values, not host objects.
4. **D11** — the same ports that isolate cadwork also isolate tests.

If a new feature needs the host, add a driven port and an adapter. If it needs the UI, add a driving adapter that calls an existing use case (or a new one). Do not include `<cwapi3d/...>` or Qt from `src/application/` or `src/ports/`.
