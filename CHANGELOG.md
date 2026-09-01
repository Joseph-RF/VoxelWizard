# Change Log
All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](http://keepachangelog.com/)
and this project adheres to [Semantic Versioning](http://semver.org/).

## [Released] - 2026-08-06

In this section, I'll be writing the series of updates and changes of the project made
starting with 0.1.0. I hope to remember all the changes I make as well as the reasons behind
them as it'll make for a better change log.

### [0.1.2]

Update 0.1.2

End of Summer Update :(

Yet another performance focussed update + OpenGL 4.6! A short update though.

Changing the block data within a chunk as well as the occupancy data to be vectors, that way, whenever there is
an empty chunk, those vectors are set to an empty vector to save on space. Measurements using the terrain generation
at this point in the project yielded a roughly 52% reduction in RAM usage per column in the scene (column in this
case refers to both empty and non-empty columns). Pretty good for a handful of lines!

A few tweaks here and there as well as the aforementioned upgrade to OpenGL 4.6. Note may downgrade in the future
if no 4.6-specific features are used for the purpose of compatibility.

### [0.1.1]

Update 0.1.1

Instancing!

Another performance focussed update. Instead of a mesh for the entire chunk, will be using quads and drawing
however many are required per chunk using instancing.

Details:

- **Going from**: Creating a single mesh for an entire chunk, where each face is drawn using two triangles and six
vertices, where each verex carries an int32 worth of data.

- **To**: Creating a quad mesh with four vertices and drawing a quad for each face using GL_TRIANGLE_STRIP then
instancing the quad however many times is required for the chunk. The quad vertices only carry positional
information in the quad. Position within the chunk, normal, and block_id is passed to the vertex shader once
per instance (i.e. once per quad/face) using instanced arrays.

- This reduces the amount of memory vertices occupy on the GPU by roughly 83% as only **1** int32 is needed per
face instead of **6**.

- Additionally, only 16 possible positions for each face is needed instead of 17 since vertices no longer need
to store position within the chunk, just position within the quad meaning 3D position data per face only
requires 4 bits instead of the previous 5.

Using Vericidium's [I Optimised My Game Engine Up To 12000 FPS](https://www.youtube.com/watch?v=40JzyaOYJeY)

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

### [0.2.0]

Next step in instancing where instead of using one draw call per chunk, use one draw call for all chunks
involved.

Will need to use OpenGL 4.2+

Once again, using Vericidium's
[I Optimised My Game Engine Up To 12000 FPS](https://www.youtube.com/watch?v=40JzyaOYJeY)

### [Future]

Character that moves around, magic, enemies, enemy logic and more!