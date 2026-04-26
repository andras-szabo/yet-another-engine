add_executable(editor)

target_sources(editor PRIVATE src/main.cpp)

target_link_libraries(editor PRIVATE engine-core)
