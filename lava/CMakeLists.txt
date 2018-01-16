source_group(\\ FILES CMakeLists.txt)

set( LAVA_PUBLIC_HEADERS
  includes.hpp

  io/Keyboard.hpp
  io/Mouse.hpp
  io/Input.hpp

  RenderAPICapabilites.h
  noncopyable.hpp

  Instance.h
  Event.h
  PhysicalDevice.h
  Surface.h
  VulkanApp.h
  Device.h
  VulkanResource.h
  Semaphore.h
  Fence.h
  Queue.h
  RenderPass.h
  DefaultFramebuffer.h
  Swapchain.h
  Image.h
  Framebuffer.h
  CommandBuffer.h
  Pipeline.h
  Descriptor.h
  Sampler.h
  Buffer.h
  QueryPool.h
##DeviceMemory.h

  Window.h

  Texture.h
  Texture1D.h
  Texture2D.h
  ##Texture2DArray.h
  TextureCubemap.h
  ##Texture3D.h

  extras/FpsController.h

  extras/Mesh.h
  extras/Material.h
  extras/ModelImporter.h
  extras/CustomFramebuffer.h
  extras/CustomMaterial.h
  extras/Geometry.h

  engine/Material.h
  engine/Node.h
  engine/Clock.h

  utils.hpp

  Log.h
  MemoryUtils.h
  StringUtils.hpp


  VulkanRenderer.h
)

set( LAVA_SOURCES

  io/Keyboard.cpp
  io/Mouse.cpp
  io/Input.cpp

  VulkanRenderer.cpp


  Instance.cpp
  Event.cpp
  PhysicalDevice.cpp
  Surface.cpp
  VulkanApp.cpp
  Device.cpp
  VulkanResource.cpp
  Semaphore.cpp
  Fence.cpp
  Queue.cpp
  RenderPass.cpp
  DefaultFramebuffer.cpp
  Swapchain.cpp
  Image.cpp
  Framebuffer.cpp
  CommandBuffer.cpp
  Pipeline.cpp
  Descriptor.cpp
  Sampler.cpp
  Buffer.cpp
  QueryPool.cpp
##DeviceMemory.cpp

  Window.cpp

  Texture.cpp
  Texture1D.cpp
  Texture2D.cpp
  ##Texture2DArray.cpp
  TextureCubemap.cpp
  ##Texture3D.cpp

  extras/Mesh.cpp
  extras/Material.cpp
  extras/ModelImporter.cpp
  extras/CustomFramebuffer.cpp
  extras/CustomMaterial.cpp
  extras/Geometry.cpp

  engine/Node.cpp
  engine/Material.cpp
  engine/Clock.cpp

	#Source.cpp
  utils.cpp

  Log.cpp
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
