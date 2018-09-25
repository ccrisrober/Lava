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

#include <iostream>

#include <glfwPompeii/glfwPompeii.h>
#include <pompeiiUtils/pompeiiUtils.h>
using namespace pompeii;
using namespace utility;

class MainWindowRenderer : public glfw::VulkanWindowRenderer
{
private:
  glfw::VulkanWindow* _window;
public:
  MainWindowRenderer( glfw::VulkanWindow* window )
    : _window( window )
  {
  }

  pompeii::utility::UIOverlay* ui;

  virtual void initResources( void ) override
  {
    vk::Extent2D extent = _window->swapchainImageSize( );

    pompeii::utility::UIOverlayCreateInfo overlayCreateInfo;
    // Setup default overlay creation info
    overlayCreateInfo.device = _window->device( );
    overlayCreateInfo.copyQueue = _window->gfxQueue( );
    overlayCreateInfo.framebuffers = _window->framebuffers( );
    overlayCreateInfo.colorformat = _window->colorFormat( );
    overlayCreateInfo.depthformat = _window->depthStencilFormat( );
    overlayCreateInfo.width = extent.width;
    overlayCreateInfo.height = extent.height;
    // Virtual function call for example to customize overlay creation
    /*OnSetupUIOverlay( overlayCreateInfo );
    // Load default shaders if not specified by example
    if ( overlayCreateInfo.shaders.size( ) == 0 ) {
      overlayCreateInfo.shaders = {
        loadShader( getAssetPath( ) + "shaders/base/uioverlay.vert.spv", VK_SHADER_STAGE_VERTEX_BIT ),
        loadShader( getAssetPath( ) + "shaders/base/uioverlay.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT ),
      };
    }*/
    ui = new pompeii::utility::UIOverlay( overlayCreateInfo );
    //updateOverlay( );
  }

  virtual void nextFrame( void )
  {
    vk::Extent2D extent = _window->swapchainImageSize( );

    ui->update( );

    auto cmd = _window->currentCommandBuffer( );

    static auto startTime = std::chrono::high_resolution_clock::now( );

    auto currentTime = std::chrono::high_resolution_clock::now( );
    float time = std::chrono::duration_cast< std::chrono::milliseconds >(
      currentTime - startTime ).count( ) / 1000.0f;

    float _red = sin( time ) * 0.5f + 0.5f;
    float _blue = cos( time ) * 0.5f + 0.5f;

    std::array<vk::ClearValue, 2 > clearValues;
    clearValues[ 0 ] = pompeii::utils::getClearValueColor( _red, 0.0f, _blue );
    clearValues[ 1 ] = pompeii::utils::getClearValueDepth( );

    cmd->beginRenderPass( _window->renderPass( ),
      _window->framebuffer( ),
      vk::Rect2D( { 0, 0 }, extent ), clearValues,
      vk::SubpassContents::eInline );

    cmd->endRenderPass( );

	ui->update();

    _window->frameReady( );
  }
};

class VulkanWindow : public glfw::VulkanWindow
{
public:
  explicit VulkanWindow( int width, int height,
    const std::string& title, bool enableLayers )
    : glfw::VulkanWindow( width, height, title, enableLayers )
  {
  }
  virtual glfw::VulkanWindowRenderer* createRenderer( void ) override
  {
    return new MainWindowRenderer( this );
  }
};


int main( int, char** )
{
  VulkanWindow app( 500, 500, "GLFWRenderer", true );
  app.show( );
  return EXIT_SUCCESS;
}