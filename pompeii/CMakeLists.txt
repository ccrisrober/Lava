option(POMPEII_IMPLEMENT_VMA "Compile implementation of Vulkan Memory Allocator." ON)

if(POMPEII_IMPLEMENT_VMA)
  add_definitions(-DPOMPEII_IMPLEMENT_VMA)
endif()

source_group(\\ FILES CMakeLists.txt)

set( POMPEII_PUBLIC_HEADERS
  includes.hpp
  noncopyable.hpp
  
  Display.h

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
  Texture3D.h
  Queue.h
  QueryPool.h
  Framebuffer.h
  Pipeline.h

  utilities.hpp

  VulkanResource.h

### engine/Clock.h
### engine/Material.h
### engine/Node.h

### extras/Geometry.h
### extras/Material.h
### extras/Mesh.h
### extras/ModelImporter.h
)

set( POMPEII_SOURCES
  Display.cpp
  
  Buffer.cpp
  CommandBuffer.cpp
  Descriptor.cpp
  Instance.cpp
  Device.cpp
  PhysicalDevice.cpp
  Event.cpp
  Fence.cpp
  Log.cpp

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
  Texture3D.cpp
  Queue.cpp
  QueryPool.cpp
  Framebuffer.cpp
  Pipeline.cpp

  utilities.cpp

  VulkanResource.cpp

###  engine/Clock.cpp
###  engine/Material.cpp
###  engine/Node.cpp

###  extras/Geometry.cpp
###  extras/Material.cpp
###  extras/Mesh.cpp
###  extras/ModelImporter.cpp
)

set( POMPEII_HEADERS
)

set( POMPEII_MOC_HEADERS
)

set( POMPEII_LINK_LIBRARIES
	${VULKAN_LIBRARY}
	${STBI_LIBRARIES}
)

common_library( pompeii )
