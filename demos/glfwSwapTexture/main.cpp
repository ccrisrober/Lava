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

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_SWIZZLE

#include <glm/glm.hpp>

#include <glm/gtc/matrix_transform.hpp>

#include <routes.h>

template <class T>
class CustomPingPong
{
public:
  CustomPingPong( const T & elem1, const T & elem2 );
  void swap( void );
  void swap( std::function<void( )> cb );
  T first( void ) const;
  T last( void ) const;
protected:
  T _elem1;
  T _elem2;
};

template<typename T>
CustomPingPong<T>::CustomPingPong( const T & elem1, const T & elem2 )
  : _elem1( std::move( elem1 ) )
  , _elem2( std::move( elem2 ) )
{
}
template<typename T>
void CustomPingPong<T>::swap( void )
{
  std::swap( _elem1, _elem2 );
}
template<typename T>
void CustomPingPong<T>::swap( std::function<void( )> callback )
{
  std::swap( _elem1, _elem2 );
  if ( callback )
  {
    callback( );
  }
}
template<typename T>
T CustomPingPong<T>::first( void ) const
{
  return _elem1;
}
template<typename T>
T CustomPingPong<T>::last( void ) const
{
  return _elem2;
}

class MainWindowRenderer : public glfw::VulkanWindowRenderer
{
private:
  glfw::VulkanWindow* _window;
public:
  MainWindowRenderer( glfw::VulkanWindow* window )
    : _window( window )
  {
  }

  struct Vertex
  {
    glm::vec3 pos;
    glm::vec2 texCoord;
  };

  const std::vector<Vertex> vertices =
  {
    { { -0.5f, -0.5f,  0.5f },{ 0.0f, 0.0f } },
    { { 0.5f, -0.5f,  0.5f },{ 1.0f, 0.0f } },
    { { -0.5f,  0.5f,  0.5f },{ 0.0f, 1.0f } },
    { { 0.5f,  0.5f,  0.5f },{ 1.0f, 1.0f } },

    { { -0.5f, -0.5f, -0.5f },{ 0.0f, 0.0f } },
    { { 0.5f, -0.5f, -0.5f },{ 1.0f, 0.0f } },
    { { -0.5f,  0.5f, -0.5f },{ 0.0f, 1.0f } },
    { { 0.5f,  0.5f, -0.5f },{ 1.0f, 1.0f } },

    { { 0.5f, -0.5f, -0.5f },{ 0.0f, 0.0f } },
    { { 0.5f, -0.5f,  0.5f },{ 1.0f, 0.0f } },
    { { 0.5f,  0.5f, -0.5f },{ 0.0f, 1.0f } },
    { { 0.5f,  0.5f,  0.5f },{ 1.0f, 1.0f } },

    { { -0.5f, -0.5f, -0.5f },{ 0.0f, 0.0f } },
    { { -0.5f, -0.5f,  0.5f },{ 1.0f, 0.0f } },
    { { -0.5f,  0.5f, -0.5f },{ 0.0f, 1.0f } },
    { { -0.5f,  0.5f,  0.5f },{ 1.0f, 1.0f } },

    { { -0.5f,  0.5f, -0.5f },{ 0.0f, 0.0f } },
    { { -0.5f,  0.5f,  0.5f },{ 1.0f, 0.0f } },
    { { 0.5f,  0.5f, -0.5f },{ 0.0f, 1.0f } },
    { { 0.5f,  0.5f,  0.5f },{ 1.0f, 1.0f } },

    { { -0.5f, -0.5f, -0.5f },{ 0.0f, 0.0f } },
    { { -0.5f, -0.5f,  0.5f },{ 1.0f, 0.0f } },
    { { 0.5f, -0.5f, -0.5f },{ 0.0f, 1.0f } },
    { { 0.5f, -0.5f,  0.5f },{ 1.0f, 1.0f } }
  };
  const std::vector<uint16_t> indices =
  {
    0,  1,  2,     1,  3,  2,
    4,  6,  5,     5,  6,  7,
    8, 10,  9,     9, 10, 11,
    12, 13, 14,    13, 15, 14,
    16, 17, 18,    17, 19, 18,
    20, 22, 21,    21, 22, 23,
  };

