cmake_minimum_required(VERSION 4.3.3)

project(ProtoEngine LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(OpenGL REQUIRED)
find_package(glad REQUIRED)
find_package(glm REQUIRED)
find_package(glfw3 REQUIRED)
find_package(assimp REQUIRED)

add_subdirectory("${CMAKE_SOURCE_DIR}/dependencies/imgui")
add_subdirectory("${CMAKE_SOURCE_DIR}/dependencies/stb")

file(GLOB_RECURSE PROTOENGINE_SOURCES
  "${CMAKE_SOURCE_DIR}/src/*.cpp"
  "${CMAKE_SOURCE_DIR}/src/*.hpp"
)
add_executable(${PROJECT_NAME} ${PROTOENGINE_SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
  "${CMAKE_SOURCE_DIR}/src"
)

target_link_libraries(${PROJECT_NAME} PRIVATE
  OpenGL::GL
  glad::glad
  glfw
  glm::glm
  stb::stb_image
  imgui::imgui
  assimp::assimp
)

target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic)
