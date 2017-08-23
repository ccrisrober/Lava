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
	RenderWindow.cpp
	VulkanBuffer.cpp
	VulkanCommandBuffer.cpp
	VulkanDevice.cpp
	VulkanFramebuffer.cpp
	VulkanQueue.cpp
	VulkanRenderAPI.cpp
	VulkanResource.cpp
	VulkanSwapChain.cpp
	VulkanTexture.cpp
	Source.cpp
	VulkanPipelineState.cpp
	VulkanSampler.cpp
	##VulkanDrawable.cpp
)

set( VKLAVA_HEADERS
	scenegraph/Mesh.h
	scenegraph/ModelImporter.h
	RenderAPICapabilites.h
	##vulkan.hpp
	routes.h
	noncopyable.hpp
	RenderWindow.h
	VulkanBuffer.h
	VulkanCommandBuffer.h
	VulkanDevice.h
	VulkanFramebuffer.h
	VulkanQueue.h
	VulkanRenderAPI.h
	VulkanResource.h
	VulkanSwapChain.h
	VulkanTexture.h
	VulkanPipelineState.h
	VulkanSampler.h
	##VulkanDrawable.h
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
)

if ( MSVC )
  add_definitions( -DVKLAVA_STATIC )
endif( )

common_application( vkLava )
