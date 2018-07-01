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

void VulkanWindow::changeNumInstancing( int value )
{
  _renderer->changeNumInstancing( value );
  //requestUpdate( );
}

void VulkanWindow::keyPressEvent( QKeyEvent* e )
{
	switch( e->key( ) )
	{
		case Qt::Key_A:
			std::cout << "A pressed" << std::endl;
			break;
		default:
			break;
	}
}
