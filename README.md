# Vulkan C++ 3D Project

A C++ project developed with [Vulkan®](https://www.khronos.org/vulkan/), the new generation graphics and compute API from Khronos.

## Introduction

This is a Vulkan project where some rendering techniques are applied. Some of them are 
[Blinn-Phong](https://github.com/smil3yZzZ/vulkan3d/commit/b9dcaa9fd51d024ceab4732db91030509f21a6fd) lightning, 
[deferred rendering](https://github.com/smil3yZzZ/vulkan3d/commit/54e6383c0692e790883b05d29551cf3bb690e314) (with subpasses),
shadow mapping (both [unidirectional](https://github.com/smil3yZzZ/vulkan3d/commit/21267e9dd409fa12e49b7b0baa968826467e934b) 
and [omnidirectional](https://github.com/smil3yZzZ/vulkan3d/commit/b3bd7c0c63eadd913ab84f7d533d912ebb53dd19)) 
and [screen-space reflections](https://github.com/smil3yZzZ/vulkan3d/tree/feature/screen_space_reflections) (under development). In addition, future developments include PCF for shadows, FXAA,
and screen space ambient occlusion. Some CPU multithreading has been tested while developing omnidirectional shadow mapping. Master branch owns all the
features/techniques done until this moment, being this project an incremental development.

## Techniques breakthrough

Techniques developed in this project are applied inside a box made of 6 planes. Inside this box there exist 3 colored towers.

### Deferred rendering + Blinn-Phong lightning

In this technique, we create 2 subpasses which compound the main render pass.

### Unidirectional shadow mapping

### Omnidirectional shadow mapping

### Screen space reflections (under development)

## Credits

Thanks to the following people:
- [Brendan Galea](https://www.youtube.com/watch?v=Y9U9IE0gVHA&list=PL8327DO66nu9qYVKLDmdLW_84-yE4auCR) for teaching like a boss for free
- Sascha Willems for his fantastic [code samples](https://github.com/SaschaWillems/Vulkan)
- Adam Sawicki (from AMD) for sharing the [Vulkan Memory Allocator](https://gpuopen.com/vulkan-memory-allocator/)
- Lou Kramer (from AMD) for giving me tips and feedback
- Baldur Karlsson for creating [RenderDoc](https://renderdoc.org/) and avoiding multiple developers' headaches
- [David Lettier](https://lettier.github.io/) for his well documented tutorials
- In general, the whole Khronos Group for Vulkan Specification