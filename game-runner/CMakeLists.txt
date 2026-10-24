add_executable(game-runner)

target_sources(game-runner
    PRIVATE
	  src/main.cpp

	PUBLIC
	  FILE_SET CXX_MODULES FILES)

target_link_libraries(game-runner PRIVATE engine-core)

apply_compiler_options(game-runner)
