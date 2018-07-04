# Examples

## Basics

### [Clear Screen](./ClearScreen.cpp)
<img src="../screenshots/clear_screen.png" height="82px" align="right">

Most basic example to demonstrate the functioning of the Vulkan graphic channel. Simple demo that shows how to change the cleaned color, similar to ```glClearColor``` in Vulkan.

### [Triangle](./TriangleNoBuffer.cpp)
<img src="../screenshots/triangle_noBuffer.png" height="82px" align="right">

Extending the example of [Clear Screen](./ClearScreen.cpp), this demo shows you how to create your first Graphic Pipeline in order to draw a triangle, whose vertices are hard-coded in the vertex shader.

### [Triangle tesselated](./TriangleIndexed.cpp)
<img src="../screenshots/triangle_indexed_tesselated.png" height="82px" align="right">

Demo that shows how to paint a triangle using or not indexing (``` #define INDEXING_MODE ```). It also allows you to paint a triangle using the tesselation technique ( ```#define TESS_MODE``` ).

We can change the way to draw the commented triangle or by decomposing the definition of the variables ```INDEXING_MODE``` to draw an indexed triangle and ```TESS_MODE``` which applies the technique of tiling using a wireframe mode to check the new vertices generated.

### [Cube Textured](./CubeTextured.cpp)
<img src="../screenshots/cube_textured.png" height="82px" align="right">

Basic application that shows how to create two buffers in GPU memory using staging buffer and a 2D texture in order to draw an indexed cube with texture.


## Advanced
### [Sampler as an uniform](./SamplerUniformQuad.cpp)
<img src="../screenshots/sampler_uniform.png" height="82px" align="right">

In this example we demonstrate how to paint a full-screen plane in order to demonstrate how to use sampler-type variables as a uniform within the pipeline and descriptor.


### [Secondary Buffer Quad](./SecondaryBuffer.cpp)
<img src="../screenshots/secondary_buffer.png" height="82px" align="right">

Basic example to demonstrate how to generate secondary CommandBuffer which facilitates the refactoring of CommandBuffer.

Before starting the rendering and update loop we generated this secondary CommandBuffer recording the Pipeline bind, descriptors and the painting. Finally, in the rendering loop the main CommandBuffer is the one that will execute all the secondary CommandBuffer.


### [Skybox with refraction and reflection](./Skybox.cpp)
<img src="../screenshots/skybox_reflection_refraction.png" height="82px" align="right">

Demo that shows a skybox next to a model on which the reflection and refraction technique is applied (with the ```Z``` and ```X``` keys).

