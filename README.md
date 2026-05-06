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

## Building Instructions

#### Building Requirements:
* CMake
* C++ Compiler

For Windows:
```bash
git clone https://github.com/Joseph-RF/OpenGL-ProjectTemplate.git
cd OpenGL_CameraAndShaderClass
mkdir build
cd .\build\
cmake ..
cmake --build .
```