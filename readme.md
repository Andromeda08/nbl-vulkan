## `nbl-vulkan`

A Vulkan framework targeting systems with GPUs that support Vulkan 1.4 and the Ray Tracing and Mesh Shading pipelines.

- Targeting a single "feature level"
  - Vulkan 1.4
  - Ray Tracing Pipeline and Ray Query support.
  - Mesh Shading Pipeline support.
  - GPUs with dedicated async compute queues.
  - Rendering to a window surface.
- Pipeline creation
  - Graphics, Compute and Ray Tracing (+ SBT creation)
  - Option for automatic DescriptorSet and PushConstant layout detection via [nbl-reflect](https://github.com/Andromeda08/nbl-reflect) and [spirv-reflect](https://github.com/KhronosGroup/SPIRV-Reflect.git).
- Memory management via [VulkanMemoryAllocator](https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator.git)