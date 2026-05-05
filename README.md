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
sudo apt install libwayland-dev libxkbcommon-dev xorg-dev
```
- On Fedora and derivatives like Red Hat:
```bash
sudo dnf install wayland-devel libxkbcommon-devel libXcursor-devel libXi-devel libXinerama-devel libXrandr-devel
```

Compiling and running the code:

```bash
cmake -B ./build
cmake --build ./build --config Debug --target all -j8
./build/ProtoEngine
```
