#pragma once

#include <qtLava/qtLava.h>

class Renderer;

class VulkanWindow: public lava::QtVulkanWindow
{
public:
	VulkanWindow( void );

	lava::QtVulkanWindowRenderer* createRenderer( void ) override;

public slots:
  void changeNumInstancing( int value );

private:
	void keyPressEvent( QKeyEvent* ) override;

	Renderer* _renderer;
};