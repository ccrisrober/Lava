#source_group(\\ FILES CMakeLists.txt)

set( LAVARENDERER_PUBLIC_HEADERS
  includes.hpp
  DefaultFramebuffer.h
  VulkanRenderer.h
  Window.h

  io/Input.hpp
  io/Keyboard.hpp
  io/Mouse.hpp
)

set( LAVARENDERER_SOURCES
  DefaultFramebuffer.cpp
  VulkanRenderer.cpp
  Window.cpp

  io/Input.cpp
  io/Keyboard.cpp
  io/Mouse.cpp
)

set( LAVARENDERER_HEADERS
)

set( LAVARENDERER_MOC_HEADERS
)

set( LAVARENDERER_LINK_LIBRARIES
  ${GLFW3_LIBRARIES}
	lava
)
set(LAVARENDERER_INCLUDE_NAME lavaRenderer)
set(LAVARENDERER_NAMESPACE lavaRenderer)
common_library( lavaRenderer )
