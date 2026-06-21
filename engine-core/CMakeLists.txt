include(GenerateExportHeader)

add_library(engine-core SHARED)

# Generates engine_core_api.h with ENGINE_CORE_API defined as __declspec(dllexport)
# when building engine-core and __declspec(dllimport) when consuming it.
# Without at least one exported symbol, MSVC never produces an import .lib.
generate_export_header(engine-core
    EXPORT_MACRO_NAME ENGINE_CORE_API
    EXPORT_FILE_NAME engine_core_api.h
)

# Module interface units (.ixx) are listed under FILE_SET CXX_MODULES so that
# CMake's dependency scanner knows to process import/export declarations and
# determine the correct build order automatically.
target_sources(engine-core
  PRIVATE
    src/Component.cpp 
    src/ComponentRegistry.cpp
    src/ComponentStorage.cpp
    src/DataFile.cpp
    src/DllLoader.cpp
    src/EngineError.cpp
    src/EngineInstance.cpp
    src/FileWatcher.cpp
    src/GameObject.cpp
    src/GUID.cpp
    src/Logger.cpp
    src/Math.cpp
    src/Scene.cpp
    src/Serialization.cpp
    src/Transform.cpp

  PUBLIC
    FILE_SET CXX_MODULES FILES
      src/Component.ixx
      src/ComponentRegistry.ixx
      src/IComponentStorage.ixx
      src/ComponentStorage.ixx
      src/DataFile.ixx
      src/DllLoader.ixx
      src/EngineCore.ixx
      src/EngineError.ixx
      src/EngineInstance.ixx
      src/FileWatcher.ixx
      src/GameObject.ixx
      src/GUID.ixx
      src/Interfaces.ixx
      src/Logger.ixx
      src/Math.ixx
      src/Reflection.ixx
      src/Scene.ixx
      src/Serialization.ixx
      src/Transform.ixx
      src/Utility.ixx
     )

# Any target that links engine-core also gets these include paths.
# CMAKE_CURRENT_BINARY_DIR exposes the generated engine_core_api.h header.
target_include_directories(engine-core
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_BINARY_DIR}
)

apply_compiler_options(engine-core)

# ─── SDK install rules ────────────────────────────────────────────────────────
# Run: cmake --install build --config Debug   (or Release)
# Output lands in <build>/sdk/ by default, or at CMAKE_INSTALL_PREFIX.
#
# SDK layout:
#   sdk/bin/<Config>/   – engine-core.dll
#   sdk/lib/<Config>/   – engine-core.lib  (MSVC import library)
#   sdk/modules/        – module interface units (.ixx only; .cpp impl units excluded)
#   sdk/include/        – public headers + generated engine_core_api.h
#   sdk/cmake/          – CompilerOptions.cmake helper

install(TARGETS engine-core
    RUNTIME DESTINATION sdk/bin/$<CONFIG>   # DLL
    ARCHIVE DESTINATION sdk/lib/$<CONFIG>   # import .lib
)

# Module interface units (.ixx only — NOT the .cpp module implementation units).
# game-template recompiles these locally to generate .ifc files; it links
# against the pre-built .lib above for the actual implementations.
install(
    FILES
        src/Component.ixx
        src/ComponentRegistry.ixx
        src/IComponentStorage.ixx
        src/ComponentStorage.ixx
        src/DataFile.ixx
        src/DllLoader.ixx
        src/EngineCore.ixx
        src/EngineError.ixx
        src/EngineInstance.ixx
        src/FileWatcher.ixx
        src/GameObject.ixx
        src/GUID.ixx
        src/Interfaces.ixx
        src/Logger.ixx
        src/Math.ixx
        src/Reflection.ixx
        src/Scene.ixx
        src/Serialization.ixx
        src/Transform.ixx
        src/Utility.ixx
    DESTINATION sdk/modules
)

# Public headers (ReflectionMacros.h, LoggerMacros.h, GameDLLInterface.h)
install(
    DIRECTORY include/
    DESTINATION sdk/include
)

# Generated export header (lives in the binary dir after generate_export_header)
install(
    FILES ${CMAKE_CURRENT_BINARY_DIR}/engine_core_api.h
    DESTINATION sdk/include
)

# cmake helper so game-template can call apply_compiler_options()
install(
    FILES ${CMAKE_SOURCE_DIR}/cmake/CompilerOptions.cmake
    DESTINATION sdk/cmake
)
