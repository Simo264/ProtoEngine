SET(CMAKE_POLICY_DEFAULT_CMP0012 NEW)
SET(CMAKE_POLICY_DEFAULT_CMP0074 NEW)
SET(CMAKE_POLICY_DEFAULT_CMP0092 NEW)

cmake_minimum_required(VERSION 3.22)

project(ProtoEngine VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(FetchContent)
set(FETCHCONTENT_QUIET FALSE)

file(GLOB_RECURSE PROTOENGINE_SOURCES
  "${CMAKE_SOURCE_DIR}/src/*.cpp"
  "${CMAKE_SOURCE_DIR}/src/*.hpp"
)

find_package(OpenGL REQUIRED)

# include GLFW 3.4
set(GLFW_BUILD_DOCS OFF CACHE BOOL "Disable building GLFW docs" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "Disable building GLFW tests" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "Disable building GLFW examples" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "Disable GLFW install target" FORCE)
FetchContent_Declare(
  glfw
  GIT_REPOSITORY https://github.com/glfw/glfw.git
  GIT_TAG        3.4
  GIT_SHALLOW    TRUE
  GIT_PROGRESS   TRUE
)
FetchContent_MakeAvailable(glfw)

# include GLM
FetchContent_Declare(
 glm
 GIT_REPOSITORY	https://github.com/g-truc/glm.git
 GIT_TAG         0af55ccecd98d4e5a8d1fad7de25ba429d60e863
 GIT_SHALLOW     TRUE
 GIT_PROGRESS    TRUE
)
FetchContent_MakeAvailable(glm)

# include Assimp
set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build shared libraries" FORCE)
# Disable all importers by default
set(ASSIMP_BUILD_ALL_IMPORTERS_BY_DEFAULT OFF CACHE BOOL "Disable building all importers" FORCE)
# Disable tools, tests, and exporters to reduce build size
set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "Disable building Assimp tests" FORCE)
set(ASSIMP_BUILD_ASSIMP_TOOLS OFF CACHE BOOL "Disable building Assimp tools" FORCE)
set(ASSIMP_INSTALL OFF CACHE BOOL "Do not create install target for Assimp" FORCE)
set(ASSIMP_BUILD_MINIZIP ON CACHE BOOL "" FORCE)
# Explicitly enable the glTF (and thus GLB), FBX and OBJ importers
set(ASSIMP_BUILD_FBX_IMPORTER ON CACHE BOOL "Build FBX importer" FORCE)
set(ASSIMP_BUILD_OBJ_IMPORTER ON CACHE BOOL "Build OBJ importer" FORCE)
set(ASSIMP_BUILD_GLTF_IMPORTER ON CACHE BOOL "Build glTF importer" FORCE)

FetchContent_Declare(
  assimp
  GIT_REPOSITORY https://github.com/assimp/assimp.git
  GIT_TAG        v6.0.4
  GIT_SHALLOW    TRUE
  GIT_PROGRESS   TRUE
)
FetchContent_MakeAvailable(assimp)

add_subdirectory("${CMAKE_SOURCE_DIR}/dependencies/glad")
add_subdirectory("${CMAKE_SOURCE_DIR}/dependencies/imgui")
add_subdirectory("${CMAKE_SOURCE_DIR}/dependencies/stb")

add_executable(ProtoEngine ${PROTOENGINE_SOURCES})

target_include_directories(ProtoEngine PRIVATE
  "${CMAKE_SOURCE_DIR}/src"
)

target_link_libraries(ProtoEngine PRIVATE
  OpenGL::GL
  glad::glad
  glfw
  glm::glm
  stb::stb_image
  imgui::imgui
  assimp
)

target_compile_options(ProtoEngine PRIVATE -Wall -Wextra -Wpedantic)
