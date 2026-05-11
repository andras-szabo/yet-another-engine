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
  PUBLIC
    FILE_SET CXX_MODULES FILES
      src/Component.ixx
      src/IComponentStorage.ixx
      src/ComponentStorage.ixx
      src/EngineCore.ixx
      src/EngineError.ixx
      src/EngineInstance.ixx
      src/GameObject.ixx
      src/GUID.ixx
      src/Logger.ixx
      src/Math.ixx
      src/Reflection.ixx
      src/Transform.ixx
     )

# Any target that links engine-core also gets these include paths.
# CMAKE_CURRENT_BINARY_DIR exposes the generated engine_core_api.h header.
target_include_directories(engine-core
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_BINARY_DIR}
)

apply_compiler_options(engine-core)
