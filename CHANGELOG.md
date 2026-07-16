# Change Log
All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](http://keepachangelog.com/)
and this project adheres to [Semantic Versioning](http://semver.org/).

## [Released] - 2026-07-15

In this section, I'll be writing the series of updates and changes of the project made
starting with 0.1.0. I hope to remember all the changes I make as well as the reasons behind
them as it'll make for a better change log.

### [0.2.2]

Update 0.2.2 

The aim primary aim was to update the meshes of chunks when neighbours were created. I thought this update wouldn't take as long but the challenge was a little bigger than I thought it would be and I was on holiday. Changes made were:
- Add occupancy data of a chunk's neighbour to a chunk when a neighbour is generated.
- Update the existing mesh of a chunk when the occupancy data is updated.
- Use a hash map to keep track of the chunks that have been generated but are not yet in the primary hash map that stores all (most) chunks.
- Tweak functions and function names.
- Tidy up code a little

### [0.2.0]

This update was originally intended to be numbered as "0.1.5" to represent a small update focused on performance and little else. While the focus has stayed consistent, the time and effort changed from "small".

As stated, the focus of this update was on performance with there being three primary changes:
 - (Main focus) Chunk data generation using a helper thread to prevent stutters when loading in new chunks.
 - Packing position vertex data into a single int (short) to reduce unneccessary memory usage.
 - Using binary bitmasks to cull block faces leading to faster chunk vertex generation and fewer overall vertices.

More detailed explanation of the above:
- Concurrently genereated chunks: 
  - Camera moves into a new chunk, main thread generates a queue of chunks that need to be created and meshes that need to be created. Chunks and meshes that need to be destroyed are also added to queues.
  - If the helper thread is available for work, pass those queues to queue structures intended only for the helper thread to use during its tasks
  - Dispatch the helper thread to create chunks and chunk vertices (not meshes, that will be done by the main thread to keep OpenGL calls to the main thread). Created chunks are held in a separate array for now while vertices are kept on a back buffer of each chunk
  - Every update loop, main thread checks if the helper thread has finished its tasks. Once it is done, pass the created chunks onto the hashmap, swap the vertex back buffers with the front buffers, and generate the meshes

- Packing position vertex data:
  - Block vertices for each chunk can only have 17 possible values from 0 to 16 meaning position in each axis can be stored using 5 bits. Position can therefore be stored using 16 bit integers.
  - Use a struct VertexData to pass the 16-bit int and the vec3 containing colour information to the GPU.

- Face culling using bitmasks
  - By storing the occupancy data of a row of blocks as bits in an integer, can bitshift the integer one bit to the left or right to check if a block has a neighbour on the left or right
  - This not only checks for one block in the row but the entire row (assuming the sides are padded)
  - Can extend this logic to the y and z axes, storing the occupancy data of the blocks in a chunk in three different arrays of integers for the x, y and z directions
  - Allows for efficient culling of block faces
  - For a far better, visual explanation: [Incredible Voxel Mesh Optimisations! - Daniel P H Fox](https://www.youtube.com/watch?v=LxVLqCiDqd8&t=402s)

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
  - Chunk is created (implemented)
  - Chunk is modified (not yet implemented)
  - Neighbouring chunk is created (not yet implemented)
  - Neighbouring chunk is modified (not yet implemented)

## [Unreleased] - 2026-04-28

Here will write a vague roadmap of the upcoming updates and changes to the project.
Not supposed to be very rigid and features will move from one planned update to the next.
Some features will be pushed further down the line, others will arrive earlier than expected.
I suspect many changes won't be planned for entirely.

### [0.3.0]

- Tall chunks (not just flat)
- Will need to add ways to modify things for individual chunks and not just entire chunks stacks (if necessary)

### [0.3.5]

- Terrain generation
- Lighting
- Look into more efficient ways to construct the vertices of a chunk with colour data

### [Future]

Better debugging UI e.g. FPS counter, button to change polygon mode, better lighting, walking and jumping, construct arena / terrain, other physics, enemies,
simple gameplay mechanics (shooting and damaging enemies + losing health). Changing occupancy data if chunks are
modified. Changing occupancy data for neighbours of modified chunks.