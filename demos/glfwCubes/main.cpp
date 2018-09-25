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

#define OBJECT_INSTANCES 1

class MainWindowRenderer : public glfw::VulkanWindowRenderer
{
private:
  glfw::VulkanWindow* _window;
public:
  MainWindowRenderer( glfw::VulkanWindow* window )
    : _window( window )
  {
  }

  struct
  {
    glm::mat4 projection;
    glm::mat4 view;
  } uboVS;

  std::shared_ptr<Buffer/*UniformDynamicBuffer*/> dynamicUbo;
  std::shared_ptr<UniformBuffer> uboVP;

  std::shared_ptr< Pipeline > pipeline;
  std::shared_ptr< PipelineLayout > pipelineLayout;
  std::shared_ptr< DescriptorSet > descriptorSet;
  std::shared_ptr< DescriptorSetLayout > descriptorSetLayout;

  std::shared_ptr< Buffer > vertexBuffer;
  std::shared_ptr< Buffer > indexBuffer;

  std::shared_ptr< DescriptorPool > descriptorPool;

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

  uint32_t dynamicAlignment = sizeof( glm::mat4 );

  // Wrapper functions for aligned memory allocation
  // There is currently no standard for this in C++ that works across all platforms and vendors, so we abstract this
  void* alignedAlloc( size_t size, size_t alignment )
  {
    void *data = nullptr;
#if defined(_MSC_VER) || defined(__MINGW32__)
    data = _aligned_malloc( size, alignment );
#else 
    int res = posix_memalign( &data, alignment, size );
    if ( res != 0 )
      data = nullptr;
#endif
    return data;
  }

  void alignedFree( void* data )
  {
#if	defined(_MSC_VER) || defined(__MINGW32__)
    _aligned_free( data );
#else 
    free( data );
#endif
  }

  virtual void releaseResources( void ) override
  {
    if ( uboDataDynamic.model )
    {
      alignedFree( uboDataDynamic.model );
    }
  }

  virtual void initResources( void ) override
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

    auto limits = device->getPhysicalDevice( )->getDeviceProperties( ).limits;

    size_t minUboAlignment = limits.minUniformBufferOffsetAlignment;
    dynamicAlignment = sizeof( glm::mat4 );
    if ( minUboAlignment > 0 ) {
      dynamicAlignment = ( dynamicAlignment + minUboAlignment - 1 ) & 
        ~( minUboAlignment - 1 );
    }

    std::cout << "minUniformBufferOffsetAlignment = " << minUboAlignment << std::endl;
    std::cout << "dynamicAlignment = " << dynamicAlignment << std::endl;

    size_t bufferSize = OBJECT_INSTANCES * dynamicAlignment;

    dynamicUbo = device->createBuffer( bufferSize,
      vk::BufferUsageFlagBits::eUniformBuffer,
      vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent );
    //device->createUniformDynamicBuffer( dynamicAlignment, OBJECT_INSTANCES );

    uboDataDynamic.model = ( glm::mat4* )alignedAlloc( bufferSize, dynamicAlignment );

    uboVP = device->createUniformBuffer( sizeof( uboVS ) );

    auto vertexStage = device->createShaderPipelineShaderStage(
      POMPEII_EXAMPLES_SPV_ROUTE + std::string( "cubeUVUniformDynamic_vert.spv" ),
      vk::ShaderStageFlagBits::eVertex
    );
    auto fragmentStage = device->createShaderPipelineShaderStage(
      POMPEII_EXAMPLES_SPV_ROUTE + std::string( "cubeUVUniformDynamic_frag.spv" ),
      vk::ShaderStageFlagBits::eFragment
    );

