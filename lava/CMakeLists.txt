source_group(\\ FILES CMakeLists.txt)

set( LAVA_SOURCES
  Instance.cpp
  PhysicalDevice.cpp
  Surface.cpp
  VulkanApp.cpp
  Device.cpp
  VulkanResource.cpp
  Semaphore.cpp
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

  Texture.cpp

	#Source.cpp
)

set( LAVA_PUBLIC_HEADERS
  includes.hpp
  ../Vulkan-Hpp/vulkan/vulkan.hpp
  RenderAPICapabilites.h
  noncopyable.hpp

  Instance.h
  PhysicalDevice.h
  Surface.h
  VulkanApp.h
  Device.h
  VulkanResource.h
  Semaphore.h
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

  Texture.h

  extras/Mesh.h
  extras/ModelImporter.h
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
