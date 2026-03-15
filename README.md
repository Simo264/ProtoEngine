# ProtoEngine 🚀

Questo è un progetto nato puramente per scopi didattici.

Si tratta di un motore grafico scritto da zero in C++23 e OpenGL 4.6. 
L'obbiettivo non è quello di competere con motori esistenti più popolari e industriali, ma piuttosto capire a fondo come funziona un engine grafico e implementarlo usando le conoscenze acquisite in algebra lineare e in computer grafica.

Librerie usate:
- glad (OpenGL loader)
- GLFW
- ImGui

## Build instructions

```bash
cmake -S . -B ./build -D GLFW_BUILD_WAYLAND=1 -D GLFW_BUILD_X11=0
cmake --build ./build --config Debug --target all -j8
 ./build/ProtoEngine
```