  void initResources( void ) override
  {
    auto device = _window->device( );

    auto cmd = _window->gfxCommandPool( )->allocateCommandBuffer( );
    cmd->begin( );
    // Vertex buffer
    {
      uint32_t vertexBufferSize = vertices.size( ) * sizeof( Vertex );

      vertexBuffer = device->createBuffer( vertexBufferSize,
        vk::BufferUsageFlagBits::eVertexBuffer |
        vk::BufferUsageFlagBits::eTransferDst,
        vk::MemoryPropertyFlagBits::eDeviceLocal );
      vertexBuffer->update<Vertex>( cmd, 0, { uint32_t( vertices.size( ) ),
        vertices.data( ) } );
    }

    // Index buffer
    {
      uint32_t indexBufferSize = indices.size( ) * sizeof( uint32_t );

      indexBuffer = device->createBuffer( indexBufferSize,
        vk::BufferUsageFlagBits::eIndexBuffer |
        vk::BufferUsageFlagBits::eTransferDst,
        vk::MemoryPropertyFlagBits::eDeviceLocal );
      indexBuffer->update<uint16_t>( cmd, 0, { uint32_t( indices.size( ) ),
        indices.data( ) } );
    }
    cmd->end( );
    _window->gfxQueue( )->submitAndWait( cmd );
  
    // Uniform buffers
    {
      mvpBuffer = device->createUniformBuffer( sizeof( uboVS ) );
    }

    auto tex1 = device->createTexture2D( POMPEII_EXAMPLES_IMAGES_ROUTE +
      std::string( "chesterfieldDiffuseMap.png" ), _window->gfxCommandPool( ),
      _window->gfxQueue( ), vk::Format::eR8G8B8A8Unorm );
    auto tex2 = device->createTexture2D( POMPEII_EXAMPLES_IMAGES_ROUTE +
      std::string( "chesterfieldNormalMap.png" ), _window->gfxCommandPool( ),
      _window->gfxQueue( ), vk::Format::eR8G8B8A8Unorm );

    cpp = new CustomPingPong<std::shared_ptr<Texture2D>>( tex1, tex2 );

    auto vertexStage = device->createShaderPipelineShaderStage(
      POMPEII_EXAMPLES_SPV_ROUTE + std::string( "cubeUV_vert.spv" ),
      vk::ShaderStageFlagBits::eVertex
    );
    auto fragmentStage = device->createShaderPipelineShaderStage(
      POMPEII_EXAMPLES_SPV_ROUTE + std::string( "cubeUV_frag.spv" ),
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

    pipelineLayout = device->createPipelineLayout( descriptorSetLayout, nullptr );

    vk::VertexInputBindingDescription binding( 0, sizeof( Vertex ), vk::VertexInputRate::eVertex );

    PipelineVertexInputStateCreateInfo vertexInput( binding, {
      vk::VertexInputAttributeDescription( 
        0, 0, vk::Format::eR32G32B32Sfloat, offsetof( Vertex, pos )
      ),
      vk::VertexInputAttributeDescription( 
        1, 0, vk::Format::eR32G32Sfloat, offsetof( Vertex, texCoord )
      )
    } );

    vk::PipelineInputAssemblyStateCreateInfo assembly( { },
      vk::PrimitiveTopology::eTriangleList, VK_FALSE );
    PipelineViewportStateCreateInfo viewport( 1, 1 );
    vk::PipelineRasterizationStateCreateInfo rasterization( {}, true,
      false, vk::PolygonMode::eFill, vk::CullModeFlagBits::eBack,
      vk::FrontFace::eCounterClockwise, false, 0.0f, 0.0f, 0.0f, 1.0f );
    PipelineMultisampleStateCreateInfo multisample( vk::SampleCountFlagBits::e1, 
      false, 0.0f, nullptr, false, false );
    vk::StencilOpState stencilOpState( vk::StencilOp::eKeep, 
      vk::StencilOp::eKeep, vk::StencilOp::eKeep, vk::CompareOp::eAlways, 
      0, 0, 0 );
    vk::PipelineDepthStencilStateCreateInfo depthStencil( {}, true, true, 
      vk::CompareOp::eLessOrEqual, false, false, stencilOpState, 
      stencilOpState, 0.0f, 0.0f );
    ;
    PipelineColorBlendStateCreateInfo colorBlend( false, vk::LogicOp::eNoOp, 
      vk::PipelineColorBlendAttachmentState( 
        false, vk::BlendFactor::eZero, vk::BlendFactor::eZero, 
        vk::BlendOp::eAdd, vk::BlendFactor::eZero, vk::BlendFactor::eZero, 
        vk::BlendOp::eAdd, vk::ColorComponentFlagBits::eR | 
        vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | 
        vk::ColorComponentFlagBits::eA
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

    std::array<vk::DescriptorPoolSize, 3> poolSize =
    {
      // Compute UBO and MVP
      vk::DescriptorPoolSize( vk::DescriptorType::eUniformBuffer, 2 ),
      // Graphics image samplers
      vk::DescriptorPoolSize( vk::DescriptorType::eCombinedImageSampler, 1 ),
      // Storage image for ray traced image output
      vk::DescriptorPoolSize( vk::DescriptorType::eStorageImage, 1 )
    };
    descriptorPool = device->createDescriptorPool( 2, poolSize );

    // Init descriptor set
    descriptorSet = device->allocateDescriptorSet( descriptorPool, descriptorSetLayout );

    std::vector<WriteDescriptorSet> wdss =
    {
      WriteDescriptorSet( 
        descriptorSet, 0, 0, vk::DescriptorType::eUniformBuffer,
        1, nullptr, DescriptorBufferInfo( mvpBuffer, 0, sizeof( uboVS ) )
      )
    };
    device->updateDescriptorSets( wdss, { } );
  }

  float lastTime = 0.0f;

  void updateBuffers( void )
  {
    auto size = _window->swapchainImageSize( );

    uint32_t width = size.width, height = size.height;

    static auto startTime = std::chrono::high_resolution_clock::now( );

    auto currentTime = std::chrono::high_resolution_clock::now( );
    float time = std::chrono::duration_cast<std::chrono::milliseconds>( currentTime - startTime ).count( ) / 1000.0f;

    uboVS.model = glm::rotate( glm::mat4( 1.0f ), time * glm::radians( 90.0f ), glm::vec3( 0.0f, 0.0f, 1.0f ) );
    uboVS.model = glm::rotate( uboVS.model, time * glm::radians( 45.0f ), glm::vec3( 0.0f, 1.0f, 0.0f ) );

    uboVS.view = glm::lookAt( glm::vec3( 2.0f, 2.0f, 2.0f ), glm::vec3( 0.0f, 0.0f, 0.0f ), glm::vec3( 0.0f, 0.0f, 1.0f ) );
    uboVS.proj = glm::perspective( glm::radians( 45.0f ), width / ( float ) height, 0.1f, 10.0f );
    uboVS.proj[ 1 ][ 1 ] *= -1;

    mvpBuffer->writeData( 0, sizeof( uboVS ), &uboVS );
  }

  virtual void nextFrame( void )
  {
    static int i = 50;
    if ( ++i >= 50 )
    {
      i = 0;
      cpp->swap( );
      swapTexture( cpp->first( ) );
    }

    updateBuffers( );

    std::array<vk::ClearValue, 2 > clearValues;
    std::array<float, 4> ccv = { 0.2f, 0.3f, 0.3f, 1.0f };
    clearValues[ 0 ].color = vk::ClearColorValue( ccv );
    clearValues[ 1 ].depthStencil = vk::ClearDepthStencilValue( 1.0f, 0 );

    const auto size = _window->swapchainImageSize( );
    auto cmd = _window->currentCommandBuffer( );

    vk::Rect2D rect;
    rect.extent = size;
    cmd->beginRenderPass(
      _window->renderPass( ),
      _window->framebuffer( ),
      rect, clearValues, vk::SubpassContents::eInline
    );

    cmd->bindGraphicsPipeline( pipeline );
    cmd->bindDescriptorSets( vk::PipelineBindPoint::eGraphics, 
      pipelineLayout, 0, { descriptorSet }, {} );

    cmd->bindVertexBuffer( 0, vertexBuffer );
    cmd->bindIndexBuffer( indexBuffer, 0, vk::IndexType::eUint16 );

    cmd->setViewportScissors( size );
    cmd->drawIndexed( indices.size( ), 1, 0, 0, 1 );

    cmd->endRenderPass( );

    _window->frameReady( );
  }
private:
  void swapTexture( std::shared_ptr<Texture2D> tex )
  {
    // We can't updateDescriptorSet while the device is using it
    _window->device( )->waitIdle( );
    std::vector<WriteDescriptorSet> wdss =
    {
      WriteDescriptorSet(
        descriptorSet, 1, 0, vk::DescriptorType::eCombinedImageSampler, 1,
        tex->descriptor, nullptr
      )
    };
    _window->device( )->updateDescriptorSets( wdss, { } );
  }

  CustomPingPong< std::shared_ptr< Texture2D > > *cpp;

  std::shared_ptr< UniformBuffer > mvpBuffer;
  std::shared_ptr< Pipeline > pipeline;
  std::shared_ptr< PipelineLayout > pipelineLayout;
  std::shared_ptr< DescriptorSet > descriptorSet;
  std::shared_ptr< DescriptorSetLayout > descriptorSetLayout;

  std::shared_ptr< Buffer > vertexBuffer;
  std::shared_ptr< Buffer > indexBuffer;
  struct
  {
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
  } uboVS;

  std::shared_ptr< DescriptorPool > descriptorPool;

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
  VulkanWindow app( 500, 500, "Cube swap texture", true );
  app.show( );
  return EXIT_SUCCESS;
}