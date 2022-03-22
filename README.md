# Vulkan C++ 3D Project

A C++ project developed with [Vulkan®](https://www.khronos.org/vulkan/), the new generation graphics and compute API from Khronos.

## Introduction

This is a Vulkan project where some rendering techniques are applied. Some of them are 
[Blinn-Phong](https://github.com/smil3yZzZ/vulkan3d/commit/b9dcaa9fd51d024ceab4732db91030509f21a6fd) lightning, 
[deferred rendering](https://github.com/smil3yZzZ/vulkan3d/commit/54e6383c0692e790883b05d29551cf3bb690e314) (with subpasses),
shadow mapping (both [unidirectional](https://github.com/smil3yZzZ/vulkan3d/commit/21267e9dd409fa12e49b7b0baa968826467e934b) 
and [omnidirectional]()) 
and [screen-space reflections](https://github.com/smil3yZzZ/vulkan3d/tree/feature/screen_space_reflections). In addition, future developments include PCF for shadows, FXAA,
and screen space ambient occlusion. Some CPU multithreading has been tested while developing omnidirectional shadow mapping. Master branch owns all the
features/techniques done until this moment, being this project an incremental development.

## Breakthrough

Khronos recently made an official Vulkan Samples repository available to the public ([press release](https://www.khronos.org/blog/vulkan-releases-unified-samples-repository?utm_source=Khronos%20Blog&utm_medium=Twitter&utm_campaign=Vulkan%20Repository)).

You can find this repository at https://github.com/KhronosGroup/Vulkan-Samples

As I've been involved with getting the official repository up and running, I'll be mostly contributing to that repository from now, but may still add samples that don't fit there in here and I'll of course continue to maintain these samples.

## Credits

Thanks to the following people:
- [Brendan Galea](https://www.youtube.com/watch?v=Y9U9IE0gVHA&list=PL8327DO66nu9qYVKLDmdLW_84-yE4auCR) for teaching like a boss for free
- Sascha Willems for his fantastic [code samples](https://github.com/SaschaWillems/Vulkan)
- Adam Sawicki (from AMD) for sharing the [Vulkan Memory Allocator](https://gpuopen.com/vulkan-memory-allocator/)
- Lou Kramer (from AMD) for giving me tips and feedback
- [David Lettier](https://lettier.github.io/) for his well documented tutorials
- In general, the whole Khronos Group for Vulkan Specification