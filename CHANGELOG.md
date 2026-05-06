# Change Log
All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](http://keepachangelog.com/)
and this project adheres to [Semantic Versioning](http://semver.org/).

## [Released] - 2026-05-06

In this section, I'll be writing the series of updates and changes of the project made
starting with 0.1.0. I hope to remember all the changes I make as well as the reasons behind
them as it'll make for a better change log.

### [0.1.0]

The first update/version of this project. Just a simple, solid coloured, flat voxel terrain.
Think superflat in Minecraft but the grass is just one colour. This update will act as the
foundation for the project.

Primary features added for this version:

(Note that by "primary" I am referring to features of a voxel engine that I found to be 
non-trivial, and therefore deserving of some notice)

- "Creation Distance" - Distance at which chunk data is loaded onto memory. Once a chunk
  is further away than this distance, remove from memory
- "Mesh Distance" - Distance at which chunk meshes are created and stored on GPU memory. Any 
chunks with existing meshes that leave this distance have their mesh, vertices and indices
destroyed and the buffers removed from the GPU. Implemented custom move assignment operators
and constructors to prevent the buffers being duplicated. (Was the first time I ever played
with move semantics and added RAII to a class. Learned quite a bit from this).
- "Render Distance" - Distance at which chunks are rendered
- Creation Distance > Mesh Distance > Render Distance
- "Block activity updating" - Set blocks to being inactive (not rendered) when they are 
surrounded on all six sides by opaque blocks. This is the greatest bottleneck when moving
from one chunk to another since it is checked when chunks are created
- As a self note, block activity should be updated when the following conditions are met:
- - Chunk is created (implemented)
- - Chunk is modified (not yet implemented)
- - Neighbouring chunk is created (not yet implemented)
- - Neighbouring chunk is modified (not yet implemented)

## [Unreleased] - 2026-04-28

Here will write a vague roadmap of the upcoming updates and changes to the project.
Not supposed to be very rigid and features will move from one planned update to the next.
Some features will be pushed further down the line, others will arrive earlier than expected.
I suspect many changes won't be planned for entirely.

### [0.1.5]

This partway update is meant to leave the chunk mesh generation and other graphical 
things in a good enough state such that other features can be focused on (such as what
differentiates this project from any other voxel engine)

Since the focus is on performance and finishing the groundwork the following features are
on the menu:

- Use a separate thread for generating meshes with a preference for nearby chunks (potentially
adding extra preference for the ones in front of the camera)
- Greedy meshing potentially???
- Other strategies for more efficiency???
- Clean up the code once groundwork done

### [Future]

Better Lighting, Walking and Jumping, Construct arena / terrain, other physics, enemies,
simple gameplay mechanics (shooting and damaging enemies + losing health)