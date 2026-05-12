# Game Engine — Implementation Plan

## Architecture Summary

| Decision | Choice |
|---|---|
| Language | C++23, Windows-only |
| Build system | CMake 3.28+ (use VS-bundled cmake at `C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe` — system cmake in PATH is outdated at 3.11) |
| IDE | Visual Studio 2022 Community 17.14 |
| Process model | Editor process + Game process (TCP IPC) |
| Object model | GameObject / Component (no ECS) |
| Custom components | C++ DLL, hot-reloaded by the engine |
| Source organisation | C++20 modules (`.ixx` interface units) for all engine-owned code; `#include` for third-party headers via the module global fragment |
| engine-core linkage | **Shared DLL** (`engine-core.dll`) — both editor and game-template link against the same DLL so engine state (registries, allocators, type system) is never duplicated across the DLL boundary |
| Binary output | All `.exe` and `.dll` outputs land in `build/bin/<Config>/` via `CMAKE_RUNTIME_OUTPUT_DIRECTORY` |
| Serialization | Custom text format |
| IPC | TCP sockets (WinSock2) |
| Reflection | Manual field registration (macros) |
| Namespace | All engine-owned code lives in `namespace Engine` |
| Renderer | Abstraction layer only for MVP (backend TBD) |

## Progress

