#include "renderer.h"

Renderer::Renderer( VulkanWindow* w )
	: _window( w )
{
  camera = Camera( glm::vec3( 0.0f, 2.0f, 3.5f ),
    glm::vec3( 0.0f, 1.0f, 0.0f ), YAW, -25.0f );
}