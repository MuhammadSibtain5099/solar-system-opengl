# Solar System – OpenGL Phong Shading Demo

## Description
Renders two textured spheres with Phong (per-fragment) lighting:
- **Sun** – stationary at the world origin, acts as the point light source.
- **Earth** – orbits the sun and self-rotates.

## Dependencies
| Library | Purpose |
|---------|---------|
| OpenGL 3.3+ | Core rendering API |
| GLFW 3 | Window & input |
| GLEW | OpenGL extension loader |
| GLM | Math (matrices, vectors) |
| stb_image (included) | Texture loading |

### Install on Ubuntu/Debian
```bash
sudo apt install libglfw3-dev libglew-dev libglm-dev
```

### Install on macOS (Homebrew)
```bash
brew install glfw glew glm
```

## Texture Maps
Place the following files inside a `textures/` folder next to the executable:
```
textures/
  sun.jpg       # e.g. from NASA or polyhaven.com
  earth.jpg     # e.g. from NASA Blue Marble
```
Free sources:
- https://visibleearth.nasa.gov/
- https://polyhaven.com/textures

The program will still run (with a fallback solid color) if textures are missing.

## Build
```bash
mkdir build && cd build
cmake ..
cmake --build . -j4
```

## Run
```bash
cd build
./solar_system        # Linux/macOS
solar_system.exe      # Windows
```
Press **ESC** to exit.

## Controls
| Key | Action |
|-----|--------|
| ESC | Quit |

## AI Assistance
This project was developed with assistance from **Claude (Anthropic)**.

### Prompts used:
1. *"Write an OpenGL program that renders two textured and illuminated spheres. The first sphere is at the center and remains stationary. The second sphere rotates around the first sphere, similar to how Earth orbits the Sun. Place a point light source at the center of the first sphere and implement Phong shading (per-fragment lighting) in GLSL. Apply texture to both spheres."*

2. *"Generate a complete CMakeLists.txt for an OpenGL project using GLFW, GLEW, GLM, and stb_image."*

3. *"Generate a README explaining dependencies, how to build with CMake, and how to run."*

## File Structure
```
solar_system/
├── main.cpp         ← C++ source (sphere geometry, texture loading, render loop)
├── vertex.glsl      ← Vertex shader (MVP transform, normal/UV passthrough)
├── fragment.glsl    ← Fragment shader (Phong: ambient + diffuse + specular)
├── stb_image.h      ← Single-header image loader (public domain)
├── CMakeLists.txt   ← Build configuration
├── README.md        ← This file
└── textures/
    ├── sun.jpg
    └── earth.jpg
```
