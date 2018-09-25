#source_group(\\ FILES CMakeLists.txt)

set( GLFWPOMPEII_PUBLIC_HEADERS
  GLFWVulkanWindow.h
)

set( GLFWPOMPEII_SOURCES
  GLFWVulkanWindow.cpp
)

set( GLFWPOMPEII_HEADERS
)

set( GLFWPOMPEII_MOC_HEADERS
)

set( GLFWPOMPEII_LINK_LIBRARIES
  ${GLFW3_LIBRARIES}
	pompeii
)
set(GLFWPOMPEII_INCLUDE_NAME glfwPompeii)
set(GLFWPOMPEII_NAMESPACE glfwPompeii)
common_library( glfwPompeii )
