# Game Engine — Implementation Plan

## Architecture Summary

| Decision | Choice |
|---|---|
| Language | C++23, Windows-only |
| Build system | CMake 3.28+ (use VS-bundled cmake at `C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe` — system cmake in PATH is outdated at 3.11) |
| IDE | Visual Studio 2022 Community 17.14 |
| Process model | Single editor process; game runs in-process via a loaded game DLL |
| Object model | GameObject / Component (no ECS) |
| Custom components | C++ DLL, loaded and hot-reloaded by the editor |
| Source organisation | C++20 modules (`.ixx` interface units) for all engine-owned code; `#include` for third-party headers via the module global fragment |
| engine-core linkage | **Shared DLL** (`engine-core.dll`) — editor and game DLL both link against it so engine state (registries, allocators, type system) is never duplicated |
| Binary output | All `.exe` and `.dll` outputs land in `build/bin/<Config>/` via `CMAKE_RUNTIME_OUTPUT_DIRECTORY` |
| Serialization | Custom text format |
| game-template | **DLL** (not exe) — exposes game lifecycle hooks (`GameInitialize`, `GameUpdate`, `GameShutdown`) and component registration; editor loads it at runtime |
| Reflection | Manual field registration (macros) |
| Namespace | All engine-owned code lives in `namespace Engine` |
| Renderer | Abstraction layer only for MVP (backend TBD) |

## Progress

