cmake_minimum_required(VERSION 3.28)
project(GameTemplate LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# All .dll and .lib outputs go to build/bin/<Config>/
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# ─── SDK location ─────────────────────────────────────────────────────────────
# Set ENGINE_CORE_SDK_DIR on the cmake command line, e.g.:
#   cmake -B build -G "Visual Studio 17 2022" -DENGINE_CORE_SDK_DIR=C:/path/to/sdk
if(NOT DEFINED ENGINE_CORE_SDK_DIR)
    message(FATAL_ERROR
        "ENGINE_CORE_SDK_DIR is not set.\n"
        "Configure with: cmake -B build -DENGINE_CORE_SDK_DIR=<path-to-sdk>")
endif()

# Normalise to a cmake-style path (forward slashes, no trailing slash)
cmake_path(SET ENGINE_CORE_SDK_DIR NORMALIZE "${ENGINE_CORE_SDK_DIR}")
message(STATUS "Using engine-core SDK at: ${ENGINE_CORE_SDK_DIR}")

# ─── Compiler options helper ──────────────────────────────────────────────────
include(${ENGINE_CORE_SDK_DIR}/cmake/CompilerOptions.cmake)

# ─── Imported engine-core target ─────────────────────────────────────────────
# Points at the pre-built DLL + import library for each configuration.
add_library(engine-core SHARED IMPORTED GLOBAL)
set_target_properties(engine-core PROPERTIES
    IMPORTED_LOCATION_DEBUG          "${ENGINE_CORE_SDK_DIR}/bin/Debug/engine-core.dll"
    IMPORTED_IMPLIB_DEBUG            "${ENGINE_CORE_SDK_DIR}/lib/Debug/engine-core.lib"
    IMPORTED_LOCATION_RELEASE        "${ENGINE_CORE_SDK_DIR}/bin/Release/engine-core.dll"
    IMPORTED_IMPLIB_RELEASE          "${ENGINE_CORE_SDK_DIR}/lib/Release/engine-core.lib"
    IMPORTED_LOCATION_RELWITHDEBINFO "${ENGINE_CORE_SDK_DIR}/bin/RelWithDebInfo/engine-core.dll"
    IMPORTED_IMPLIB_RELWITHDEBINFO   "${ENGINE_CORE_SDK_DIR}/lib/RelWithDebInfo/engine-core.lib"
    IMPORTED_LOCATION_MINSIZEREL     "${ENGINE_CORE_SDK_DIR}/bin/MinSizeRel/engine-core.dll"
    IMPORTED_IMPLIB_MINSIZEREL       "${ENGINE_CORE_SDK_DIR}/lib/MinSizeRel/engine-core.lib"
    MAP_IMPORTED_CONFIG_MINSIZEREL   Release
)
target_include_directories(engine-core INTERFACE
    "${ENGINE_CORE_SDK_DIR}/include"
)

# ─── Collect SDK module interfaces ───────────────────────────────────────────
file(GLOB ENGINE_CORE_MODULE_INTERFACES "${ENGINE_CORE_SDK_DIR}/modules/*.ixx")

# ─── game-template DLL ────────────────────────────────────────────────────────
# The engine SDK's .ixx files are added to game-template's own FILE_SET so MSVC
# recompiles them locally and generates the .ifc (Binary Module Interface) files
# needed to resolve `import EngineCore;` etc.
#
# Note: some engine .ixx files contain function definitions for ENGINE_CORE_API
# types. When compiled here (with ENGINE_CORE_API = __declspec(dllimport)), MSVC
# issues C4273 and creates local object-file copies. These are functionally
# identical to the DLL's implementations and are dead code in game-template.dll.
# /FORCE:MULTIPLE tells the linker to continue when duplicate symbols arise from
# those copies conflicting with engine-core.lib's import stubs.
# The two critical singletons (GlobalLoggerInstance, GlobalComponentRegistry) and
# GUID's RNG are moved to .cpp module implementation units in the engine, so they
# always resolve to the DLL — not a local copy.
add_library(game-template SHARED)

target_sources(game-template
  PRIVATE
    src/GameDLL.cpp
    src/SpinnerComponent.cpp
  PUBLIC
    FILE_SET CXX_MODULES
    BASE_DIRS
      "${ENGINE_CORE_SDK_DIR}/modules"
      "${CMAKE_CURRENT_SOURCE_DIR}/src"
    FILES
      ${ENGINE_CORE_MODULE_INTERFACES}
      src/SpinnerComponent.ixx
)

target_link_libraries(game-template PRIVATE engine-core)

# Suppress LNK2005 duplicate-symbol errors caused by local ENGINE_CORE_API
# definitions compiled from the SDK .ixx files (see note above).
if(MSVC)
    target_link_options(game-template PRIVATE /FORCE:MULTIPLE)
endif()

apply_compiler_options(game-template)

