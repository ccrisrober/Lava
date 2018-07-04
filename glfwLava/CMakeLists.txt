#source_group(\\ FILES CMakeLists.txt)

set( GLFWLAVA_PUBLIC_HEADERS
  GLFWVulkanWindow.h
)

set( GLFWLAVA_SOURCES
  GLFWVulkanWindow.cpp
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
