source_group(\\ FILES CMakeLists.txt)

set( LAVA_PUBLIC_HEADERS
  includes.hpp
  noncopyable.hpp
  
  Buffer.h
  CommandBuffer.h
  Descriptor.h
  Instance.h
  Device.h
  PhysicalDevice.h
  Event.h
  Fence.h
  Log.h
  MemoryUtils.h
  StringUtils.hpp
  RenderAPICapabilites.h

  DefaultFramebuffer.h

  Image.h
  RenderPass.h
  Surface.h
  Sampler.h
  Semaphore.h
  Swapchain.h
  Texture.h
  Texture1D.h
  Texture2D.h
  TextureCubemap.h
  Texture2DArray.h
  #Texture3D.h
  Queue.h
  Framebuffer.h
  Pipeline.h

  utils.hpp

  VulkanResource.h
  VulkanRenderer.h
  Window.h

  engine/Clock.h
  engine/Material.h
  engine/Node.h

  extras/Geometry.h
  extras/Material.h
  extras/Mesh.h
  extras/ModelImporter.h

  io/Input.hpp
  io/Keyboard.hpp
  io/Mouse.hpp
)

set( LAVA_SOURCES
  Buffer.cpp
  CommandBuffer.cpp
  Descriptor.cpp
  Instance.cpp
  Device.cpp
  PhysicalDevice.cpp
  Event.cpp
  Fence.cpp
  Log.cpp

  DefaultFramebuffer.cpp

  Image.cpp
  RenderPass.cpp
  Surface.cpp
  Sampler.cpp
  Semaphore.cpp
  Swapchain.cpp
  Texture.cpp
  Texture1D.cpp
  Texture2D.cpp
  TextureCubemap.cpp
  Texture2DArray.cpp
  #Texture3D.cpp
  Queue.cpp
  Framebuffer.cpp
  Pipeline.cpp

  utils.cpp

  VulkanResource.cpp
  VulkanRenderer.cpp
  Window.cpp

  engine/Clock.cpp
  engine/Material.cpp
  engine/Node.cpp

  extras/Geometry.cpp
  extras/Material.cpp
  extras/Mesh.cpp
  extras/ModelImporter.cpp

  io/Input.cpp
  io/Keyboard.cpp
  io/Mouse.cpp
)

set( LAVA_HEADERS
)

set( LAVA_MOC_HEADERS
)

set( LAVA_LINK_LIBRARIES
	${VULKAN_LIBRARY}
	${GLFW3_LIBRARIES}
	${GLM_LIBRARIES}
	${STBI_LIBRARIES}
  ${ASSIMP_LIBRARIES}
)

common_library( lava )