| Task | Status | Notes |
|---|---|---|
| P1-01 — CMake root project layout | ✅ Done | Project at `D:\CppProjects\Engine\`. Solution generated at `build\Engine.sln`. Three targets: `engine-core` (shared DLL), `editor` (exe), `game-template` (DLL — game lifecycle hooks + component registration; loaded by the editor at runtime). |
| P1-02 — CMake build configuration | ✅ Done | `cmake/CompilerOptions.cmake` with `apply_compiler_options()` function; `/W4`, `/permissive-`, per-config `/Od`+`/RTC1` (Debug) and `/O2` (Release/RelWithDebInfo), `/O1` (MinSizeRel). Applied to all three targets. |
| P1-03 — Logger | ✅ Done | `LogManager`, `ConsoleLogSink` (ANSI colours per level), `FileLogSink` (appends to `EngineLog.txt`), `Log<>` template with `std::source_location` + `std::format`, configurable `minLogLevel`, DLL-exported via `ENGINE_CORE_API`. Convenience macros (`LOG_TRACE/DEBUG/INFO/WARN/ERROR`) in `engine-core/include/LoggerMacros.h` — include alongside `import Logger;`. |
| P1-04 — Core utility types | ⚠️ Partial | **Done:** `EngineError` (type + message, DLL-exported constructor) and `Expected<T>` alias over `std::expected<T, EngineError>` in `EngineError.ixx`; `export import`-ed from `EngineCore.ixx` so `import EngineCore;` exposes them transitively. **Remaining:** (1) string helpers needed by the serialiser (trim, split, case); (2) file I/O helpers (`ReadAllText`, `WriteAllText`) returning `Expected<T>` — defer until just before P3. |
| P1-05 — Math types: Vec2, Vec3, Vec4 | ✅ Done | `Math.ixx` exports `Vec2`, `Vec3`, `Vec4`: arithmetic operators (+, -, *, /), compound-assignment forms, unary negation, commutative `float * Vec` free functions, `Dot`, `Cross` (Vec3 only), `SqrMagnitude`, `Length`, `Normalize`/`NormalizeSafe`/`Normalized`/`NormalizedSafe` (with fallback), `Lerp` free functions (unclamped), swizzle accessors (`Vec3`: xy/xz/yz; `Vec4`: xyz/xy/zw), direction constants (Up/Down/Left/Right/Forward/Back), and `std::formatter` specializations for all three. `export import`-ed from `EngineCore.ixx`. |
| P1-06 — Mat4 and Quaternion | ✅ Done | In `Math.ixx`. **Mat4x4**: identity, multiply, transpose, inverse (cofactor-expansion, asserts non-zero det), `Scale`/`Translate`/`TRS` factory methods, `FromQuaternion`, `operator*(Mat4x4, Vec4)`. **Mat3x3**: identity, multiply, transpose, row/col accessors, `At`, `operator*(Mat3x3, Vec3)`. **Quaternion**: `Euler` (two overloads, `RotateOrder` enum for all 6 orderings), `Slerp` (falls back to lerp near-parallel), `AngleAxis`, `LookRotation`, `FromTo` (relative rotation), `Inverse` (conjugate, unit-quat only), `Magnitude`, `operator*` (Hamilton product), `operator-` (relative rotation), scalar `operator*`, `operator/=`, `operator+=`, construction from `Mat3x3`. `Mat4x4::TRS` composes all three. |
| P1-07 — Transform | ✅ Done | `Transform.ixx` exports `Transform : Component` with local position/rotation/scale. `_localToWorld` matrix removed — Transform now owns only `int _sceneNodeIndex` (index into the active scene's local-transform array). `RefreshLocalToWorld()` builds the matrix and calls `IScene::SetLocalTransform`. `AddToScene(IScene&, name)` registers the node and stores the returned index. `OnSceneNodeIndexChanged(int)` override keeps the index in sync if the scene ever reassigns it. |
| P2-01 — Component base class | ✅ Done | `Component.ixx` exports `Component` (DLL-exported). Virtual lifecycle hooks: `OnCreate()`, `OnUpdate(float dt)`, `OnDestroy()`, `OnSceneNodeIndexChanged(int)`; all defaulted no-ops. `GetOwner()` returns `GameObject*` (forward-declared). `IsEnabled()`/`SetEnabled(bool)` with `_enabled = true` default. `_owner` is private, set exclusively by `friend class GameObject`. `export import`-ed from `EngineCore.ixx`. |
| P2-02 — Field registration macros and FieldDescriptor | ✅ Done |`Reflection.ixx` exports `FieldType` enum (Float, Int, Bool, String, Vec2, Vec3, Vec4, Quaternion) and `FieldDescriptor` (name, type, byte offset). `ReflectionMacros.h` companion header provides: `FIELD(...)` (passive no-op annotation), `COMPONENT_BODY(ClassName)` (declares static `GetTypeName`, `GetFieldDescriptors`, and virtual `GetReflectedFields` override), `REFLECTED_FIELDS(ClassName, ...)` and `REFLECTED_FIELDS_EMPTY(ClassName)` for use in `.reflected.h` seam files. `Component` gains a virtual `GetReflectedFields()` base returning an empty span. `export import`-ed from `EngineCore.ixx`. |
| P2-03 (prereq) — GUID | ✅ Done | `GUID.ixx` exports `Engine::GUID` (64-bit unsigned int, generated via `thread_local` `mt19937_64` seeded from `random_device`). `Invalid()` static factory returns `GUID(0)`; `IsValid()` checks for non-zero. `operator unsigned long long()` is `explicit` to prevent accidental arithmetic. `operator!=` omitted (synthesized from `operator==` in C++23). `std::hash<Engine::GUID>` and `std::formatter<Engine::GUID>` specializations defined (reachable to importers without needing `export`). `export import`-ed from `EngineCore.ixx`. |
| P2-03 design — GetComponent&lt;T&gt;() type identity | 📝 Decided | **Chosen approach: custom static type ID (no RTTI).** Each component class exposes a static `StaticTypeId()` (returning a unique `size_t` derived from the address of a local static) and a virtual `GetTypeId() const override` that delegates to it. `COMPONENT_BODY` macro will emit both. `GetComponent<T>()` compares `comp->GetTypeId() == T::StaticTypeId()` and `static_cast`s on match — zero RTTI, no exceptions. Template is constrained with a C++20 concept (`IsComponent`) requiring `std::derived_from<T, Component>` and a valid `T::StaticTypeId()`. |
| EngineInstance — DLL/linker fixes | ✅ Done | Moved constructor and destructor out of the class body (non-inline) to suppress C4251 on the private `std::unique_ptr<IComponentStorage>` member. Out-of-class `= default` caused duplicate-symbol linker errors (MSVC module quirk); replaced with explicit `{}` bodies. Remaining C4251 on the `unique_ptr` member itself is a known MSVC false positive when both types are `ENGINE_CORE_API` in the same DLL; options are pragma suppression on that line, raw pointer + manual delete, or pimpl. |
| P2-04 — Transform as built-in Component | ✅ Done | `Transform` is added as a mandatory component in `GameObject::GameObject()`. The constructor calls `AddComponent<Transform>()` then immediately `_transform->AddToScene(Instance.GetActiveScene(), _name)` to register a scene node. `ComponentStorage::CreateComponentImpl` now calls `rawPtr->OnCreate()` after construction so the lifecycle hook is wired. |
| P2-05 — Scene class + data-oriented hierarchy | ✅ Done | `Scene.ixx` (inside `namespace Engine::Scene`) implements `Scene` (no longer inheriting `IScene`) via a pimpl. `SceneImpl` was extracted into its own module `SceneImpl.ixx` to allow `Scene.ixx` to stay lean. Parallel flat arrays: `_nodes`, `_hierarchy` (parent/firstChild/firstSibling/depth/isDirty), `_localTransforms`, `_globalTransforms`, `_nodeNames`, `_nodeTransformComponents`. `_nodeTransformComponents` stores `Component*` (not `Transform*`) to avoid a cyclic module dependency between `SceneImpl` and `Transform`; `UpdateNodeIndex` calls `OnSceneNodeIndexChanged` via virtual dispatch. `AddNode` appends to all arrays and links into the sibling-list hierarchy. `SetParent` unlinks from the old parent chain, re-links under the new parent, and recursively updates depths. `IScene` is a separate module (`IScene.ixx`) still imported by `Transform` (for `SetLocalTransform` / `AddNode`). `EngineInstance` holds a non-owning `Scene::Scene*` (ownership TBD) and exposes `GetActiveScene()`. |
| P2-06 — Scene graph traversal | ✅ Done | `SceneImpl` provides `WalkDepthFirst` (recursive, pre-order) and `WalkBreadthFirst` (iterative, using a persistent `std::deque<int> _walkHelperQueue` to reuse heap allocation). Both take `std::function<void(std::size_t)>` — index only, no `Scene&` in the impl. `Scene::WalkDepthFirst/BreadthFirst` wrap the caller's `std::function<void(Scene&, std::size_t)>` in a forwarding lambda that captures `*this`, bridging the two signatures cleanly. `UpdateWorldTransforms` walks depth-first and recomputes `_globalTransforms[i] = _globalTransforms[parent] * _localTransforms[i]` for dirty nodes; root node (parent == -1) uses its local transform directly. |
| P3-01 to P3-04 — Custom text serialization format | ✅ Done | `DataFile.ixx` implements the full serialization stack. Format: INI/TOML-inspired; named nodes delimited by `[name]` / `{` / `}`, leaf values as comma-separated strings. `DataFile::Serialize` writes to file; `DataFile::FromStream` / `FromString` / `Deserialize` parse back to a tree. `operator[]` (non-const) creates children on first access; const overload + `at()` throw on missing keys; `HasChild` guards safe lookup. `StringViewHash` / `StringViewEq` with `is_transparent` enable zero-allocation heterogeneous lookup in the `_childIndexByName` map. `ISerializable` interface (`ToDataFile` / `FromDataFile`) provided for types that prefer self-contained serialization. `export import`-ed from `EngineCore.ixx`. |
| P2-02 extension — Composite field support | ✅ Done | `Reflection.ixx` extended: `FieldType::Composite` added to the enum; `GetDescriptorsFn` alias (`std::span<const FieldDescriptor>(*)()`) introduced; `FieldDescriptor` gains a `getChildren` function-pointer field (defaults to `nullptr`). Composite descriptors point to the child type's static `GetFieldDescriptors()`, enabling recursive serialization without any virtual dispatch or inheritance on field types. |
| P3-05 & P3-06 — Component ↔ DataFile via FieldDescriptors | ✅ Done | `Serialization.ixx` (new module) exports `SerializeFields(const void* base, span<FieldDescriptor>, DataFile&)` and `DeserializeFields(void* base, span<FieldDescriptor>, const DataFile&)`. Both functions use `reinterpret_cast` with `offsetof`-derived byte offsets to read/write each field directly. All `FieldType` values handled: Bool, Float, Int, String, Vec2, Vec3, Vec4, Quaternion (stored as 2/3/4 comma-separated floats; Quaternion order: w, x, y, z), and Composite (recurses into a child DataFile node using `getChildren()`). `DeserializeFields` guards every field access with `HasChild` — missing keys are silently skipped, giving forward-compatible deserialization. |
| P3-07 prereq — GameObject hierarchy infrastructure | ✅ Done | `Scene` now owns `GameObject`s via a `std::vector<std::unique_ptr<GameObject>>` parallel array in `SceneImpl`. `Scene::CreateGameObject(IComponentStorage*, name, parentNodeIndex)` is the exclusive creation path — it atomically allocates a `GameObject`, calls `AddComponent<Transform>` with the correct parent, and stores the `unique_ptr`. The old public `AddNode` is removed. `Transform::Transform` now accepts `parentNodeIndex` (no longer hardcodes root). `Component::_owner` is assigned before `OnCreate()` is called (previously a bug — `_owner` was never set). `Scene` has a proper destructor; `EngineInstance::Initialize` safely deletes the old scene before creating a new one. **Agreed serialization format:** GUID as the DataFile node key; `name` as a leaf value; components nested under `[components]`; child GameObjects nested under `[children]`. |
| P3-07 — Scene serialization | ✅ Done | `SerializeScene(const Scene&, DataFile&)` implemented in `Serialization.ixx`. Walks DFS from root's children (skips node 0). For each node: GUID as key, `name` as leaf, component fields into `[components]` (each component keyed by its type name, fields via `SerializeFields`), child GameObjects into `[children]` recursively. `Transform` is serialized like any other component — no special-casing on the write path. |
| P3-08 — Scene deserialization | ✅ Done | `DeserializeScene(const DataFile&, IComponentStorage&) -> Scene` implemented in `Serialization.ixx`. Recursively walks `[children]` nodes; for each: parses GUID from key, reads `name`, calls `scene.CreateGameObject(name, parentNodeIndex)`, restores GUID, then for each entry in `[components]` calls `DeserializeFields`. **Special case:** `Transform` is already created by `CreateGameObject` — fields loaded into the existing instance; no second instance created. |
| P3-09 — Scene round-trip tests | ⚠️ Partial | `RunSceneTest()` in `editor/src/EditorTests.ixx` creates a 3-node hierarchy (First_Child → Second_Child → Third_Child), serializes to `DataFile`, deserializes back, and asserts that GameObject count, names, and GUIDs all match. **Remaining:** component field value equality not yet verified. |
| P4-01 — Game DLL ABI | ✅ Done | `engine-core/include/GameDLLInterface.h` defines `GAME_DLL_ABI_VERSION 1` and the single `extern "C"` export `GameDLL_GetVersion()`. `game-template` converted from exe to `SHARED` DLL; `GameDLL.cpp` implements `GetVersion` and hosts a placeholder `MyComponent` (self-registers via `REGISTER_COMPONENT` on DLL load, unregisters on unload — no explicit register/unregister ABI functions needed). Bug fixed: `GlobalComponentRegistry()` now returns `ComponentRegistry&` (was returning by value). |
| P4-02 — DLL loader wrapper | ✅ Done | `DllLoader.ixx` exports `DllLoader` (RAII wrapper): `Load(wstring_view)` calls `LoadLibraryW` and uses `FormatMessage` for rich Windows error messages; `Unload()` / destructor call `FreeLibrary`; `IsLoaded()` guards state. Template `ResolveFunctionName<FPtr>(const char*)` wraps `GetProcAddress` and returns `Expected<FPtr>`. Free function `CheckGameDllVersion(const DllLoader&)` resolves `GameDLL_GetVersion` and rejects ABI version mismatches. `ErrorType::Dll` added to `EngineError.ixx`. `export import DllLoader` added to `EngineCore.ixx`. |
| P4-03 — ComponentTypeRegistry | ✅ Done | Already implemented as part of P4-01. `ComponentRegistry.ixx` maps `unsigned int typeID → ComponentFactoryFn`. Type IDs are `DJBHash(#ClassName)` (stable across DLL loads as long as the class name is unchanged), so the registry key is consistent with serialized data. `REGISTER_COMPONENT` macro registers on DLL load and unregisters on unload. Field descriptors are not stored statically — they are accessed via virtual `GetReflectedFields()` on component instances, which is sufficient for all current use cases. |
| P4-04 — DLL hot-reload | ⚠️ Partial | `FileWatcher.ixx` (`engine-core`) monitors a directory via `ReadDirectoryChangesW` (overlapped I/O). `Poll()` is non-blocking; `IsAvailableToCopy()` probes the file with an exclusive `CreateFileW` (share mode 0) to detect linker lock release. `HotReloadManager.ixx` (`editor`) orchestrates the full reload cycle: serialize live scene → clear scene → unload old DLL → wait for file availability → shadow-copy DLL to `%TEMP%\shadow_<name>.dll` → re-setup watcher → load shadow copy → deserialize scene. `Update()` is called each frame; errors are logged via `LOG_ERROR`. **Remaining:** availability wait loop (~10 s max) is a blocking spin-sleep on whichever thread calls `Update()` — should be moved to a dedicated background thread. Deserialization failure after a successful DLL load leaves the loader loaded but the scene empty; a recovery path is still needed. |
| P4-05 — Serialization state across reload | ✅ Done | Snapshot-serialize-deserialize cycle fully implemented in `HotReloadManager::LoadGameDll`. Components are destroyed before the old DLL is unloaded (safe ordering). `DeserializeFields` skips missing keys — new fields get default values; removed component types are skipped with `LOG_WARNING` (graceful forward/backward compatibility). `DeserializeFields` runs before `component->OnCreate()`, so `Transform::OnCreate()` calls `RefreshLocalToWorld()` with the restored position/rotation/scale already in place. |
| Component ownership & lifecycle fix | ✅ Done | `IComponentStorage` gains `virtual void DestroyComponent(Component*)`. `ComponentStorage` implements it: calls `OnDestroy()` then `std::erase_if` to remove the owning `unique_ptr`. `GameObject` stores `IComponentStorage* _storage` (set on first `AddComponent` call); its destructor iterates all components and calls `_storage->DestroyComponent(c)` on each, guaranteeing no components outlive their owner with a dangling `_owner` pointer. `SceneImpl` member declaration order fixed: `gameObjects` moved to last so it is destroyed first (in reverse order), before `TransformStorage` — correct ordering for any future `Transform::OnDestroy` that accesses scene data. |

## MVP Goal

A **console-based editor** that can:
- Load a game project folder (code + assets + scenes)
- Deserialize scenes into a live scene graph
- Display and modify scenes via console commands
- Save changes back to disk
- Load the game DLL, run game logic in-process with Play / Pause / Stop controls
- Detect and react to game DLL recompilation (hot-reload)

---

## Phase 1 — Project Foundation & Build System

Establish the repo layout, CMake configuration, and core utility infrastructure that everything else depends on.

- P1-01: Define repository layout and create CMake root project (engine-core lib, editor exe, game-template exe)
- P1-02: Configure C++23 standard, compiler warnings, sanitizers, and Debug/Release build profiles
- P1-03: Implement a Logger (severity levels: Trace/Debug/Info/Warn/Error; console + file sinks)
- P1-04: Implement core utility types: result/error type, string helpers, file-system wrappers (std::filesystem)
- P1-05: Implement math types: Vec2, Vec3, Vec4 (with arithmetic operators, dot, cross, normalize, lerp)
- P1-06: Implement Mat4 (identity, multiply, transpose, inverse) and Quaternion (from euler, slerp, to-matrix)
- P1-07: Implement Transform struct (position, rotation, scale → local matrix, world matrix from parent chain) — basic Transform done; hierarchy deferred to P2-05

---

## Phase 2 — GameObject / Component Model

The runtime object model at the heart of the engine.

- P2-01: Define Component base class: lifecycle hooks (OnCreate, OnUpdate, OnDestroy), owner pointer, enabled flag
- P2-02: Define field registration macros (REGISTER_FIELD, REGISTER_COMPONENT) and FieldDescriptor type
- P2-03: Define GameObject class: name, GUID, component map, parent/child pointers, AddComponent / GetComponent
- P2-04: Implement Transform as a built-in Component; link it as a mandatory component on every GameObject
- P2-05: Define Scene class + data-oriented transform hierarchy: flat world-transform array with parallel arrays for hierarchy relationships, mesh, physics body data, etc. — enabling efficient per-system processing. Also handles scene name/GUID and Update(dt) loop propagating to components.
- P2-06: Implement scene graph traversal helpers: depth-first iteration, world-transform accumulation

---

## Phase 3 — Custom Text Serialization Format

A simple, human-readable file format for scenes and assets.

- P3-01: Design and document the text format syntax (inspired by TOML/INI; define grammar on paper first)
- P3-02: Implement the Lexer/Tokenizer (keywords, identifiers, strings, numbers, braces, colons)
- P3-03: Implement the Parser (token stream → generic data tree: nodes, key-value pairs, arrays)
- P3-04: Implement the Serializer (data tree → formatted text output)
- P3-05: Implement Component → data tree using registered FieldDescriptors
- P3-06: Implement data tree → Component reconstruction (field name matching, type coercion)
- P3-07: Implement Scene serialization (walk hierarchy, emit each GameObject and its components)
- P3-08: Implement Scene deserialization (reconstruct hierarchy, instantiate and populate components)
- P3-09: Write round-trip tests (serialize a scene, deserialize it, assert equality)

---

## Phase 4 — Game DLL Interface & Hot-Reload

Allow user-authored C++ components to be compiled into a DLL and loaded at runtime.

- P4-01: Define the game DLL ABI: exported C functions (RegisterComponents, UnregisterComponents, GetVersion)
- P4-02: Implement DLL loader wrapper (LoadLibrary / FreeLibrary, symbol resolution, error handling)
- P4-03: Implement ComponentTypeRegistry: maps type name strings to factory functions + field descriptors
- P4-04: Implement DLL hot-reload: watch DLL file for changes, serialize live state, unload, reload, deserialize
- P4-05: Handle serialization state across reload (snapshot scenes before unload, restore after reload)
- P4-06: Create a game-template CMake project with a sample custom component as a reference/starting point
- P4-07: Design and evaluate component pooling in `ComponentStorage`: instead of freeing a destroyed component immediately, hold it in a per-type free-list; on next allocation of the same type, reclaim from the pool rather than heap-allocating. Key questions to answer: (1) what constitutes a "safe to reuse" component (no custom destructor side effects, no external references held); (2) whether to opt in per type (e.g. a flag on `COMPONENT_BODY`) or make it the default; (3) interaction with DLL unload — pooled components whose type is unregistered must be fully destroyed at that point, not recycled.

---

## Phase 5 — Asset System

Track and load project assets; provide stable references usable by components.

- P5-01: Define asset types (Texture, Mesh, AudioClip, etc. as enums/tags) and the asset manifest format
- P5-02: Implement AssetDatabase: scan project folder, index by GUID and path, detect duplicates
- P5-03: Implement asset reference type (AssetRef<T>) that stores a GUID and resolves lazily
- P5-04: Implement file watcher (ReadDirectoryChangesW) to detect asset additions/modifications/deletions
- P5-05: Implement asset hot-reload notification (broadcast change events to interested systems)

---

## Phase 6 — In-Process Game Loop & Play Mode

The editor loads the game DLL and drives a game loop in the same process.

- P6-01: Extend the game DLL ABI with game lifecycle hooks: `GameInitialize(Scene*)`, `GameUpdate(float dt)`, `GameShutdown()` exported as `extern "C"` alongside component registration
- P6-02: Implement a fixed-timestep game loop driven by the editor (target Hz configurable, spiral-of-death guard)
- P6-03: Implement Play / Pause / Stop state machine in the editor
- P6-04: Implement scene state snapshot on Play start; restore original state on Stop (so edits made during play are discarded)
- P6-05: Integrate hot-reload with the play state: if a DLL rebuild is detected while stopped, reload immediately; if playing, stop → reload → optionally re-enter play

---

## Phase 7 — Standalone Game Runner

A thin launcher executable that runs a shipped game without the editor.

- P7-01: Convert (or add) a `game-runner` CMake exe target that links `engine-core.dll` and loads the game DLL by path
- P7-02: Implement command-line argument parsing (project path, scene to load, window config)
- P7-03: Implement the standalone game loop (same fixed-timestep logic as the editor play mode, no editor overhead)
- P7-04: Implement clean shutdown: call `GameShutdown`, unload DLL, destroy scene

---

## Phase 8 — Console Editor MVP

The first usable editor: a console application with typed commands.

- P8-01: Implement project loader: read project folder, discover and index all scenes and assets
- P8-02: Implement the console command parser (tokenize input, dispatch to command handlers)
- P8-03: Implement commands: list-scenes, load-scene \<name\>, list-objects, inspect \<object\>
- P8-04: Implement commands: set-field \<obj\> \<component\> \<field\> \<value\>, add-component, remove-component
- P8-05: Implement save-scene command (serialize current scene back to disk)
- P8-06: Implement play / pause / stop commands (drive the in-process game loop; snapshot and restore scene state)
- P8-07: Implement hot-reload response: detect DLL rebuild, trigger Phase 5 reload flow, report to console

---

## Phase 9 — Rendering Abstraction Layer

Decouple the engine from any specific graphics API.

- P9-01: Define IRenderer interface (BeginFrame, EndFrame, Submit, Present) and renderer factory
- P9-02: Define render resource interfaces: IMesh, ITexture, IShader, IRenderTarget
- P9-03: Define RenderCommand types (DrawMesh, SetCamera, SetLight, ClearTarget) and RenderCommandQueue
- P9-04: Implement scene-to-render-data pass: walk scene graph, collect MeshRenderer components → draw calls
- P9-05: Implement NullRenderer (no-op implementation of IRenderer) for headless/test use

---

## Future Phases (not yet planned in detail)

- **Phase 10**: First concrete renderer backend (DirectX 11 or 12, or Vulkan)
- **Phase 11**: GUI editor (Qt / WxWidgets / Dear ImGui — TBD)
- **Phase 12**: Physics integration abstraction
- **Phase 13**: Audio integration abstraction
- **Phase 14**: Scripting (Lua or Python bindings — optional)
- **Phase 15**: Packaging / distribution pipeline
