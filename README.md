# ProtoEngine

Real time graphics engine written in C++23 and OpenGL 4.6.
This project aims to explore and implement modern rendering techniques, 
with a particular focus on Physically Based Rendering to fully understand the principles of realistic shading

## Implemented features:

- Program Pipeline
- Scene Graph
- Model Loading

## Libraries used:

- glad (OpenGL loader)
- GLFW
- GLM
- ImGui
- Assimp

## Build instructions

Installing dependencies for GLFW:

- On Debian and derivatives like Ubuntu and Linux Mint:
```bash
conan profile detect --force
conan install . -s build_type=Debug --output-folder=build --build=missing

cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=build/Debug/generators/conan_toolchain.cmake -DCMAKE_BUILD_TYPE=Debug
cmake --build ./build --parallel 8

#cmake -S . -B ./build -DGLFW_BUILD_WAYLAND=1 -DGLFW_BUILD_X11=0 -DCMAKE_VERBOSE_MAKEFILE=OFF
#./build/ProtoEngine
```
