add_library(game-template SHARED)

target_sources(game-template PRIVATE src/GameDLL.cpp)

target_link_libraries(game-template PRIVATE engine-core)

apply_compiler_options(game-template)