| Task | Status | Notes |
|---|---|---|
| P1-01 — CMake root project layout | ✅ Done | Project at `D:\CppProjects\Engine\`. Solution generated at `build\Engine.sln`. Three targets: `engine-core` (shared DLL), `editor` (exe), `game-template` (exe). |
| P1-02 — CMake build configuration | ✅ Done | `cmake/CompilerOptions.cmake` with `apply_compiler_options()` function; `/W4`, `/permissive-`, per-config `/Od`+`/RTC1` (Debug) and `/O2` (Release/RelWithDebInfo), `/O1` (MinSizeRel). Applied to all three targets. |
| P1-03 — Logger | ✅ Done | `LogManager`, `ConsoleLogSink` (ANSI colours per level), `FileLogSink` (appends to `EngineLog.txt`), `Log<>` template with `std::source_location` + `std::format`, configurable `minLogLevel`, DLL-exported via `ENGINE_CORE_API`. Convenience macros (`LOG_TRACE/DEBUG/INFO/WARN/ERROR`) in `engine-core/include/LoggerMacros.h` — include alongside `import Logger;`. |
| P1-04 — Core utility types | ⚠️ Partial | **Done:** `EngineError` (type + message, DLL-exported constructor) and `Expected<T>` alias over `std::expected<T, EngineError>` in `EngineError.ixx`; `export import`-ed from `EngineCore.ixx` so `import EngineCore;` exposes them transitively. **Remaining:** (1) string helpers needed by the serialiser (trim, split, case); (2) file I/O helpers (`ReadAllText`, `WriteAllText`) returning `Expected<T>` — defer until just before P3. |
| P1-05 — Math types: Vec2, Vec3, Vec4 | ✅ Done | `Math.ixx` exports `Vec2`, `Vec3`, `Vec4`: arithmetic operators (+, -, *, /), compound-assignment forms, unary negation, commutative `float * Vec` free functions, `Dot`, `Cross` (Vec3 only), `SqrMagnitude`, `Length`, `Normalize`/`NormalizeSafe`/`Normalized`/`NormalizedSafe` (with fallback), `Lerp` free functions (unclamped), swizzle accessors (`Vec3`: xy/xz/yz; `Vec4`: xyz/xy/zw), direction constants (Up/Down/Left/Right/Forward/Back), and `std::formatter` specializations for all three. `export import`-ed from `EngineCore.ixx`. |
| P1-06 — Mat4 and Quaternion | ✅ Done | In `Math.ixx`. **Mat4x4**: identity, multiply, transpose, inverse (cofactor-expansion, asserts non-zero det), `Scale`/`Translate`/`TRS` factory methods, `FromQuaternion`, `operator*(Mat4x4, Vec4)`. **Mat3x3**: identity, multiply, transpose, row/col accessors, `At`, `operator*(Mat3x3, Vec3)`. **Quaternion**: `Euler` (two overloads, `RotateOrder` enum for all 6 orderings), `Slerp` (falls back to lerp near-parallel), `AngleAxis`, `LookRotation`, `FromTo` (relative rotation), `Inverse` (conjugate, unit-quat only), `Magnitude`, `operator*` (Hamilton product), `operator-` (relative rotation), scalar `operator*`, `operator/=`, `operator+=`, construction from `Mat3x3`. `Mat4x4::TRS` composes all three. |
| P1-07 — Transform | ⚠️ Partial | `Transform.ixx` exports `Transform` class with local position/rotation/scale storage and a `_localToWorld` matrix, updated via `SetLocalTRS`, `SetLocalPosition`, `SetLocalRotation`, `SetLocalScale`, and `RefreshLocalToWorld`. **Deferred:** parent-chain / hierarchy integration — to be addressed in P2-05 with a data-oriented design (flat world-transform array + parallel arrays for hierarchy, mesh, physics data, etc. so systems like physics can process transforms efficiently). |
| P2-01 — Component base class | ✅ Done | `Component.ixx` exports `Component` (DLL-exported). Virtual lifecycle hooks: `OnCreate()`, `OnUpdate(float dt)`, `OnDestroy()`; all defaulted no-ops. `GetOwner()` returns `GameObject*` (forward-declared). `IsEnabled()`/`SetEnabled(bool)` with `_enabled = true` default. `_owner` is private, set exclusively by `friend class GameObject`. `export import`-ed from `EngineCore.ixx`. |
| P2-02 — Field registration macros and FieldDescriptor | ✅ Done |`Reflection.ixx` exports `FieldType` enum (Float, Int, Bool, String, Vec2, Vec3, Vec4, Quaternion) and `FieldDescriptor` (name, type, byte offset). `ReflectionMacros.h` companion header provides: `FIELD(...)` (passive no-op annotation), `COMPONENT_BODY(ClassName)` (declares static `GetTypeName`, `GetFieldDescriptors`, and virtual `GetReflectedFields` override), `REFLECTED_FIELDS(ClassName, ...)` and `REFLECTED_FIELDS_EMPTY(ClassName)` for use in `.reflected.h` seam files. `Component` gains a virtual `GetReflectedFields()` base returning an empty span. `export import`-ed from `EngineCore.ixx`. |
| P2-03 (prereq) — GUID | ✅ Done | `GUID.ixx` exports `Engine::GUID` (64-bit unsigned int, generated via `thread_local` `mt19937_64` seeded from `random_device`). `Invalid()` static factory returns `GUID(0)`; `IsValid()` checks for non-zero. `operator unsigned long long()` is `explicit` to prevent accidental arithmetic. `operator!=` omitted (synthesized from `operator==` in C++23). `std::hash<Engine::GUID>` and `std::formatter<Engine::GUID>` specializations defined (reachable to importers without needing `export`). `export import`-ed from `EngineCore.ixx`. |
| P2-03 design — GetComponent&lt;T&gt;() type identity | 📝 Decided | **Chosen approach: custom static type ID (no RTTI).** Each component class exposes a static `StaticTypeId()` (returning a unique `size_t` derived from the address of a local static) and a virtual `GetTypeId() const override` that delegates to it. `COMPONENT_BODY` macro will emit both. `GetComponent<T>()` compares `comp->GetTypeId() == T::StaticTypeId()` and `static_cast`s on match — zero RTTI, no exceptions. Template is constrained with a C++20 concept (`IsComponent`) requiring `std::derived_from<T, Component>` and a valid `T::StaticTypeId()`. |
| EngineInstance — DLL/linker fixes | ✅ Done | Moved constructor and destructor out of the class body (non-inline) to suppress C4251 on the private `std::unique_ptr<IComponentStorage>` member. Out-of-class `= default` caused duplicate-symbol linker errors (MSVC module quirk); replaced with explicit `{}` bodies. Remaining C4251 on the `unique_ptr` member itself is a known MSVC false positive when both types are `ENGINE_CORE_API` in the same DLL; options are pragma suppression on that line, raw pointer + manual delete, or pimpl. |

## MVP Goal

A **console-based editor** that can:
- Load a game project folder (code + assets + scenes)
- Deserialize scenes into a live scene graph
- Display and modify scenes via console commands
- Save changes back to disk
- Launch the game as a separate process and communicate over TCP
- Detect and react to game DLL recompilation (hot-reload)

---

## Phase 1 — Project Foundation & Build System

Establish the repo layout, CMake configuration, and core utility infrastructure that everything else depends on.

- P1-01: Define repository layout and create CMake root project (engine-core lib, editor exe, game-template exe)
- P1-02: Configure C++23 standard, compiler warnings, sanitizers, and Debug/Release build profiles
- P1-03: Implement a Logger (severity levels: Trace/Debug/Info/Warn/Error; console + file sinks)
- P1-04: Implement core utility types: result/error type, string helpers, file-system wrappers (std::filesystem)
- P1-05: ✅ Implement math types: Vec2, Vec3, Vec4 (with arithmetic operators, dot, cross, normalize, lerp)
- P1-06: ✅ Implement Mat4 (identity, multiply, transpose, inverse) and Quaternion (from euler, slerp, to-matrix)
- P1-07: ⚠️ Implement Transform struct (position, rotation, scale → local matrix, world matrix from parent chain) — basic Transform done; hierarchy deferred to P2-05

---

## Phase 2 — GameObject / Component Model

The runtime object model at the heart of the engine.

- P2-01: ✅ Define Component base class: lifecycle hooks (OnCreate, OnUpdate, OnDestroy), owner pointer, enabled flag
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

## Phase 4 — Asset System

Track and load project assets; provide stable references usable by components.

- P4-01: Define asset types (Texture, Mesh, AudioClip, etc. as enums/tags) and the asset manifest format
- P4-02: Implement AssetDatabase: scan project folder, index by GUID and path, detect duplicates
- P4-03: Implement asset reference type (AssetRef<T>) that stores a GUID and resolves lazily
- P4-04: Implement file watcher (ReadDirectoryChangesW) to detect asset additions/modifications/deletions
- P4-05: Implement asset hot-reload notification (broadcast change events to interested systems)

---

## Phase 5 — Game DLL Interface & Hot-Reload

Allow user-authored C++ components to be compiled into a DLL and loaded at runtime.

- P5-01: Define the game DLL ABI: exported C functions (RegisterComponents, UnregisterComponents, GetVersion)
- P5-02: Implement DLL loader wrapper (LoadLibrary / FreeLibrary, symbol resolution, error handling)
- P5-03: Implement ComponentTypeRegistry: maps type name strings to factory functions + field descriptors
- P5-04: Implement DLL hot-reload: watch DLL file for changes, serialize live state, unload, reload, deserialize
- P5-05: Handle serialization state across reload (snapshot scenes before unload, restore after reload)
- P5-06: Create a game-template CMake project with a sample custom component as a reference/starting point

---

## Phase 6 — IPC Protocol & TCP Messaging

Define how the editor and game processes talk to each other.

- P6-01: Design the message protocol: message header (type, length), message type enum, versioning
- P6-02: Implement TcpServer (WinSock2, async accept, per-connection receive loop)
- P6-03: Implement TcpClient (connect, send, async receive loop)
- P6-04: Implement message framing: length-prefixed binary frames, serialize/deserialize message payloads
- P6-05: Implement MessageDispatcher: route received messages to registered handler callbacks
- P6-06: Define and implement editor-side messages (SceneUpdate, FieldChange, PlayCmd, StopCmd, etc.)
- P6-07: Define and implement game-side messages (StateSnapshot, LogOutput, EntityCreated, etc.)

---

## Phase 7 — Game Process

The runtime process that loads scenes and runs game logic.

- P7-01: Implement game process entry point: parse args (project path, editor address), connect IPC
- P7-02: Implement the game loop: fixed-timestep Update, variable-rate frame tick
- P7-03: Implement scene playback: call OnCreate on load, tick OnUpdate each frame
- P7-04: Implement Play / Pause / Stop commands received over IPC
- P7-05: Implement live scene sync: apply FieldChange messages from editor without restarting

---

## Phase 8 — Console Editor MVP

The first usable editor: a console application with typed commands.

- P8-01: Implement project loader: read project folder, discover and index all scenes and assets
- P8-02: Implement the console command parser (tokenize input, dispatch to command handlers)
- P8-03: Implement commands: list-scenes, load-scene \<name\>, list-objects, inspect \<object\>
- P8-04: Implement commands: set-field \<obj\> \<component\> \<field\> \<value\>, add-component, remove-component
- P8-05: Implement save-scene command (serialize current scene back to disk)
- P8-06: Implement launch-game command (spawn game process, wait for IPC connection)
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
