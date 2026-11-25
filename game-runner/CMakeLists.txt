add_executable(game-runner)

target_sources(game-runner
    PRIVATE
    src/main.cpp)

target_link_libraries(game-runner PRIVATE engine-core)

apply_compiler_options(game-runner)
