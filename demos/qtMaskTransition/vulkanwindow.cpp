/**
 * Copyright (c) 2017 - 2018, Pompeii
 * All rights reserved.
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 **/

#include "vulkanwindow.h"

#include "renderer.h"

#include <QKeyEvent>

VulkanWindow::VulkanWindow( void )
{
}

pompeii::qt::VulkanWindowRenderer* VulkanWindow::createRenderer( void )
{
	_renderer = new Renderer( this );
	return _renderer;
}

void VulkanWindow::changeMaskLevel( int value )
{
  _renderer->changeMaskLevel( value * 0.01f ); // from [0, 100] to [0.0, 1.0]
  requestUpdate( );
}

void VulkanWindow::changeImage( int idx )
{
  _renderer->changeImage( idx );
  requestUpdate( );
}