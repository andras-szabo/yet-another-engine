add_executable(editor)

target_sources(editor 
	PUBLIC
	  FILE_SET CXX_MODULES FILES
	  src/EditorTests.ixx
	  src/HotReloadManager.ixx
	PRIVATE src/main.cpp)

target_link_libraries(editor PRIVATE engine-core)

apply_compiler_options(editor)
