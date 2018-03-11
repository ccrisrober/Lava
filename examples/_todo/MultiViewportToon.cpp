/**
 * Copyright (c) 2017 - 2018, Lava
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

#include <lava/lava.h>
using namespace lava;

#include <routes.h>

class CustomRenderer : public VulkanWindowRenderer
{
public:
  CustomRenderer( lava::VulkanWindow *w )
    : VulkanWindowRenderer( )
    , _window( w )
  {
    _window->setWindowTitle( "Clear Screen" );
  }

  void nextFrame( void ) override
  {
    if ( Input::isKeyPressed( lava::Keyboard::Key::Esc ) )
    {
      _window->_window->close( );
    }

    std::array<vk::ClearValue, 2 > clearValues;
    std::array<float, 4> ccv = { 0.0f, 0.0f, 0.0, 1.0f };
    clearValues[ 0 ].color = vk::ClearColorValue( ccv );
    clearValues[ 1 ].depthStencil  = vk::ClearDepthStencilValue(  1.0f, 0 );

    const glm::ivec2 size = _window->swapChainImageSize( );
    auto cmd = _window->currentCommandBuffer( );
    vk::Rect2D rect;
    rect.extent.width = size.x;
    rect.extent.height = size.y;
    cmd->beginRenderPass(
      _window->defaultRenderPass( ),
      _window->currentFramebuffer( ),
      rect, clearValues, vk::SubpassContents::eInline
    );

    uint32_t WINDOW_WIDTH = size.x;
    uint32_t WINDOW_HEIGHT = size.y;

    unsigned int min_size = ( WINDOW_HEIGHT > WINDOW_WIDTH ) ? WINDOW_WIDTH : WINDOW_HEIGHT;
    unsigned int x_delta = ( WINDOW_WIDTH - min_size ) / 2;
    unsigned int y_delta = ( WINDOW_HEIGHT - min_size ) / 2;

    std::array<vk::Rect2D, 4> out_scissors;
    std::array<vk::Viewport, 4> out_viewports;

    {
      /* Top-left region */
      out_scissors[ 0 ].extent.height = WINDOW_HEIGHT / 2 - y_delta;
      out_scissors[ 0 ].extent.width = WINDOW_WIDTH / 2 - x_delta;
      out_scissors[ 0 ].offset.x = 0;
      out_scissors[ 0 ].offset.y = 0;

      /* Top-right region */
      out_scissors[ 1 ] = out_scissors[ 0 ];
      out_scissors[ 1 ].offset.x = WINDOW_WIDTH / 2 + x_delta;

      /* Bottom-left region */
      out_scissors[ 2 ] = out_scissors[ 0 ];
      out_scissors[ 2 ].offset.y = WINDOW_HEIGHT / 2 + y_delta;

      /* Bottom-right region */
      out_scissors[ 3 ] = out_scissors[ 2 ];
      out_scissors[ 3 ].offset.x = WINDOW_WIDTH / 2 + x_delta;
    }

    {
      /* Top-left region */
      out_viewports[ 0 ].height = float( WINDOW_HEIGHT / 2 - y_delta );
      out_viewports[ 0 ].width = float( WINDOW_WIDTH / 2 - x_delta );
      out_viewports[ 0 ].x = 0.0f;
      out_viewports[ 0 ].y = 0.0f;
      out_viewports[ 0 ].minDepth = 0.0f;
      out_viewports[ 0 ].maxDepth = 1.0f;

      /* Top-right region */
      out_viewports[ 1 ] = out_viewports[ 0 ];
      out_viewports[ 1 ].x = float( WINDOW_WIDTH / 2 + x_delta );

      /* Bottom-left region */
      out_viewports[ 2 ] = out_viewports[ 0 ];
      out_viewports[ 2 ].y = float( WINDOW_HEIGHT / 2 + y_delta );

      /* Bottom-right region */
      out_viewports[ 3 ] = out_viewports[ 2 ];
      out_viewports[ 3 ].x = float( WINDOW_WIDTH / 2 + x_delta );
    }

    cmd->endRenderPass( );

    _window->requestUpdate( );
  }
private:
  VulkanWindow *_window;
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
    LAVA_KHR_EXT, // OS specific surface extension
    VK_EXT_DEBUG_REPORT_EXTENSION_NAME
  };


  instance = Instance::create( vk::InstanceCreateInfo(
    { },
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