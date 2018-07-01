#pragma once

#include <qtLava/qtLava.h>

class Renderer;

class VulkanWindow: public lava::QtVulkanWindow
{
public:
	VulkanWindow( void );

	lava::QtVulkanWindowRenderer* createRenderer( void ) override;

public slots:
  void changeAmount( int value );
  void changeTessLevel( int value );
  void enableWireframe( bool enable );

private:
	Renderer* _renderer;
};