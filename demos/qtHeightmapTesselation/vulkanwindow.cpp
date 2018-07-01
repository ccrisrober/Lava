#include "vulkanwindow.h"

#include "renderer.h"

#include <QKeyEvent>

VulkanWindow::VulkanWindow( void )
{
}

lava::QtVulkanWindowRenderer* VulkanWindow::createRenderer( void )
{
	_renderer = new Renderer( this );
	return _renderer;
}

void VulkanWindow::changeAmount( int value )
{
  _renderer->changeAmount( value );
  requestUpdate( );
}

void VulkanWindow::changeTessLevel( int value )
{
  _renderer->changeTessLevel( value );
  requestUpdate( );
}

void VulkanWindow::enableWireframe( bool enable )
{
  _renderer->enableWireframe( enable );
  requestUpdate( );
}