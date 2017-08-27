set( Name vklava )
set( NAME VKLAVA )
set( namespace lava )
set( namespace lava )
set( INCLUDE_NAME vklava )
configure_file( ${CMAKE_SOURCE_DIR}/CMake/common/cpp/api.h
  ${PROJECT_BINARY_DIR}/include/vklava/api.h @ONLY )
configure_file( ${CMAKE_SOURCE_DIR}/CMake/common/cpp/version.h
  ${PROJECT_BINARY_DIR}/include/vklava/version.h @ONLY )
configure_file( ${CMAKE_SOURCE_DIR}/CMake/common/cpp/version.cpp
  ${PROJECT_BINARY_DIR}/src/vklava/version.cpp @ONLY )

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/routes.h.in
  ${CMAKE_CURRENT_BINARY_DIR}/routes.h)

set( VKLAVA_SOURCES
  Instance.cpp
  PhysicalDevice.cpp
  Surface.cpp
  VulkanApp.cpp
  Device.cpp
  VulkanResource.cpp
  Semaphore.cpp
  Queue.cpp
  RenderPass.cpp
  FramebufferSwapchain.cpp
  Swapchain.cpp
  Image.cpp
  Framebuffer.cpp
  CommandBuffer.cpp
  Pipeline.cpp
  Descriptor.cpp
  Sampler.cpp
  Buffer.cpp

	Source.cpp
)

set( VKLAVA_HEADERS
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
  FramebufferSwapchain.h
  Swapchain.h
  Image.h
  Framebuffer.h
  CommandBuffer.h
  Pipeline.h
  Descriptor.h
  Sampler.h
  Buffer.h
)

set( VKLAVA_MOC_HEADERS
)

include_directories( ${CMAKE_CURRENT_BINARY_DIR} )

set( VKLAVA_LINK_LIBRARIES
	${VULKAN_LIBRARY}
  	${GLFW3_LIBRARIES}
  	${GLM_LIBRARIES}
  	${STBI_LIBRARIES}
  	${ASSIMP_LIBRARIES}
    ${GLSLANG_LIBRARIES}
)

if ( MSVC )
  add_definitions( -DVKLAVA_STATIC )
endif( )

common_application( vkLava )
