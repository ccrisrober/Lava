#include "renderer.h"

Renderer::Renderer( VulkanWindow* w )
	: _window( w )
{
  _window->setContinuousRendering( true );

  camera = Camera( glm::vec3( 0.0f, 2.0f, 8.0f ) );
}