The model is loaded through the [assimp](https://github.com/assimp/assimp) library, which allows the loading of meshes in different formats, such as .OBJ and .PLY.


### [Planar reflection with Stencil Buffer](./PlanarReflection.cpp)
<img src="../screenshots/planar_reflection.png" height="82px" align="right">

Demo that recreates the technique of planar reflection using the stencil buffer. This kind of buffer is an optional extension of the depth buffer that gives you more control over the question of which fragments should be drawn and which shouldn't.

Like the depth buffer, a value is stored for every pixel, but this time you get to control when and how this value changes and when a fragment should be drawn depending on this value. Note that if the depth test fails, the stencil test no longer determines whether a fragment is drawn or not, but these fragments can still affect values in the stencil buffer!


### [Plane instancing with blending](./Instancing.cpp)
<img src="../screenshots/plane_instancing_blending.png" height="82px" align="right">

This demo shows a series of planes indexed and instantiated with active blending. Thanks to the instantiation technique we can draw a greater amount of geometry in a single call.


### [Matcap with 2D array](./Matcap2DArray.cpp)
<img src="../screenshots/matcap_2DArray.png" height="82px" align="right">

Application that shows how to apply different Matcap using a 2D Array texture. This type of texture requires that all images have the same size.


### [Pipeline Derivation with SpecializationInfo](./MeshDerivationSpecialization.cpp)
<img src="../screenshots/pipeline_derivation_specialization.png" height="82px" align="right">

Example of how to create Graphics Pipelines using PipelineCache and Pipelines derivation flags.

Finally, this demo demonstrates the use of SPIR-V specialization constants used to specify shader constants at pipeline creation time. 

### [Earth](./Earth.cpp)
<img src="../screenshots/earth.png" height="82px" align="right">

Foo

### [Cemetery Dynamic Uniform Buffer](./CementeryScene.cpp)
<img src="../screenshots/cemetery_dub.png" height="82px" align="right">

Foo


### [Deferred Shading](./DeferredShading.cpp)
<img src="../screenshots/deferred_shading.png" height="82px" align="right">

Foo


### [MultiSetDescriptor](./MultiSetDescriptor.cpp)
<img src="../screenshots/multiset_descriptor.png" height="82px" align="right">

Foo


### [Stencil Toon Outline](./StencilToonOutline.cpp)
<img src="../screenshots/stencil_toon_outline.png" height="82px" align="right">

Foo


### [Swap Quad Texture](./SwapQuadTexture.cpp)
<img src="../screenshots/swap_quad_texture.png" height="82px" align="right">

Foo

## Geometry Shader
### [Rotation 2D Figures](./GeometryFigures.cpp)
<img src="../screenshots/geometry_figures.png" height="82px" align="right">

Example to demonstrate how to generate convex polygons through a geometry shader. This shader receives a list of vertices whose information contains the center of the polygon, the color and the number of sides.

### [Mesh Explosion](./MeshExplosion.cpp)
<img src="../screenshots/mesh_explosion.png" height="82px" align="right">

In this demo we paint a mesh whose triangles "explode" according to the normal of the triangle as if it were an explosion.

Within the geometry shader, to avoid that the triangles are lost in scene, a trigonometric function is executed to animate the different triangles that make up the mesh.

### [Mesh Normals](./MeshNormals.cpp)
<img src="../screenshots/mesh_normals.png" height="82px" align="right">

TODO

### [Clip Planes](./ClipPlanes.cpp)
<img src="../screenshots/clip_planes.png" height="82px" align="right">

TODO

### [Billboard with Geometry Shader](./BillboardGeometry.cpp)
<img src="../screenshots/geometry_billboards.png" height="82px" align="right">

Simple example where we demonstrate how to apply the Billboard Particles technique using a geometry shader. The images used are inside a Texture2DArray.

Copyright: Golden Sun (Camelot Software Planning)

### [Butterflies in a grassland](./ButterfliesGrassland.cpp)
<img src="../screenshots/butterflies_grassland.png" height="82px" align="right">

In this complete example, it shows you how to draw grass animated by the wind applying the blending functions of the Graphic Pipeline. On the other hand, a collection of butterflies based on the popular butterfly "Butterfree" of Pokémon are also drawn which flap their wings. Both the grass and the butterflies are drawn from a collection of points that mark the center of the geometries and through the Geometry stage we generate the rest of the triangles. Finally, a simple Skybox is drawn to add a better atmosphere to the scene.


### [MultiViewport Perfect Toon](./MultiViewportToon.cpp)
<img src="../screenshots/multiviewport_toon.png" height="82px" align="right">

Foo


## Tesselations Shader
### [Heightmap with Tesselation Shader](./HeightmapTesselation.cpp)
<img src="../screenshots/heightmap_tesselation.png" height="82px" align="right">

Through this example we show how to draw a plane to which the tiling shaders are applied with the drawing of a Heightmap that represents a map of the planet Earth. To represent this Heightmap two textures are used: The first represents the diffuse map of the Earth and the second represents the map of heights. Both the height and the level of tessellation are configurable through the keyboard.


## Compute Pipelines
*Compute shaders are mandatory in Vulkan and must be supported on all devices*

### [Compute copy](./ComputeCopy.cpp)
<img src="../screenshots/compute_copy.png" height="82px" align="right">

Example of how to perform a vector copy using compute shaders.


### [Compute sum](./ComputeSum.cpp)
<img src="../screenshots/compute_sum.png" height="82px" align="right">

Example of how to perform mathematical operations on vectors using compute shaders.


### [Cube with computed fractal](./FractalCompute.cpp)
<img src="../screenshots/cube_computed_fractal.png" height="82px" align="right">

A compute shader generates a [Mandelbrot Fractal](https://en.wikipedia.org/wiki/Mandelbrot_set) and this texture is applied over a mesh.

The idea of this demo is to create a Computation Pipeline whose task is to generate a texture that represents the well-known Mandelbrot Fractal with dynamic center, that is, using a Uniform Buffer we determine in real time the center of this fractal. Finally, this texture is applied on an indexed cube that rotates.
