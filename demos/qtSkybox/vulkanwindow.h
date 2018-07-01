#pragma once

#include <qtLava/qtLava.h>

class Renderer;

class VulkanWindow: public lava::QtVulkanWindow
{
public:
	VulkanWindow( void );

	lava::QtVulkanWindowRenderer* createRenderer( void ) override;

public slots:
  void togglePaused( void );
  void styleSwitched( bool enable );
  void changeModel( const QString& modelName );

private:
	void keyPressEvent( QKeyEvent* ) override;

	Renderer* _renderer;
};