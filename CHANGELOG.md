# Change Log
All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](http://keepachangelog.com/)
and this project adheres to [Semantic Versioning](http://semver.org/).

## [Released] - 2026-08-06

In this section, I'll be writing the series of updates and changes of the project made
starting with 0.1.0. I hope to remember all the changes I make as well as the reasons behind
them as it'll make for a better change log.

### [0.1.0]

Update 0.1.0

The first update/version of this project. Just voxel terrain, shot from a 2.5D Iso perspective that will act as 
a jumping off point for this project. 

Note that this will be using my Infinite Voxel Terrain project as a starting point. Specifically, it uses 
Version 0.2.6 with some of the performance features that appear in the following update such as block_id into 
the vertex data and keeping a colour look-up in the vertex shader using a UBO.

Main changes with regards to the Infinite Voxel Terrain project:

- Will start by moving from an infinite world to a finite world. 
- Changing the camera to an orbiting camera with orthographic projection instead of perspective.
- Checking if a chunk is empty before generating vertices to avoid needless work. Test results below:
	- Tests before skipping vertex generation of empty chunks:
		- 20 milliseconds average for world size of 16 averaged over 20 initScene calls (release)
		- 79 milliseconds average for world size of 32 averaged over 20 initScene calls (release)
		- 1609 milliseconds average for world size of 64 averaged over 5 initScene calls (release)
	- Tests after skipping vertex generation of empty chunks:
		- 16 milliseconds average for world size of 16 averaged over 20 initScene calls (release)
		- 66 milliseconds average for world size of 32 averaged over 20 initScene calls (release)
		- 1176 milliseconds average for world size of 64 averaged over 5 initScene calls (release)
	- Looking at a 16% reduction at least in initScene call time (which as of now is the primary application startup
function call) by simply adding around 10 lines of code.
- Removed code that won't be needed in this project that comes from Infinite Voxel Terrain

## [Unreleased] - 2026-08-06

Here will write a vague roadmap of the upcoming updates and changes to the project.
Not supposed to be very rigid and features will move from one planned update to the next.
Some features will be pushed further down the line, others will arrive earlier than expected.
I suspect many changes won't be planned for entirely.

### [0.1.1]

Fairly quick and easy (hopefully) implementation of instanced quads.
Drawn to be able to use quad strips and avoid 2 of the 6 vertices used to draw a quad (two triangles)
Instance call will be done once per chunk so should primarily just be a reduction in data
Using Vericidium'ss [I Optimised My Game Engine Up To 12000 FPS](https://www.youtube.com/watch?v=40JzyaOYJeY)

### [0.2.0]

Next step in instancing where instead of using one draw call per chunk, use one draw call for all chunks
involved.

Will need to use OpenGL 4.2+

Once again, using Vericidium's
[I Optimised My Game Engine Up To 12000 FPS](https://www.youtube.com/watch?v=40JzyaOYJeY)

### [Future]

Character that moves around, magic, enemies, enemy logic and more!