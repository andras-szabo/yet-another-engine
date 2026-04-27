add_library(engine-core SHARED)

# Module interface units (.ixx) are listed under FILE_SET CXX_MODULES so that
# CMake's dependency scanner knows to process import/export declarations and
# determine the correct build order automatically.
target_sources(engine-core
  PUBLIC
    FILE_SET CXX_MODULES FILES
      src/EngineCore.ixx
)

# Any target that links engine-core also gets this include path.
# Used for third-party headers that can't be wrapped in a module (added later).
target_include_directories(engine-core
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

apply_compiler_options(engine-core)
