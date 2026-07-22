# Voxel Wizard (OpenGL)

## About The Project

A project born from my love of my Minecraft and interest in voxel engines. I currently have
yet to define properly where this will go but for now I want to get at least some terrain
generation going. Once that has been achieved, things may get more interesting.

I have decided to add a Change Log to this project mainly to keep track of the things I want
to add and be able to reflect on the things I have done. Seems useful.

## Built With

This project was built with the following third party dependencies:

* [OpenGL](https://www.opengl.org/)
* [DearImGui](https://github.com/ocornut/imgui)
* [GLM](https://glm.g-truc.net/0.9.9/index.html)
* [GLFW](https://www.glfw.org/)
* [Glad](https://github.com/premake-libs/glad?tab=readme-ov-file)
* [stb](https://github.com/nothings/stb)
* [FastNoiseLite](https://github.com/Auburn/FastNoiseLite)

## Building Instructions

#### Building Requirements:
* CMake
* C++ Compiler

For Windows:
```bash
git clone https://github.com/Joseph-RF/VoxelWizard
cd VoxelWizard
mkdir build
cd .\build\
cmake ..
cmake --build .
```

## Sources:

* [Voxel World Optimisations](https://vercidium.com/blog/voxel-world-optimisations/)
Vericidium article on general optimisations for voxel engine
* [Let's Make a Voxel Engine](https://sites.google.com/site/letsmakeavoxelengine/home?authuser=0)
* [Meshing in a Minecraft Game](https://0fps.net/2012/06/30/meshing-in-a-minecraft-game/)
More optimisations
* [Voxel Engine in a Weekend](https://daymare.net/blogs/voxel-engine-in-a-weekend/)
Article for getting started on a voxel engine. At the end of the article there are a lot of
pointers on how to improve it
* [General Youtube Search for optimisations](https://www.youtube.com/results?search_query=voxel+engine+optimisations)
* [I Optimised My Game Engine Up To 12000 FPS](https://www.youtube.com/watch?v=40JzyaOYJeY)
* [Perlin Noise: A Procedural Generation Algorithm](https://rtouti.github.io/graphics/perlin-noise-algorithm)