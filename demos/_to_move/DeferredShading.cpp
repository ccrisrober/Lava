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
#include <pompeiiUtils/pompeiiUtils.h>
#include <pompeiiRenderer/pompeiiRenderer.h>
#include <glm/glm.hpp>

#include <routes.h>

struct
{
  struct
  {
    std::shared_ptr<pompeii::Texture2D> colorMap;
    std::shared_ptr<pompeii::Texture2D> normalMap;
  } model;
  struct
  {
    std::shared_ptr<pompeii::Texture2D> colorMap;
    std::shared_ptr<pompeii::Texture2D> normalMap;
  } floor;
} textures;

struct
{
  std::shared_ptr<pompeii::utility::Geometry> model;
  std::shared_ptr<pompeii::utility::Geometry> floor;
} models;

struct
{
  glm::mat4 projection;
  glm::mat4 view;
  glm::mat4 model;
  glm::vec4 instancePos[ 3 ];
} uboVS, uboOffScreenVS;

struct Light
{
  glm::vec4 position;
  glm::vec3 color;
  float radius;
};

struct
{
  Light lights[ 4 ];
  glm::vec4 viewPos;
} uboFragmentLights;

struct
{
  std::shared_ptr<pompeii::Buffer> vsFullScreen;
  std::shared_ptr<pompeii::Buffer> vsOffScreen;
  std::shared_ptr<pompeii::Buffer> fsLights;
} ubos;

struct
{
  std::shared_ptr<pompeii::PipelineLayout> deferred;
  std::shared_ptr<pompeii::PipelineLayout> offscreen;
} pipelines;

struct
{
  std::shared_ptr<pompeii::DescriptorSet> deferred;
  std::shared_ptr<pompeii::DescriptorSet> offscreen;
} descriptorSets;

std::shared_ptr<pompeii::DescriptorSet> descriptorSet;
std::shared_ptr<pompeii::DescriptorSetLayout> descriptorSetLayout;

std::shared_ptr<pompeii::utility::CustomFramebuffer> customFbo;
#include <routes.h>

class CustomRenderer : public glfw::VulkanWindowRenderer
{
public:
  CustomRenderer( glfw::VulkanWindow *w )
    : glfw::VulkanWindowRenderer( )
    , _window( w )
  {
    _window->setWindowTitle( "Clear Screen" );
  }


  void initResources( void ) override
  {
    auto device = _window->device( );

    models.model = std::make_shared<pompeii::utility::Geometry>( device,
      POMPEII_EXAMPLES_MESHES_ROUTE + std::string( "armor.dae" ) );

    models.floor = std::make_shared<pompeii::utility::Geometry>( device,
      POMPEII_EXAMPLES_MESHES_ROUTE + std::string( "floor.obj_" ) );

    textures.model.colorMap = device->createTexture2D(
      POMPEII_EXAMPLES_IMAGES_ROUTE + std::string( "armor_color.png" ),
      _window->gfxCommandPool( ), _window->gfxQueue( ),
      vk::Format::eR8G8B8A8Unorm );

    textures.model.normalMap = device->createTexture2D(
      POMPEII_EXAMPLES_IMAGES_ROUTE + std::string( "armor_normal.png" ),
      _window->gfxCommandPool( ), _window->gfxQueue( ),
      vk::Format::eR8G8B8A8Unorm );

    textures.floor.colorMap = device->createTexture2D(
      POMPEII_EXAMPLES_IMAGES_ROUTE + std::string( "stonefloor_color.png" ),
      _window->gfxCommandPool( ), _window->gfxQueue( ),
      vk::Format::eR8G8B8A8Unorm );

    textures.floor.normalMap = device->createTexture2D(
      POMPEII_EXAMPLES_IMAGES_ROUTE + std::string( "stonefloor_normal.png" ),
      _window->gfxCommandPool( ), _window->gfxQueue( ),
      vk::Format::eR8G8B8A8Unorm );
  }

  void nextFrame( void ) override
  {
    if ( pompeii::Input::isKeyPressed( pompeii::Keyboard::Key::Esc ) )
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

    const vk::Offset2D size = _window->swapChainImageSize( );
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
  glfw::VulkanWindow *_window;
  float _red = 0.0f;
  float _blue = 0.0f;
};


class CustomVkWindow : public glfw::VulkanWindow
{
public:
  glfw::VulkanWindowRenderer* createRenderer( void ) override
  {
    return new CustomRenderer( this );
  }
};

int main( void )
{
  std::shared_ptr<pompeii::Instance> instance;

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
    /*#ifndef NDEBUG
    "VK_LAYER_LUNARG_standard_validation",
    #endif*/
  };
  std::vector<const char*> extensions =
  {
    VK_KHR_SURFACE_EXTENSION_NAME,  // Surface extension
    POMPEII_KHR_EXT, // OS specific surface extension
    VK_EXT_DEBUG_REPORT_EXTENSION_NAME
  };


  instance = pompeii::Instance::create( vk::InstanceCreateInfo(
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
