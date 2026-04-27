add_executable(game-template)

target_sources(game-template PRIVATE src/main.cpp)

target_link_libraries(game-template PRIVATE engine-core)

apply_compiler_options(game-template)
