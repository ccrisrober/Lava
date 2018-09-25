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

#include <pompeii/pompeii.h>
using namespace pompeii;

#include <routes.h>

#include "utils/TeapotQuad.h"

class CustomRenderer : public VulkanWindowRenderer
{
public:
  CustomRenderer( glfw::VulkanWindow *w )
    : VulkanWindowRenderer( )
    , _window( w )
  {
    _window->setWindowTitle( "Clear Screen" );
  }

  TeapotQuad * tq;

  void initResources( void ) override
  {
    auto device = _window->device( );
    tq = new TeapotQuad( device, _window->gfxCommandPool( ), 
      _window->gfxQueue( ) );
  }
  void nextFrame( void ) override
  {
    if ( Input::isKeyPressed( pompeii::Keyboard::Key::Esc ) )
    {
      _window->_window->close( );
    }
    static auto startTime = std::chrono::high_resolution_clock::now( );

    auto currentTime = std::chrono::high_resolution_clock::now( );
    float time = std::chrono::duration_cast<std::chrono::milliseconds>(
      currentTime - startTime ).count( ) / 1000.0f;

    _red = sin( time ) * 0.5f + 0.5f;
    _blue = cos( time ) * 0.5f + 0.5f;

    std::array<vk::ClearValue, 2 > clearValues;
    std::array<float, 4> ccv = { _red, 0.0f, _blue, 1.0f };
    clearValues[ 0 ].color = vk::ClearColorValue( ccv );
    clearValues[ 1 ].depthStencil = vk::ClearDepthStencilValue( 1.0f, 0 );

    const auto size = _window->swapChainImageSize( );
    auto cmd = _window->currentCommandBuffer( );
    vk::Rect2D rect;
    rect.extent.width = size.x;
    rect.extent.height = size.y;
    cmd->beginRenderPass(
      _window->defaultRenderPass( ),
      _window->currentFramebuffer( ),
      rect, clearValues, vk::SubpassContents::eInline
    );

    cmd->endRenderPass( );

    _window->requestUpdate( );
  }
private:
  VulkanWindow *_window;
  float _red = 0.0f;
  float _blue = 0.0f;
};

class CustomVkWindow : public VulkanWindow
{
public:
  VulkanWindowRenderer* createRenderer( void ) override
  {
    return new CustomRenderer( this );
  }
};

int main( void )
{
  std::shared_ptr<Instance> instance;

  // Create instance
  vk::ApplicationInfo appInfo(
    "App Name",
    VK_MAKE_VERSION( 1, 0, 0 ),
    "FooEngine",
    VK_MAKE_VERSION( 1, 0, 0 ),
    VK_API_VERSION_1_0
  );


  std::vector<const char*> layers =
  {
#ifndef NDEBUG
    "VK_LAYER_LUNARG_standard_validation",
#endif
  };
  std::vector<const char*> extensions =
  {
    VK_KHR_SURFACE_EXTENSION_NAME,  // Surface extension
    POMPEII_KHR_EXT, // OS specific surface extension
    VK_EXT_DEBUG_REPORT_EXTENSION_NAME
  };


  instance = Instance::create( vk::InstanceCreateInfo(
  {},
    &appInfo,
    layers.size( ),
    layers.data( ),
    extensions.size( ),
    extensions.data( )
  ) );

  CustomVkWindow w;
  w.setVulkanInstance( instance );
  w.resize( 500, 500 );

  w.show( );

  return 0;
}