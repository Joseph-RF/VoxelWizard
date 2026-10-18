# Change Log
All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](http://keepachangelog.com/)
and this project adheres to [Semantic Versioning](http://semver.org/).

## [Released] - 2026-10-12

In this section, I'll be writing the series of updates and changes of the project made
starting with 0.1.0. I hope to remember all the changes I make as well as the reasons behind
them as it'll make for a better change log.

### [0.2.1]

Update 0.2.1

The Wizard Moves

Camera is no longer tied to keyboard presses but to a target_entity. If said target_entity leaves the camera's
deadzone, the camera will move to keep it in the deadzone.

View and projection matrices are now OWNED by the camera, and are calculated once per app update. This is because 
view and projection are needed in multiple places in the game so no reason to calculate it twice within one frame.

### [0.2.0]

Update 0.2.0

Game Architecture and Entity Component System

Before diving deeper into the game I wanted to establish an architectural pattern to follow for the rest of development.
I knew that without it, the code would grow wild and unwieldy, especially to make changes. This could prove fatal for
the game since I could see myself quickly getting frustrated with a project that becomes harder and harder to work with.

The form of architecture I settled on was Entity Component System (ECS), as it offered flexibility for the types of
objects I could create as well as enabling good performance through data being stored in contiguous, dense arrays.

Flexible and performant code was too alluring of a prospect which is why I went for it. 

The implementation of ECS in this project is copied straight from Austin Morlan's 
[A Simple Entity Component System (ECS) (C++)](https://austinmorlan.com/posts/entity_component_system/)

Another great resource for learning has been Dave Churchill's
[COMP4300 Game Programming](https://www.youtube.com/watch?v=p6JKosBmB0I&list=PLdEgIOpXtE-s)

To complement the new architecture, I have reshuffled the source files a little into two main directories: engine and
game. Always good to keep things organised.

Functionally, nothing is different to the previous update but it should set me up for success later down the line.

### [0.1.3]

Update 0.1.3

The Wizard appears! (Or at least a strange placeholder for it)

#### Resource Manager

Adding a resource manager for meshes that will be used by several entities which will definitely be the case for enemies
but just getting ahead of that potential problem now. It features a method to parse .obj files directly into meshes for
actors such as the player

Currently, object models are initialised in a method of the App class since the App class is currently the one holding
onto the resource manager. I am considering moving this to the renderer, but for now it'll sit in the App.

#### Player

This is far less interesting and just holds onto the position, rotation, size and mesh of the player. Not much to see in
this class. For now.

The App will hold onto the player as well as other future actors such as enemies.

#### Other changes

RenderContext modified to now include resource manager and player data. Need to get that information to the renderer
somehow. This is one of those reasons "for" the moving of the resource manager to the renderer.

Did some work on Chunks and Columns to lower memory usage. Removed obsolete "indices" data member from Chunk. Empty
chunks, on top of now having any block data, now no longer have a mesh. Added a check in the generateMesh method for
empty chunks.

Moved the generation of block data from a height map, to a separate function instead of sitting in the Chunk constructor

Moved the Mesh class to its own file as it is no longer unique to Chunks. Modified Mesh to now accept indexed vertex
data for Mesh generation on top of the previous non-indexed Chunk meshes. This was done to work with the data coming in
from .obj files.

#### Things to keep in mind

Doing some slightly dangerous behaviour where resource manager's tryGetMesh returns a raw pointer from a unique
pointer it owns. Will need to be careful with the usage.

How should the lifetime of models work? Are all models loaded in and stay until the end of the program?

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

## [Unreleased] - 2026-10-12

Here will write a vague roadmap of the upcoming updates and changes to the project.
Not supposed to be very rigid and features will move from one planned update to the next.
Some features will be pushed further down the line, others will arrive earlier than expected.
I suspect many changes won't be planned for entirely.

### [Future]

Why is there a direction enum for Camera and another for Events, surely it could be the same?

Once models start be finalised, consider changing the size of the shader and mesh ids in the
renderable component. Could halve size from 16 bits to 8 bits if there will never be more than 255 distinct 
models.

When the camera moves to keep Wizard in deadzone, add a little bit of padding. Make the camera's displacement smooth
when it adjusts for the deadzone.

Character that moves around, magic, enemies, enemy logic and more!