cmake_minimum_required(VERSION 4.3.3)

project(stb_image LANGUAGES C CXX)

add_library(stb_image STATIC "${CMAKE_CURRENT_SOURCE_DIR}/stb_image.cpp")

target_include_directories(stb_image PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

set_target_properties(stb_image PROPERTIES
  POSITION_INDEPENDENT_CODE ON
  CXX_STANDARD 23
  CXX_STANDARD_REQUIRED ON
)

add_library(stb::stb_image ALIAS stb_image)