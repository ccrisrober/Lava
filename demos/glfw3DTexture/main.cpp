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

#include <routes.h>

#include "../utils/Camera.h"
#include <glm/gtc/matrix_transform.hpp>

const unsigned int SCR_WIDTH = 500;
const unsigned int SCR_HEIGHT = 500;

class MainWindowRenderer : public glfw::VulkanWindowRenderer
{
private:
  glfw::VulkanWindow* _window;
public:
  MainWindowRenderer( glfw::VulkanWindow* window )
    : _window( window )
  {
    camera = Camera( glm::vec3( 0.0f, 0.0f, 3.5f ) );
  }

  // camera
  Camera camera;
  // timing
  float deltaTime = 0.0f; // time between current frame and last frame
  float lastFrame = 0.0f;

  float lastX = SCR_WIDTH * 0.5f;
  float lastY = SCR_HEIGHT * 0.5f;
  bool firstMouse = true;

  void initResources( void ) override
  {
    auto device = _window->device( );

    geometry = std::make_shared<pompeii::utility::Geometry>( device,
      POMPEII_EXAMPLES_MESHES_ROUTE + std::string( "monkey.obj_" ) );

    mvpBuffer = device->createUniformBuffer( sizeof( uboVS ) );

    int size = 16;
    float a = 1.0f / ( size - 1.0f );
    std::vector<glm::vec4> colors( size * size * size );

    int i = 0;
    glm::vec4 c;
    for ( int z = 0; z < size; ++z )
    {
      for ( int y = 0; y < size; ++y )
      {
        for ( int x = 0; x < size; ++x, ++i )
        {
          c.r = ( ( x & 1 ) != 0 ) ? x * a : 1.0f - x * a;
          c.g = ( ( y & 1 ) != 0 ) ? y * a : 1.0f - y * a;
          c.b = ( ( z & 1 ) != 0 ) ? z * a : 1.0f - z * a;
          colors[ i ] = c;
        }
      }
    }

    tex = _window->device( )->createTexture3D( size, size, size, colors.data( ), 
      sizeof( colors[ i ] ) * colors.size( ), _window->gfxCommandPool( ), 
      _window->gfxQueue( ), vk::Format::eR32G32B32A32Sfloat );

    std::array<vk::DescriptorPoolSize, 2> poolSize =
    {
      vk::DescriptorPoolSize( vk::DescriptorType::eUniformBuffer, 1 ),
      vk::DescriptorPoolSize( vk::DescriptorType::eCombinedImageSampler, 1 )
    };
    auto dspPool = device->createDescriptorPool( 1, poolSize );

    auto vertexStage = device->createShaderPipelineShaderStage(
      POMPEII_EXAMPLES_SPV_ROUTE + std::string( "texture3d_vert.spv" ),
      vk::ShaderStageFlagBits::eVertex
    );
    auto fragmentStage = device->createShaderPipelineShaderStage(
      POMPEII_EXAMPLES_SPV_ROUTE + std::string( "texture3d_frag.spv" ),
      vk::ShaderStageFlagBits::eFragment
    );

    std::vector<DescriptorSetLayoutBinding> dslbs =
    {
      DescriptorSetLayoutBinding( 0, vk::DescriptorType::eUniformBuffer,
        vk::ShaderStageFlagBits::eVertex
      ),
      DescriptorSetLayoutBinding( 1, vk::DescriptorType::eCombinedImageSampler,
        vk::ShaderStageFlagBits::eFragment
      )
    };

    descriptorSetLayout = device->createDescriptorSetLayout( dslbs );

    pipelineLayout = device->createPipelineLayout( descriptorSetLayout );

    vk::VertexInputBindingDescription binding( 0, sizeof( pompeii::utility::Vertex ),
      vk::VertexInputRate::eVertex );

    PipelineVertexInputStateCreateInfo vertexInput( binding, {
      vk::VertexInputAttributeDescription(
        0, 0, vk::Format::eR32G32B32Sfloat,
        offsetof( pompeii::utility::Vertex, position )
      )
    } );
    
    vk::PipelineInputAssemblyStateCreateInfo assembly( { },
      vk::PrimitiveTopology::eTriangleList, VK_FALSE );
    PipelineViewportStateCreateInfo viewport( 1, 1 );
    vk::PipelineRasterizationStateCreateInfo rasterization( { }, true,
      false, vk::PolygonMode::eFill, vk::CullModeFlagBits::eBack,
      vk::FrontFace::eCounterClockwise, false, 0.0f, 0.0f, 0.0f, 1.0f );
    PipelineMultisampleStateCreateInfo multisample( vk::SampleCountFlagBits::e1,
      false, 0.0f, nullptr, false, false );
    vk::StencilOpState stencilOpState( vk::StencilOp::eKeep,
      vk::StencilOp::eKeep, vk::StencilOp::eKeep, vk::CompareOp::eAlways,
      0, 0, 0 );
    vk::PipelineDepthStencilStateCreateInfo depthStencil( { }, true, true,
      vk::CompareOp::eLessOrEqual, false, false, stencilOpState,
      stencilOpState, 0.0f, 0.0f );
    ;
    PipelineColorBlendStateCreateInfo colorBlend( false, vk::LogicOp::eNoOp,
      vk::PipelineColorBlendAttachmentState( false,
        vk::BlendFactor::eZero, vk::BlendFactor::eZero, vk::BlendOp::eAdd,
        vk::BlendFactor::eZero, vk::BlendFactor::eZero, vk::BlendOp::eAdd,
        vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
        vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA
      ), { 1.0f, 1.0f, 1.0f, 1.0f }
    );
    PipelineDynamicStateCreateInfo dynamic( {
      vk::DynamicState::eViewport, vk::DynamicState::eScissor
    } );

    pipeline = device->createGraphicsPipeline( _window->pipelineCache( ), { },
    { vertexStage, fragmentStage }, vertexInput, assembly, nullptr,
      viewport, rasterization, multisample, depthStencil, colorBlend, dynamic,
      pipelineLayout, _window->renderPass( )
    );

    // Init descriptor set
    descriptorSet = device->allocateDescriptorSet( dspPool,
      descriptorSetLayout );

    std::vector<WriteDescriptorSet> wdss =
    {
      WriteDescriptorSet(
        descriptorSet, 0, 0, vk::DescriptorType::eUniformBuffer,
        1, nullptr, DescriptorBufferInfo( mvpBuffer, 0, sizeof( uboVS ) )
      ),
      WriteDescriptorSet(
        descriptorSet, 1, 0, vk::DescriptorType::eCombinedImageSampler, 1,
        tex->descriptor, nullptr
      )
    };
    device->updateDescriptorSets( wdss, { } );
  }

