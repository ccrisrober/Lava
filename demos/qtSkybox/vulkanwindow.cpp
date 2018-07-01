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

void VulkanWindow::styleSwitched( bool enable )
{
  _renderer->modeReflect = enable;
  requestUpdate( );
}

void VulkanWindow::changeModel( const QString& modelName )
{
  std::cout << modelName.toStdString( ) << std::endl;
  _renderer->setModel( modelName.toStdString( ) );
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

void VulkanWindow::togglePaused( void )
{
  setContinuousRendering( !isContinuosRendering( ) );
}