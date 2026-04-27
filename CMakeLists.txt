cmake_minimum_required(VERSION 3.28)  # 3.28+ required for C++20 modules support

project(Engine LANGUAGES CXX)
include(cmake/CompilerOptions.cmake)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)  # Disable compiler-specific extensions; stick to the standard

# Place all .exe and .dll outputs in the same directory so the DLL is
# automatically found at runtime without any PATH changes or manual copying.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_subdirectory(engine-core)
add_subdirectory(editor)
add_subdirectory(game-template)
