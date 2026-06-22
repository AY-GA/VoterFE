cmake_minimum_required(VERSION 3.12)

set(CMAKE_C_STANDARD, 23)
set(CMAKE_C_STANDARD_REQUIRED, TRUE)
set(CMAKE_CXX_STANDARD, 23)
set(CMAKE_CXX_STANDARD_REQUIRED, TRUE)

project(voternn VERSION 1.0.0)

add_compile_options(-std=c++23)

if(DEBUG EQUAL 1)
	add_compile_definitions(VNN_DEBUG)
	add_compile_options(-g)
	message("VNN_DEBUG enabled")
else()
	add_compile_options(-O3)
	message("VNN_DEBUG features disabled.")
endif()

add_executable(voternn
"src/application.cpp"
"src/util/math.cpp"
"src/types/opinion.cpp"
"src/types/voter.cpp"
"src/ui/uimgr.cpp"
"src/types/district.cpp"
"src/util/socket/listener.cpp"
"src/util/init.cpp"
"src/callbacks/callbacks.cpp"
"src/util/llm/llmapi.cpp"
"src/simulation/simulation.cpp"
"src/types/law.cpp"
"src/types/media.cpp"
"src/types/product.cpp"
"src/types/company.cpp"
"src/types/grid.cpp"
"src/util/file.cpp"
"src/util/socket/listener_types.cpp"
"src/system/electoral.cpp"
"src/util/voter_profiles_loader.cpp"
)


target_include_directories(voternn PUBLIC "/usr/include/libxml2/")
target_include_directories(voternn PUBLIC "depend/")
target_link_libraries(voternn PUBLIC -lcurl -lxml2 -lz -llzma -lm -lncursesw -lssl -lpthread)

unset(DEBUG CACHE)
