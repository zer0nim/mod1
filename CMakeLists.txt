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
	# Update submodules as needed
    option(GIT_SUBMODULE "Check submodules during build" ON)
    if (GIT_SUBMODULE)
        message(STATUS "Submodule update")
        execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive
                        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                        RESULT_VARIABLE GIT_SUBMOD_RESULT)
        if (NOT GIT_SUBMOD_RESULT EQUAL "0")
            message(FATAL_ERROR "git submodule update --init failed with ${GIT_SUBMOD_RESULT}, please checkout submodules")
        endif()
    endif()
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
add_subdirectory(lib/glm)
add_compile_definitions(GLM)
include_directories(lib/glm)

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
if (NOT json_POPULATED)
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