    std::vector<DescriptorSetLayoutBinding> dslbs =
    {
      DescriptorSetLayoutBinding( 0, vk::DescriptorType::eUniformBufferDynamic,
        vk::ShaderStageFlagBits::eVertex
      ),
      DescriptorSetLayoutBinding( 1, vk::DescriptorType::eUniformBuffer,
        vk::ShaderStageFlagBits::eVertex
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

    std::array<vk::DescriptorPoolSize, 2> poolSize =
    {
      // Compute Uniform Dynamic Buffer (models)
      vk::DescriptorPoolSize( vk::DescriptorType::eUniformBufferDynamic, 1 ),
      // Compute Uniform Buffer (projection and view)
      vk::DescriptorPoolSize( vk::DescriptorType::eUniformBuffer, 1 ),
    };
    descriptorPool = device->createDescriptorPool( 1, poolSize );

    // Init descriptor set
    descriptorSet = device->allocateDescriptorSet( descriptorPool, 
      descriptorSetLayout );

    std::vector<WriteDescriptorSet> wdss =
    {
      WriteDescriptorSet(
        descriptorSet, 0, 0, vk::DescriptorType::eUniformBufferDynamic,
        1, nullptr, DescriptorBufferInfo( dynamicUbo, 0, sizeof( glm::mat4 ) )
      ),
      WriteDescriptorSet(
        descriptorSet, 1, 0, vk::DescriptorType::eUniformBuffer,
        1, nullptr, DescriptorBufferInfo( uboVP, 0, sizeof( uboVS ) )
      )
    };
    device->updateDescriptorSets( wdss, { } );
    
    // Aligned offset
    {
      glm::mat4* modelMat = ( glm::mat4* )( ( ( uint64_t ) uboDataDynamic.model + ( 0 * dynamicAlignment ) ) );
      *modelMat = glm::scale(glm::mat4( 1.0f ), glm::vec3( 2.5f ) );
    }
    if( OBJECT_INSTANCES > 1 )
    {
      glm::mat4* modelMat = ( glm::mat4* )( ( ( uint64_t ) uboDataDynamic.model + ( 1 * dynamicAlignment ) ) );
      *modelMat = glm::translate( 
        glm::scale( glm::mat4( 1.0f ),
          glm::vec3( 2.5f )
        ), glm::vec3( -1.5f, 1.5f, -1.5f )
      );
    }

    dynamicUbo->set( &uboDataDynamic );
  }
  
  struct UboDataDynamic
  {
    glm::mat4 *model = nullptr;
  } uboDataDynamic;

  virtual void nextFrame( void )
  {
    auto cmd = _window->currentCommandBuffer( );

    //static auto startTime = std::chrono::high_resolution_clock::now( );

    //auto currentTime = std::chrono::high_resolution_clock::now( );
    //float time = std::chrono::duration_cast< std::chrono::milliseconds >(
    //  currentTime - startTime ).count( ) / 1000.0f;

    vk::Extent2D extent = _window->swapchainImageSize( );

    uboVS.view = glm::lookAt( 
      glm::vec3( 0.0f, 3.0f, -10.0f ),  // Camera is at (0,3,-10), in World Space
      glm::vec3( 0.0f, 0.0f, 0.0f ),    // and looks at the origin
      glm::vec3( 0.0f, -1.0f, 0.0f )    // Head is up (set to 0,-1,0 to look upside-down)
    );
    uboVS.projection = glm::perspective( 
      glm::radians( 45.0f ), 
      extent.width / ( float ) extent.height, 
      0.1f, 100.0f
    );
    uboVS.projection[ 1 ][ 1 ] *= -1;

    uboVP->writeData( 0, sizeof( uboVS ), &uboVS );

    std::array<vk::ClearValue, 2 > clearValues;
    clearValues[ 0 ] = pompeii::utils::getClearValueColor( 0.0f, 0.0f, 0.0f );
    clearValues[ 1 ] = pompeii::utils::getClearValueDepth( );

    cmd->beginRenderPass( _window->renderPass( ),
      _window->framebuffer( ),
      vk::Rect2D( { 0, 0 }, extent ), clearValues,
      vk::SubpassContents::eInline );

    cmd->setViewportScissors( extent );

    cmd->bindGraphicsPipeline( pipeline );

    cmd->bindVertexBuffer( 0, vertexBuffer, 0 );
    cmd->bindIndexBuffer( indexBuffer, 0, vk::IndexType::eUint16 );

    // Render multiple objects using different model matrices by dynamically offsetting into one uniform buffer
    for ( uint32_t j = 0; j < OBJECT_INSTANCES; ++j )
    {
      // One dynamic offset per dynamic descriptor to offset into the ubo containing all model matrices
      uint32_t dynamicOffset = j * static_cast<uint32_t>( dynamicAlignment );
      // Bind the descriptor set for rendering a mesh using the dynamic offset
      cmd->bindDescriptorSets( vk::PipelineBindPoint::eGraphics, 
        pipelineLayout, 0, descriptorSet, dynamicOffset );

      cmd->drawIndexed( indices.size( ), 1, 0, 0, 1 );
    }

    cmd->endRenderPass( );

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