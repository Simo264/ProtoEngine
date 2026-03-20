cmake_minimum_required(VERSION 3.14)

project(ProtoEngine VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(FetchContent)

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
  SOURCE_DIR     ${CMAKE_SOURCE_DIR}/dependencies/glfw
)
FetchContent_MakeAvailable(glfw)

# include GLM
FetchContent_Declare(
 glm
 GIT_REPOSITORY	https://github.com/g-truc/glm.git
 GIT_TAG         0af55ccecd98d4e5a8d1fad7de25ba429d60e863
 GIT_PROGRESS    TRUE
 SOURCE_DIR      ${CMAKE_SOURCE_DIR}/dependencies/glm
)
FetchContent_MakeAvailable(glm)

add_subdirectory("${CMAKE_SOURCE_DIR}/dependencies/glad")
add_subdirectory("${CMAKE_SOURCE_DIR}/dependencies/imgui")
add_subdirectory("${CMAKE_SOURCE_DIR}/dependencies/stb")

add_executable(ProtoEngine ${PROTOENGINE_SOURCES})

target_include_directories(ProtoEngine PRIVATE
  "${CMAKE_SOURCE_DIR}/src"
  "${CMAKE_SOURCE_DIR}/dependencies/glfw/include"
)

target_link_libraries(ProtoEngine PRIVATE
  OpenGL::GL
  glad::glad
  glfw
  glm::glm
  stb::stb_image
  imgui::imgui
)

target_compile_options(ProtoEngine PRIVATE -Wall -Wextra -Wpedantic)
