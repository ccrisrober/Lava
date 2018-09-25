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
using namespace pompeii;

#include <chrono>
#include <ctime>
#include <glm/glm.hpp>

class Timer
{
public:
  void update( void )
  {
      using namespace std::chrono;

      const high_resolution_clock::time_point currTime = high_resolution_clock::now( );
      const double deltaTimeMillis = 0.001 * 
        (double)duration_cast<microseconds>(currTime - prevTime).count();

      const double timePointMillis = 0.001 * 
        (double)duration_cast<microseconds>(currTime - prevFpsTime).count();
      if (timePointMillis > c_updateInterval)
      {
        fps = (float)std::round(fpsFrameCounter / (0.001 * timePointMillis));
        fpsFrameCounter = 0;
        prevFpsTime = currTime;
        fpsUpdated = true;
      }
      ++fpsFrameCounter;

      deltaTimeSeconds = (float)(0.001 * deltaTimeMillis);
      timeSeconds += deltaTimeSeconds;
      prevTime = currTime;

      {
        const time_t tmpTime = system_clock::to_time_t(system_clock::now());
        tm localTm = *localtime(&tmpTime);
        year = (float)localTm.tm_year + 1900.0f;
        month = (float)localTm.tm_mon;
        day = (float)localTm.tm_mday;
        secs = (float)localTm.tm_sec;
      }
  }

  bool isFpsUpdated()
  {
    const bool val = fpsUpdated;
    fpsUpdated = false;
    return val;
  }

  float deltaTimeSeconds  = 0.0f;
  float timeSeconds       = 0.0f;
  float fps               = 0.0f;

  float year  = 0.0f;
  float month = 0.0f;
  float day   = 0.0f;
  float secs  = 0.0f;

private:
  const double c_updateInterval = 16.6 * 10.0;

  std::chrono::high_resolution_clock::time_point prevTime
  { std::chrono::high_resolution_clock::now() };

  std::chrono::high_resolution_clock::time_point prevFpsTime
  { std::chrono::high_resolution_clock::now() };

  uint32_t fpsFrameCounter  = 0;
  bool fpsUpdated           = false;
};

namespace Resources
{
  const std::string imagePath { "textures" };
  const std::vector<std::string> imageFiles
  { "channel0.png", "channel1.png", "channel2.png", "channel3.png" };
  const std::vector<std::string> imageFilesForSearch
  { "channel0", "channel1", "channel2", "channel3" };

  const std::string shaderPath { "shaders" };
  const std::vector<std::string> shaderFiles { "toy.vert", "toy.frag" };
  const std::vector<std::string> spirvFiles { "toy.vert.spv", "toy.frag.spv" };
}

class RendererInput
{
public:
  float date[4]{ 0.0f, 0.0f, 0.0f, 0.0f };
  struct
  {
    uint32_t leftPosX;
    uint32_t leftPosY;
    uint32_t clickLeft;
    uint32_t clickRight;
  } mousePos;

  float globalTime    = 0.0f;
  float deltaTime     = 0.0f;

  uint32_t frameIndex = 0;
};

class MainWindowRenderer : public glfw::VulkanWindowRenderer
{
private:
  glfw::VulkanWindow* _window;
public:
  MainWindowRenderer( glfw::VulkanWindow* window )
    : _window( window )
  {
  }

  void run( void )
  {
    timer.update( );

    if( timer.isFpsUpdated( ) )
    {
      std::cout << "    " << std::to_string(round(timer.timeSeconds))
                << "    " << std::to_string(timer.fps) << " fps" << std::endl;
    }

    ++frameIndex;

    render( );
  }

  void render( void )
  {
    auto cmd = _window->currentCommandBuffer( );


    std::array<vk::ClearValue, 2 > clearValues;
    clearValues[ 0 ] = pompeii::utils::getClearValueColor( 0.0f, 0.0f, 0.0f );
    clearValues[ 1 ] = pompeii::utils::getClearValueDepth( );

    vk::Extent2D extent = _window->swapchainImageSize( );

    cmd->beginRenderPass( _window->renderPass( ),
      _window->framebuffer( ),
      vk::Rect2D( { 0, 0 }, extent ), clearValues,
      vk::SubpassContents::eInline );

    cmd->bindGraphicsPipeline( graphicsPipeline );
    cmd->draw( 3, 1, 0, 0 );

    cmd->endRenderPass( );

    _window->frameReady( ); // Submit and present
  }

  RendererInput getRenderInput( void )
  {
    RendererInput rendererInput;
    rendererInput.globalTime = timer.timeSeconds;
    rendererInput.deltaTime = timer.deltaTimeSeconds;
    rendererInput.frameIndex = frameIndex;
    //rendererInput.mousePos = getMousePos( );
    rendererInput.date[ 0 ] = timer.year;
    rendererInput.date[ 1 ] = timer.month;
    rendererInput.date[ 2 ] = timer.day;
    rendererInput.date[ 3 ] = timer.secs;

    return rendererInput;
  }

  virtual void nextFrame( void )
  {
    run( );

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

    vk::Extent2D extent = _window->swapchainImageSize( );

    cmd->beginRenderPass( _window->renderPass( ),
      _window->framebuffer( ),
      vk::Rect2D( { 0, 0 }, extent ), clearValues,
      vk::SubpassContents::eInline );

    cmd->endRenderPass( );

    _window->frameReady( );
  }
private:
  Timer timer;
  uint32_t frameIndex = 0;

  std::shared_ptr< pompeii::Pipeline > graphicsPipeline;

  glm::uvec2 mousePos;
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