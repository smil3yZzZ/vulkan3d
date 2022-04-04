# Vulkan C++ 3D Project

A C++ project developed with [Vulkan®](https://www.khronos.org/vulkan/), the new generation graphics and compute API from Khronos.

## Introduction

This is a Vulkan project where some rendering techniques are applied. Some of them are 
[Blinn-Phong](https://github.com/smil3yZzZ/vulkan3d/commit/b9dcaa9fd51d024ceab4732db91030509f21a6fd) lighting, 
[deferred rendering](https://github.com/smil3yZzZ/vulkan3d/commit/54e6383c0692e790883b05d29551cf3bb690e314) (with subpasses),
shadow mapping (both [unidirectional](https://github.com/smil3yZzZ/vulkan3d/commit/21267e9dd409fa12e49b7b0baa968826467e934b) 
and [omnidirectional](https://github.com/smil3yZzZ/vulkan3d/commit/b3bd7c0c63eadd913ab84f7d533d912ebb53dd19)) 
and [screen-space reflections](https://github.com/smil3yZzZ/vulkan3d/tree/feature/screen_space_reflections).
Currently, tile-based lighting is being developed with the use of compute shaders and multiple lights. In addition, future developments 
include PCF for shadows, FXAA (MSAA has been discarded since deferred rendering is being used) and Screen Space Ambient Occlusion. 
Some CPU multithreading has been tested while developing omnidirectional shadow mapping. 
Master branch owns all the features/techniques done until this moment, being this project within an incremental development.

## Techniques breakthrough

Techniques developed in this project are applied inside a box made of 6 planes. Inside this box there exist 3 colored towers. Also, a billboard has been created
in order to represent a point light in a separate render pass (we will avoid this in the scheme for the sake of simplicity).

![Scene overview (omnidirectional shadows, not unidirectional)](/renderDocCapture.png "Scene overview (omnidirectional shadows, not unidirectional)")

### Deferred rendering + Blinn-Phong lighting

In order to perform deferred rendering, we create 2 subpasses which compound the main render pass: G-buffer subpass and composition subpass.
The first subpass represents the G-buffer, which calculates per-fragment albedo, normal and depth. These layouts are sent to the composition subpass where
a quad of 6 vertices is created in the vertex buffer. Along with the previously g-buffered layouts, the lighting is computed in the fragment shader. It is 
critical to reconstruct the world position from depth before doing any calculations. The design and some RenderDoc screenshots are shown in the figures below.

![Deferred rendering + Blinn-Phong lighting design](/deferred_structure.png "Deferred rendering + Blinn-Phong lighting design")

![G-Buffer screenshot](/gBuffer.png "G-Buffer screenshot")

![Composition screenshot](/composition.png "Composition screenshot")

### Unidirectional shadow mapping

The unidirectional shadow mapping technique consisted in adding to the previous design a renderpass before the main one. In this renderpass a depth map is 
calculated from the point of view of the light by using a new projection-view matrix. In composition subpass, the mapped texture of the previous render pass
is loaded and shadows are checked. The design and the result are shown in the figures below.

![Unidirectional shadows design](/shadow_unidir_structure.png "Unidirectional shadows design")

![Unidirectional shadows scene](/unidir_scene.png "Unidirectional shadows scene")

### Omnidirectional shadow mapping

The process for omnidirectional shadow mapping technique has similarities to the unidirectional technique. A new renderpass is created, but we create
a cube texture -by using the Multiview extension- instead of a depth map in order to sample the distances from light for all directions. In the composition subpass, the distances are compared 
per-fragment so the existence of shadows is checked. The result has been previously shown in the scene overview.

![Omnidirectional shadows design](/shadow_omnidir_structure.png "Omnidirectional shadows design")

### Screen space reflections

This process take more render passes than before in order to create several textures that need to be used in the composition. These are the following:
- Mappings Render Pass: It generates a 2D Array texture in order to get view space positions and normals.
- UV Render Pass: It generates a 2D texture map with UV positions being reflected per fragment, considering a push constant variable
that represents per-fragment reflection. Here, ray marching is applied by using the previously calculated
view space positions and normals.
- Post Processing Render Pass: Retrieves the color 2D texture from Deferred Render Pass along with the UV positions texture
from UV Render Pass. It applies per-fragment reflections.

![Screen space reflections design](/refl_scene.png "Screen space reflections design")
![Screen space reflections design](/refl_scene2.png "Screen space reflections design")

The scheme implemented is described in the image below. Future improvements consider blurring and a specular map to represent
brightness in a more realistic way.

![Screen space reflections design](/screen_space_refl_design.png "Screen space reflections design")

### Tile-based lighting (under development)

This kind of lighting involves the use of compute shaders in order to paralellize the lighting calculation by dividing the screen into
tiles (quads of same size). Currently, some tests are being performed with compute shaders in order to think about the correct design.

## Credits

Thanks to the following people:
- [Brendan Galea](https://www.youtube.com/watch?v=Y9U9IE0gVHA&list=PL8327DO66nu9qYVKLDmdLW_84-yE4auCR) for teaching like a boss for free
- Sascha Willems for his fantastic [code samples](https://github.com/SaschaWillems/Vulkan)
- Adam Sawicki (from AMD) for sharing the [Vulkan Memory Allocator](https://gpuopen.com/vulkan-memory-allocator/)
- Lou Kramer (from AMD) for giving me tips and feedback
- Baldur Karlsson for creating [RenderDoc](https://renderdoc.org/) and avoiding multiple developers' headaches
- [David Lettier](https://lettier.github.io/) for his well documented tutorials
- In general, the whole Khronos Group for the Vulkan Specification