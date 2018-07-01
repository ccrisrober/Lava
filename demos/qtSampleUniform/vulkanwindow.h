#pragma once

#include <qtLava/qtLava.h>

class Renderer;

class VulkanWindow: public lava::QtVulkanWindow
{
public:
	VulkanWindow( void );

	lava::QtVulkanWindowRenderer* createRenderer( void ) override;

public slots:
  void changeSampler( int idx );

private:
	Renderer* _renderer;
};