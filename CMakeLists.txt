cmake_minimum_required(VERSION 3.13)

# - project setup  -------------------------------------------------------------

project(mod1 VERSION 1.0.0 DESCRIPTION "Basic water flow simulation" LANGUAGES CXX)

message(STATUS "Setting up include headers directory...")
include_directories(include)
include_directories(include/utils)
include_directories(include/utils/opengl)
include_directories(include/utils/opengl/UI)

if (EXISTS ${PROJECT_SOURCE_DIR}/asset)
	message(STATUS "Copying assets...")
	file(COPY ${PROJECT_SOURCE_DIR}/asset DESTINATION ${CMAKE_BINARY_DIR})
endif()

if (EXISTS ${PROJECT_SOURCE_DIR}/shaders)
	message(STATUS "Copying shaders...")
	file(COPY ${PROJECT_SOURCE_DIR}/shaders DESTINATION ${CMAKE_BINARY_DIR})
endif()

# - dependencies ---------------------------------------------------------------

find_package(Git)

if (GIT_FOUND AND EXISTS "${PROJECT_SOURCE_DIR}/.git")
	message(STATUS "Updating git submodules...")
	set(SUBMODULES lib/glad;lib/stb;lib/sdl;lib/freetype;lib/assimp;)
	set(REPOSITORIES https://github.com/Dav1dde/glad.git;https://github.com/nothings/stb.git;https://github.com/SDL-mirror/SDL.git;git://git.sv.nongnu.org/freetype/freetype2.git;https://github.com/assimp/assimp.git)

	foreach(UPD_SUB IN LISTS SUBMODULES)
		message(STATUS "Updating ${UPD_SUB}...")
		execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive --remote -- ${UPD_SUB} WORKING_DIRECTORY ${PROJECT_SOURCE_DIR} RESULT_VARIABLE GIT_SUBMOD_RESULT)

		if (NOT GIT_SUBMOD_RESULT EQUAL "0")
			list(FIND SUBMODULES ${UPD_SUB} SUB_INDEX)
			list(GET REPOSITORIES ${SUB_INDEX} SUB_URL)

			execute_process(COMMAND ${GIT_EXECUTABLE} submodule add ${SUB_URL} ${UPD_SUB} WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
			execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive --remote -- ${UPD_SUB} WORKING_DIRECTORY ${PROJECT_SOURCE_DIR} RESULT_VARIABLE GIT_SUBMOD_RESULT)
			if (NOT GIT_SUBMOD_RESULT EQUAL "0")
				message(WARNING "Unable to update submodule ${UPD_SUB}")
			endif()
		endif()
	endforeach()
else()
	message(WARNING "Unable to update git submodules, please update them manually.")
endif()

message(STATUS "Setting up glad...")
add_subdirectory(lib/glad)
add_compile_definitions(GLAD)
include_directories(${CMAKE_BINARY_DIR}lib/glad/include)

message(STATUS "Setting up SDL...")
add_subdirectory(lib/sdl)
add_compile_definitions(SDL)
include_directories(lib/sdl/include)

message(STATUS "Setting up stb...")
include_directories(lib/stb)

message(STATUS "Setting up glm...")
find_package(glm REQUIRED)

message(STATUS "Setting up Freetype...")
add_subdirectory(lib/freetype)
add_compile_definitions(FREETYPE)
include_directories(lib/freetype/include)

message(STATUS "Setting up Assimp...")
add_subdirectory(lib/assimp)
add_compile_definitions(ASSIMP)
include_directories(lib/assimp/include)

message(STATUS "Setting up NlohmannJson...")
include(FetchContent)

FetchContent_Declare(json
  GIT_REPOSITORY https://github.com/ArthurSonzogni/nlohmann_json_cmake_fetchcontent
  GIT_TAG v3.9.1)

FetchContent_GetProperties(json)
if(NOT json_POPULATED)
  FetchContent_Populate(json)
  add_subdirectory(${json_SOURCE_DIR} ${json_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()

# - build options --------------------------------------------------------------

message(STATUS "Setting up build options...")
file(GLOB_RECURSE SRC_FILES "./include/*.hpp" "./src/*.cpp")
add_executable(mod1 ${SRC_FILES})

set_property(TARGET mod1 PROPERTY CXX_STANDARD 17)
set_property(TARGET mod1 PROPERTY CXX_STANDARD_REQUIRED ON)

# test that filesystem header actually is there and works
try_compile(HAS_FS "${CMAKE_BINARY_DIR}/temp"
"${CMAKE_SOURCE_DIR}/test/hasFilesystem.cpp"
            CMAKE_FLAGS -DCMAKE_CXX_STANDARD=17 -DCMAKE_CXX_STANDARD_REQUIRED=ON
            LINK_LIBRARIES stdc++fs)
if(HAS_FS)
    message(STATUS "Compiler has filesystem support")
else()
    message(FATAL_ERROR "Compiler is missing filesystem capabilities")
endif(HAS_FS)

target_compile_options(mod1 PUBLIC -l SDL2 -lGL)

if (UNIX)
	target_compile_options(mod1 PUBLIC -Wall -Wextra)
elseif (WIN32)
	target_compile_options(mod1 PUBLIC)
	set_target_properties(mod1 PROPERTIES COMPILE_DEFINITIONS BUILDER_STATIC_DEFINE)
else ()
	message(FATAL_ERROR "Detected platform is not supported!")
endif()

# - linking --------------------------------------------------------------------

message(STATUS "Linking...")
target_link_libraries(mod1 PRIVATE stdc++fs)  # for c++17 filesystem support
find_package(OpenGL REQUIRED)
target_link_libraries(mod1 PRIVATE OpenGL)
target_link_libraries(mod1 PRIVATE glad ${CMAKE_DL_LIBS})
target_link_libraries(mod1 PRIVATE SDL2)
target_link_libraries(mod1 PRIVATE glm)
target_link_libraries(mod1 PRIVATE freetype)
target_link_libraries(mod1 PRIVATE assimp)
target_link_libraries(mod1 PRIVATE nlohmann_json::nlohmann_json)