  void updateMVP( void )
  {
    auto size = _window->swapchainImageSize( );

    uint32_t width = size.width, height = size.height;

    static auto startTime = std::chrono::high_resolution_clock::now( );

    auto currentTime = std::chrono::high_resolution_clock::now( );
    float time = std::chrono::duration_cast<std::chrono::milliseconds>( 
      currentTime - startTime ).count( ) / 1000.0f;

    uboVS.model = glm::scale( glm::mat4( 1.0f ), glm::vec3( 6.0f ) + 
      std::sin( time ) * glm::vec3( 2.0f ) );
    uboVS.model = glm::rotate( uboVS.model, time * glm::radians( 8.0f ),
      glm::vec3( 0.0f, 1.0f, 0.0f ) );
    uboVS.view = camera.GetViewMatrix( );
    uboVS.proj = glm::perspective( glm::radians( camera.Zoom ),
      ( float ) width / ( float ) height, 0.1f, 100.0f );
    uboVS.proj[ 1 ][ 1 ] *= -1;

    mvpBuffer->set( &uboVS );
  }

  void nextFrame( void ) override
  {
    updateMVP( );

    std::array<vk::ClearValue, 2 > clearValues;
    std::array<float, 4> ccv = { 1.0f, 1.0f, 1.0f, 1.0f };
    clearValues[ 0 ].color = vk::ClearColorValue( ccv );
    clearValues[ 1 ].depthStencil = vk::ClearDepthStencilValue( 1.0f, 0 );

    auto cmd = _window->currentCommandBuffer( );
    vk::Rect2D rect;
    rect.extent = _window->swapchainImageSize( );
    cmd->beginRenderPass(
      _window->renderPass( ),
      _window->framebuffer( ),
      rect, clearValues, vk::SubpassContents::eInline
    );

    cmd->setViewportScissors( _window->swapchainImageSize( ) );

    {
      cmd->bindGraphicsPipeline( pipeline );
      cmd->bindDescriptorSets( vk::PipelineBindPoint::eGraphics,
        pipelineLayout, 0, { descriptorSet }, { } );

      geometry->render( cmd, 1 );
    }
    cmd->endRenderPass( );

    _window->frameReady( );
  }

private:
  std::shared_ptr< Texture3D > tex;
public:
  std::shared_ptr< DescriptorSetLayout > descriptorSetLayout;
  std::shared_ptr< DescriptorSet > descriptorSet;
  std::shared_ptr< PipelineLayout > pipelineLayout;
  std::shared_ptr< Pipeline > pipeline;
  std::shared_ptr< UniformBuffer > mvpBuffer;

  struct
  {
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
  } uboVS;

  std::shared_ptr< pompeii::utility::Geometry > geometry;

public:
  void mouseEvent( double xpos, double ypos )
  {
    if ( firstMouse )
    {
      lastX = xpos;
      lastY = ypos;
      firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement( xoffset, yoffset );
  }
};

class VulkanWindow : public glfw::VulkanWindow
{
protected:
  MainWindowRenderer* _renderer;
public:
  explicit VulkanWindow( int width, int height,
    const std::string& title, bool enableLayers )
    : glfw::VulkanWindow( width, height, title, enableLayers )
  {
  }
  virtual void mouseEvent( double xPos, double yPos ) override
  {
    _renderer->mouseEvent( xPos, yPos );
  }
  virtual glfw::VulkanWindowRenderer* createRenderer( void ) override
  {
    _renderer = new MainWindowRenderer( this );
    return _renderer;
  }
};


int main( int, char** )
{
  VulkanWindow app( SCR_WIDTH, SCR_HEIGHT, "Texture3D", true );
  app.show( );
  return EXIT_SUCCESS;
}