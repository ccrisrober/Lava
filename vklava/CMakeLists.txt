set( Name vklava )
set( NAME VKLAVA )
set( namespace vklava )
set( NAMESPACE VKLAVA )
set( INCLUDE_NAME vklava )
configure_file( ${CMAKE_SOURCE_DIR}/CMake/common/cpp/api.h
  ${PROJECT_BINARY_DIR}/include/vklava/api.h @ONLY )
configure_file( ${CMAKE_SOURCE_DIR}/CMake/common/cpp/version.h
  ${PROJECT_BINARY_DIR}/include/vklava/version.h @ONLY )
configure_file( ${CMAKE_SOURCE_DIR}/CMake/common/cpp/version.cpp
  ${PROJECT_BINARY_DIR}/src/vklava/version.cpp @ONLY )

set( VKLAVA_SOURCES
	RenderWindow.cpp
	VulkanCommandBuffer.cpp
	VulkanDevice.cpp
	VulkanFramebuffer.cpp
	VulkanQueue.cpp
	VulkanRenderAPI.cpp
	VulkanResource.cpp
	VulkanSwapChain.cpp
	VulkanTexture.cpp
	Source.cpp
)

set( VKLAVA_HEADERS
	noncopyable.hpp
	RenderWindow.h
	VulkanCommandBuffer.h
	VulkanDevice.h
	VulkanFramebuffer.h
	VulkanQueue.h
	VulkanRenderAPI.h
	VulkanResource.h
	VulkanSwapChain.h
	VulkanTexture.h
)

set( VKLAVA_MOC_HEADERS
)

include_directories( ${CMAKE_CURRENT_BINARY_DIR} )

set( VKLAVA_LINK_LIBRARIES
	${VULKAN_LIBRARY}
  	${GLFW3_LIBRARIES}
  	${GLM_LIBRARIES}
)

if ( MSVC )
  add_definitions( -DVKLAVA_STATIC )
endif( )

common_application( vkLava )
