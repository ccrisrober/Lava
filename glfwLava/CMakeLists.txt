#source_group(\\ FILES CMakeLists.txt)

set( GLFWLAVA_PUBLIC_HEADERS
  Engine.h
  GLFWVulkanWindow.h
  ##includes.hpp
  ##DefaultFramebuffer.h
  ##VulkanRenderer.h
  ##Window.h

  ##io/Input.hpp
  ##io/Keyboard.hpp
  ##io/Mouse.hpp
)

set( GLFWLAVA_SOURCES
  Engine.cpp
  GLFWVulkanWindow.cpp
  ##DefaultFramebuffer.cpp
  ##VulkanRenderer.cpp
  ##Window.cpp

  ##io/Input.cpp
  ##io/Keyboard.cpp
  ##io/Mouse.cpp
)

set( GLFWLAVA_HEADERS
)

set( GLFWLAVA_MOC_HEADERS
)

set( GLFWLAVA_LINK_LIBRARIES
  ${GLFW3_LIBRARIES}
	lava
)
set(GLFWLAVA_INCLUDE_NAME glfwLava)
set(GLFWLAVA_NAMESPACE glfwLava)
common_library( glfwLava )
