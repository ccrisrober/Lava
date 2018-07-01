#include "renderer.h"

Renderer::Renderer( VulkanWindow* w )
	: _window( w )
{
  _window->setContinuousRendering( true );
}