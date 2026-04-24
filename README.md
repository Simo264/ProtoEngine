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

```bash
cmake -S . -B ./build -DGLFW_BUILD_WAYLAND=1 -DGLFW_BUILD_X11=0 -DCMAKE_VERBOSE_MAKEFILE=OFF
cmake --build ./build --config Debug --target all -j8
 ./build/ProtoEngine
```
