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

#include <routes.h>
#include <random>

#include "../utils/Camera.h"

class MainWindowRenderer : public glfw::VulkanWindowRenderer
{
private:
  glfw::VulkanWindow* _window;
  struct Vertex
  {
    glm::vec3 position;
  };

  // camera
  Camera camera;
  // timing
  float deltaTime = 0.0f; // time between current frame and last frame
  float lastFrame = 0.0f;

  const std::vector<Vertex> vertices =
  {
    { { -0.5f, -0.5f,  0.5f } },
    { { 0.5f, -0.5f,  0.5f } },
    { { -0.5f,  0.5f,  0.5f } },
    { { 0.5f,  0.5f,  0.5f } },

    { { -0.5f, -0.5f, -0.5f } },
    { { 0.5f, -0.5f, -0.5f } },
    { { -0.5f,  0.5f, -0.5f } },
    { { 0.5f,  0.5f, -0.5f } },

    { { 0.5f, -0.5f, -0.5f } },
    { { 0.5f, -0.5f,  0.5f } },
    { { 0.5f,  0.5f, -0.5f } },
    { { 0.5f,  0.5f,  0.5f } },

    { { -0.5f, -0.5f, -0.5f } },
    { { -0.5f, -0.5f,  0.5f } },
    { { -0.5f,  0.5f, -0.5f } },
    { { -0.5f,  0.5f,  0.5f } },

    { { -0.5f,  0.5f, -0.5f } },
    { { -0.5f,  0.5f,  0.5f } },
    { { 0.5f,  0.5f, -0.5f } },
    { { 0.5f,  0.5f,  0.5f } },

    { { -0.5f, -0.5f, -0.5f } },
    { { -0.5f, -0.5f,  0.5f } },
    { { 0.5f, -0.5f, -0.5f } },
    { { 0.5f, -0.5f,  0.5f } }
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
public:
  MainWindowRenderer( glfw::VulkanWindow* window )
    : _window( window )
  {
    camera = Camera( glm::vec3( 0.0f, 4.0f, 35.0f ),
      glm::vec3( 0.0f, 1.0f, 0.0f ), YAW, -15.0f );
  }

  void createGrassPoints( std::vector<Vertex>& vertices )
  {
    for ( float i = -50.0f; i < 50.0f; i += 2.5f )
    {
      for ( float j = -50.0f; j < 50.0f; j += 2.5f )
      {
        glm::vec3 point;
        point.x = i;
        point.y = -5.0f;
        point.z = j;
        vertices.push_back( Vertex{ point } );
      }
    }
  }

  void createButterfliesPoints( std::vector< Vertex >& vertices )
  {
    std::random_device rd; // obtain a random number from hardware
    std::mt19937 eng( rd( ) ); // seed the generator
    std::uniform_int_distribution<> distr( -25.0f, 25.0f ); // define the range
    for ( int i = 0; i < 250; ++i )
    {
      glm::vec3 point;
      point.x = distr( eng );
      point.y = 5.0f;
      point.z = distr( eng );
      vertices.push_back( Vertex{ point } );
    }
    butterflies.ubo.numPrimitives = vertices.size( );
  }

  void createGrassPipelineAndDescriptor( void )
  {
    auto device = _window->device( );

    std::vector<Vertex> vertices;
    createGrassPoints( vertices );

    grass.uboBuffer = device->createUniformBuffer( sizeof( grass.ubo ) );
    grass.uboBuffer2 = device->createUniformBuffer( sizeof( grass.ubo2 ) );

    // Vertex buffer
    {
      uint32_t vertexBufferSize = vertices.size( ) * sizeof( Vertex );
      auto stagingBuffer = device->createBuffer( vertexBufferSize,
        vk::BufferUsageFlagBits::eTransferSrc,
        vk::MemoryPropertyFlagBits::eHostVisible |
        vk::MemoryPropertyFlagBits::eHostCoherent );
      stagingBuffer->writeData( 0, vertexBufferSize, vertices.data( ) );

      grass.vertexBuffer = device->createBuffer( vertexBufferSize,
        vk::BufferUsageFlagBits::eVertexBuffer |
        vk::BufferUsageFlagBits::eTransferDst,
        vk::MemoryPropertyFlagBits::eDeviceLocal );

      auto cmd = _window->gfxCommandPool( )->allocateCommandBuffer( );
      cmd->begin( );
      stagingBuffer->copy( cmd, grass.vertexBuffer, 0, 0, vertexBufferSize );
      cmd->end( );

      _window->gfxQueue( )->submitAndWait( cmd );
    }
    grass.numElements = uint32_t( vertices.size( ) );
    vertices.clear( );

    grass.tex = device->createTexture2D( POMPEII_EXAMPLES_IMAGES_ROUTE +
      std::string( "grass.png" ), _window->gfxCommandPool( ),
      _window->gfxQueue( ), vk::Format::eR8G8B8A8Unorm );

    // Init descriptor and pipeline layouts
    std::vector<DescriptorSetLayoutBinding> dslbs =
    {
      DescriptorSetLayoutBinding( 0,
      vk::DescriptorType::eUniformBuffer,
      vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eGeometry ),
      DescriptorSetLayoutBinding( 1,
      vk::DescriptorType::eCombinedImageSampler,
      vk::ShaderStageFlagBits::eFragment ),
      DescriptorSetLayoutBinding( 2,
      vk::DescriptorType::eUniformBuffer,
      vk::ShaderStageFlagBits::eFragment )
    };

    grass.descriptorSetLayout = device->createDescriptorSetLayout( dslbs );

    grass.pipelineLayout = device->createPipelineLayout(
      grass.descriptorSetLayout, nullptr );


    // Init descriptor set
    grass.descriptorSet = device->allocateDescriptorSet(
      descriptorPool, grass.descriptorSetLayout );
    std::vector<WriteDescriptorSet> wdss =
    {
      WriteDescriptorSet( grass.descriptorSet, 0, 0,
      vk::DescriptorType::eUniformBuffer, 1, nullptr,
      DescriptorBufferInfo( grass.uboBuffer, 0,
      sizeof( grass.ubo )
      )
      ),
      WriteDescriptorSet( grass.descriptorSet, 1, 0,
        vk::DescriptorType::eCombinedImageSampler,
        1, grass.tex->descriptor, nullptr
      ),
      WriteDescriptorSet( grass.descriptorSet, 2, 0,
        vk::DescriptorType::eUniformBuffer, 1, nullptr,
        DescriptorBufferInfo( grass.uboBuffer2, 0,
          sizeof( grass.ubo2 )
        )
      )
    };
    device->updateDescriptorSets( wdss, { } );


    // init pipeline
    auto vertexStage = device->createShaderPipelineShaderStage(
      POMPEII_EXAMPLES_SPV_ROUTE + std::string( "grass_vert.spv" ),
      vk::ShaderStageFlagBits::eVertex
    );
    auto geometryStage = device->createShaderPipelineShaderStage(
      POMPEII_EXAMPLES_SPV_ROUTE + std::string( "grass_geom.spv" ),
      vk::ShaderStageFlagBits::eGeometry
    );
    auto fragmentStage = device->createShaderPipelineShaderStage(
      POMPEII_EXAMPLES_SPV_ROUTE + std::string( "grass_frag.spv" ),
      vk::ShaderStageFlagBits::eFragment
    );
    vk::VertexInputBindingDescription binding( 0, sizeof( Vertex ),
      vk::VertexInputRate::eVertex );

    PipelineVertexInputStateCreateInfo vertexInput( binding, {
      vk::VertexInputAttributeDescription( 0, 0,
      vk::Format::eR32G32B32Sfloat, offsetof( Vertex, position )
      )
    } );
    vk::PipelineInputAssemblyStateCreateInfo assembly( { },
      vk::PrimitiveTopology::ePointList, VK_FALSE );
    PipelineViewportStateCreateInfo viewport( 1, 1 );
    vk::PipelineRasterizationStateCreateInfo rasterization( { },
      true, false, vk::PolygonMode::eFill, vk::CullModeFlagBits::eNone,
      vk::FrontFace::eClockwise, false, 0.0f, 0.0f, 0.0f, 1.0f );
    PipelineMultisampleStateCreateInfo multisample( vk::SampleCountFlagBits::e1,
      false, 0.0f, nullptr, false, false );
    vk::StencilOpState stencilOpState( vk::StencilOp::eKeep, vk::StencilOp::eKeep,
      vk::StencilOp::eKeep, vk::CompareOp::eAlways, 0, 0, 0 );
    vk::PipelineDepthStencilStateCreateInfo depthStencil( { }, true, true,
      vk::CompareOp::eLessOrEqual, false, false, stencilOpState,
      stencilOpState, 0.0f, 0.0f );

    vk::PipelineColorBlendAttachmentState colorBlendAttachment( false,
      vk::BlendFactor::eZero, vk::BlendFactor::eZero, vk::BlendOp::eAdd,
      vk::BlendFactor::eZero, vk::BlendFactor::eZero, vk::BlendOp::eAdd,
      vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
      vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA );
    PipelineColorBlendStateCreateInfo colorBlend( false, vk::LogicOp::eNoOp,
      colorBlendAttachment, { 1.0f, 1.0f, 1.0f, 1.0f } );
    PipelineDynamicStateCreateInfo dynamic( { vk::DynamicState::eViewport,
      vk::DynamicState::eScissor } );

    grass.pipeline = device->createGraphicsPipeline(
      _window->pipelineCache( ), { }, { vertexStage, geometryStage, fragmentStage },
      vertexInput, assembly, nullptr, viewport, rasterization, multisample,
      depthStencil, colorBlend, dynamic, grass.pipelineLayout,
      _window->renderPass( )
    );
  }

  void createBufferfliesPipelineAndDescriptor( void )
  {
    auto device = _window->device( );

    std::vector<Vertex> vertices;
    createButterfliesPoints( vertices );

    butterflies.uboBuffer = device->createUniformBuffer( sizeof( butterflies.ubo ) );

    // Vertex buffer
    {
      uint32_t vertexBufferSize = vertices.size( ) * sizeof( Vertex );
      auto stagingBuffer = device->createBuffer( vertexBufferSize,
        vk::BufferUsageFlagBits::eTransferSrc,
        vk::MemoryPropertyFlagBits::eHostVisible |
        vk::MemoryPropertyFlagBits::eHostCoherent );
      stagingBuffer->writeData( 0, vertexBufferSize, vertices.data( ) );

      butterflies.vertexBuffer = device->createBuffer( vertexBufferSize,
        vk::BufferUsageFlagBits::eVertexBuffer |
        vk::BufferUsageFlagBits::eTransferDst,
        vk::MemoryPropertyFlagBits::eDeviceLocal );

      auto cmd = _window->gfxCommandPool( )->allocateCommandBuffer( );
      cmd->begin( );
      stagingBuffer->copy( cmd, butterflies.vertexBuffer, 0, 0, vertexBufferSize );
      cmd->end( );

      _window->gfxQueue( )->submitAndWait( cmd );
    }
    butterflies.numElements = uint32_t( vertices.size( ) );
    vertices.clear( );

    butterflies.tex = device->createTexture2D(
      POMPEII_EXAMPLES_IMAGES_ROUTE + std::string( "butterfly.png" ),
      _window->gfxCommandPool( ), _window->gfxQueue( ),
      vk::Format::eR8G8B8A8Unorm );

    // Init descriptor and pipeline layouts
    std::vector<DescriptorSetLayoutBinding> dslbs =
    {
      DescriptorSetLayoutBinding( 0,
      vk::DescriptorType::eUniformBuffer,
      vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eGeometry ),
      DescriptorSetLayoutBinding( 1,
      vk::DescriptorType::eCombinedImageSampler,
      vk::ShaderStageFlagBits::eFragment )
    };

    butterflies.descriptorSetLayout = device->createDescriptorSetLayout( dslbs );

    butterflies.pipelineLayout = device->createPipelineLayout(
      butterflies.descriptorSetLayout, nullptr );


    // Init descriptor set
    butterflies.descriptorSet = device->allocateDescriptorSet(
      descriptorPool, butterflies.descriptorSetLayout );
    std::vector<WriteDescriptorSet> wdss =
    {
      WriteDescriptorSet( butterflies.descriptorSet, 0, 0,
      vk::DescriptorType::eUniformBuffer, 1, nullptr,
      DescriptorBufferInfo( butterflies.uboBuffer, 0,
      sizeof( butterflies.ubo )
      )
      ),
      WriteDescriptorSet( butterflies.descriptorSet, 1, 0,
        vk::DescriptorType::eCombinedImageSampler,
        1, butterflies.tex->descriptor, nullptr
      )
    };
    device->updateDescriptorSets( wdss, { } );


    // init pipeline
    auto vertexStage = device->createShaderPipelineShaderStage(
      POMPEII_EXAMPLES_SPV_ROUTE + std::string( "butterflies_vert.spv" ),
      vk::ShaderStageFlagBits::eVertex
    );
    auto geometryStage = device->createShaderPipelineShaderStage(
      POMPEII_EXAMPLES_SPV_ROUTE + std::string( "butterflies_geom.spv" ),
      vk::ShaderStageFlagBits::eGeometry
    );
    auto fragmentStage = device->createShaderPipelineShaderStage(
      POMPEII_EXAMPLES_SPV_ROUTE + std::string( "butterflies_frag.spv" ),
      vk::ShaderStageFlagBits::eFragment
    );
    vk::VertexInputBindingDescription binding( 0, sizeof( Vertex ),
      vk::VertexInputRate::eVertex );

    PipelineVertexInputStateCreateInfo vertexInput( binding, {
      vk::VertexInputAttributeDescription( 0, 0,
      vk::Format::eR32G32B32Sfloat, offsetof( Vertex, position )
      )
    } );
    vk::PipelineInputAssemblyStateCreateInfo assembly( { },
      vk::PrimitiveTopology::ePointList, VK_FALSE );
    PipelineViewportStateCreateInfo viewport( 1, 1 );
    vk::PipelineRasterizationStateCreateInfo rasterization( { },
      true, false, vk::PolygonMode::eFill, vk::CullModeFlagBits::eNone,
      vk::FrontFace::eClockwise, false, 0.0f, 0.0f, 0.0f, 1.0f );
    PipelineMultisampleStateCreateInfo multisample( vk::SampleCountFlagBits::e1,
      false, 0.0f, nullptr, false, false );
    vk::StencilOpState stencilOpState( vk::StencilOp::eKeep, vk::StencilOp::eKeep,
      vk::StencilOp::eKeep, vk::CompareOp::eAlways, 0, 0, 0 );
    vk::PipelineDepthStencilStateCreateInfo depthStencil( { }, true, true,
      vk::CompareOp::eLessOrEqual, false, false, stencilOpState,
      stencilOpState, 0.0f, 0.0f );

    vk::PipelineColorBlendAttachmentState colorBlendAttachment( false,
      vk::BlendFactor::eZero, vk::BlendFactor::eZero, vk::BlendOp::eAdd,
      vk::BlendFactor::eZero, vk::BlendFactor::eZero, vk::BlendOp::eAdd,
      vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
      vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA );
    PipelineColorBlendStateCreateInfo colorBlend( false, vk::LogicOp::eNoOp,
      colorBlendAttachment, { 1.0f, 1.0f, 1.0f, 1.0f } );
    PipelineDynamicStateCreateInfo dynamic( { vk::DynamicState::eViewport,
      vk::DynamicState::eScissor } );

    butterflies.pipeline = device->createGraphicsPipeline(
      _window->pipelineCache( ), { }, { vertexStage, geometryStage, fragmentStage },
      vertexInput, assembly, nullptr, viewport, rasterization, multisample,
      depthStencil, colorBlend, dynamic, butterflies.pipelineLayout,
      _window->renderPass( )
    );
  }

  void creaeSkyboxPipelineAndDescriptor( void )
  {
    auto device = _window->device( );

    // Vertex buffer
    {
      uint32_t vertexBufferSize = vertices.size( ) * sizeof( Vertex );
      auto stagingBuffer = device->createBuffer( vertexBufferSize,
        vk::BufferUsageFlagBits::eTransferSrc,
        vk::MemoryPropertyFlagBits::eHostVisible |
        vk::MemoryPropertyFlagBits::eHostCoherent );
      stagingBuffer->writeData( 0, vertexBufferSize, vertices.data( ) );

      skybox.vertexBuffer = device->createBuffer( vertexBufferSize,
        vk::BufferUsageFlagBits::eVertexBuffer |
        vk::BufferUsageFlagBits::eTransferDst,
        vk::MemoryPropertyFlagBits::eDeviceLocal );

      auto cmd = _window->gfxCommandPool( )->allocateCommandBuffer( );
      cmd->begin( );
      stagingBuffer->copy( cmd, skybox.vertexBuffer, 0, 0, vertexBufferSize );
      cmd->end( );

      _window->gfxQueue( )->submitAndWait( cmd );
    }

    // Index buffer
    {
      uint32_t indexBufferSize = indices.size( ) * sizeof( uint32_t );

      auto stagingBuffer = device->createBuffer( indexBufferSize,
        vk::BufferUsageFlagBits::eTransferSrc,
        vk::MemoryPropertyFlagBits::eHostVisible |
        vk::MemoryPropertyFlagBits::eHostCoherent );
      stagingBuffer->writeData( 0, indexBufferSize, indices.data( ) );

      skybox.indexBuffer = device->createBuffer( indexBufferSize,
        vk::BufferUsageFlagBits::eIndexBuffer |
        vk::BufferUsageFlagBits::eTransferDst,
        vk::MemoryPropertyFlagBits::eDeviceLocal );

      auto cmd = _window->gfxCommandPool( )->allocateCommandBuffer( );
      cmd->begin( );
      stagingBuffer->copy( cmd, skybox.indexBuffer, 0, 0, indexBufferSize );
      cmd->end( );

      _window->gfxQueue( )->submitAndWait( cmd );
    }

    // MVP buffer
    skybox.uniformMVP = device->createUniformBuffer( sizeof( skybox.ubo ) );

    std::array< std::string, 6 > cubeImages =
    {
      POMPEII_EXAMPLES_IMAGES_ROUTE + std::string( "/landCubemap/right.jpg" ),
      POMPEII_EXAMPLES_IMAGES_ROUTE + std::string( "/landCubemap/left.jpg" ),
      POMPEII_EXAMPLES_IMAGES_ROUTE + std::string( "/landCubemap/top.jpg" ),
      POMPEII_EXAMPLES_IMAGES_ROUTE + std::string( "/landCubemap/bottom.jpg" ),
      POMPEII_EXAMPLES_IMAGES_ROUTE + std::string( "/landCubemap/back.jpg" ),
      POMPEII_EXAMPLES_IMAGES_ROUTE + std::string( "/landCubemap/front.jpg" ),
    };
    skybox.tex = device->createTextureCubemap( cubeImages,
      _window->gfxCommandPool( ), _window->gfxQueue( ),
      vk::Format::eR8G8B8A8Unorm );

    // Init descriptor and pipeline layouts
    std::vector<DescriptorSetLayoutBinding> dslbs =
    {
      DescriptorSetLayoutBinding( 0, vk::DescriptorType::eUniformBuffer,
      vk::ShaderStageFlagBits::eVertex ),
      DescriptorSetLayoutBinding( 1, vk::DescriptorType::eCombinedImageSampler,
      vk::ShaderStageFlagBits::eFragment )
    };
    auto descriptorSetLayout = device->createDescriptorSetLayout( dslbs );

    skybox.pipelineLayout = device->createPipelineLayout( descriptorSetLayout );

    // init pipeline
    auto vertexStage = device->createShaderPipelineShaderStage(
      POMPEII_EXAMPLES_SPV_ROUTE + std::string( "skybox_vert.spv" ),
      vk::ShaderStageFlagBits::eVertex
    );
    auto fragmentStage = device->createShaderPipelineShaderStage(
      POMPEII_EXAMPLES_SPV_ROUTE + std::string( "skybox_frag.spv" ),
      vk::ShaderStageFlagBits::eFragment
    );
    vk::VertexInputBindingDescription binding( 0, sizeof( Vertex ),
      vk::VertexInputRate::eVertex );

    PipelineVertexInputStateCreateInfo vertexInput( binding, {
      vk::VertexInputAttributeDescription( 0, 0,
      vk::Format::eR32G32B32Sfloat, offsetof( Vertex, position ) )
    } );
    vk::PipelineInputAssemblyStateCreateInfo assembly( { },
      vk::PrimitiveTopology::eTriangleList, VK_FALSE );
    PipelineViewportStateCreateInfo viewport( 1, 1 );
    vk::PipelineRasterizationStateCreateInfo rasterization( { }, true,
      false, vk::PolygonMode::eFill, vk::CullModeFlagBits::eNone,
      vk::FrontFace::eCounterClockwise, false, 0.0f, 0.0f, 0.0f, 1.0f );
    PipelineMultisampleStateCreateInfo multisample(
      vk::SampleCountFlagBits::e1, false, 0.0f, nullptr, false, false );
    vk::StencilOpState stencilOpState( vk::StencilOp::eKeep,
      vk::StencilOp::eKeep, vk::StencilOp::eKeep, vk::CompareOp::eAlways,
      0, 0, 0 );
    vk::PipelineDepthStencilStateCreateInfo depthStencil( { }, true, true,
      vk::CompareOp::eLessOrEqual, false, false, stencilOpState,
      stencilOpState, 0.0f, 0.0f );
    vk::PipelineColorBlendAttachmentState colorBlendAttachment( false,
      vk::BlendFactor::eZero, vk::BlendFactor::eZero, vk::BlendOp::eAdd,
      vk::BlendFactor::eZero, vk::BlendFactor::eZero, vk::BlendOp::eAdd,
      vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
      vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA );
    PipelineColorBlendStateCreateInfo colorBlend( false, vk::LogicOp::eNoOp,
      colorBlendAttachment, { 1.0f, 1.0f, 1.0f, 1.0f } );
    PipelineDynamicStateCreateInfo dynamic( {
      vk::DynamicState::eViewport, vk::DynamicState::eScissor
    } );


    skybox.pipeline = device->createGraphicsPipeline( _window->pipelineCache( ),
      { }, { vertexStage, fragmentStage }, vertexInput, assembly, nullptr,
      viewport, rasterization, multisample, depthStencil, colorBlend, dynamic,
      skybox.pipelineLayout, _window->renderPass( ) );

    // Init descriptor set
    skybox.descriptorSet = device->allocateDescriptorSet(
      descriptorPool, descriptorSetLayout );
    std::vector<WriteDescriptorSet> wdss =
    {
      WriteDescriptorSet( skybox.descriptorSet, 0, 0,
      vk::DescriptorType::eUniformBuffer, 1, nullptr,
      DescriptorBufferInfo( skybox.uniformMVP, 0, sizeof( skybox.ubo ) )
      ),
      WriteDescriptorSet( skybox.descriptorSet, 1, 0,
        vk::DescriptorType::eCombinedImageSampler, 1,
        DescriptorImageInfo(
          vk::ImageLayout::eGeneral,
          skybox.tex->view,
          skybox.tex->sampler
        ), nullptr
      )
    };

    device->updateDescriptorSets( wdss, { } );
  }

  std::shared_ptr<DescriptorPool> descriptorPool;

  void initResources( void ) override
  {
    auto device = _window->device( );

    std::array<vk::DescriptorPoolSize, 2> poolSize =
    {
      vk::DescriptorPoolSize(
        vk::DescriptorType::eUniformBuffer, 4 ),
      vk::DescriptorPoolSize(
        vk::DescriptorType::eCombinedImageSampler, 3 )
    };
    descriptorPool = device->createDescriptorPool( 3, poolSize );

    createGrassPipelineAndDescriptor( );

    createBufferfliesPipelineAndDescriptor( );

    creaeSkyboxPipelineAndDescriptor( );

    // Secondary cmds are created in initSwapchainResources( ) method;
  }

  virtual void initSwapChainResources( void ) override
  {
    createSecondaryCmds( );
  }

  void createSecondaryCmds( void )
  {
    const auto size = _window->swapchainImageSize( );
    auto renderPass = _window->renderPass( );

    // Grass rendering
    {
      grass.cmd = _window->gfxCommandPool( )->
        allocateCommandBuffer( vk::CommandBufferLevel::eSecondary );

      grass.cmd->begin(
        vk::CommandBufferUsageFlagBits::eSimultaneousUse |
        vk::CommandBufferUsageFlagBits::eRenderPassContinue,
        renderPass );

      grass.cmd->bindGraphicsPipeline( grass.pipeline );
      grass.cmd->bindDescriptorSets( vk::PipelineBindPoint::eGraphics,
        grass.pipelineLayout, 0, { grass.descriptorSet }, nullptr );
      grass.cmd->bindVertexBuffer( 0, grass.vertexBuffer, 0 );

      grass.cmd->setViewportScissors( size );
      grass.cmd->draw( grass.numElements, 1, 0, 0 );

      grass.cmd->end( );
    }

    // Butterflies rendering
    {
      butterflies.cmd = _window->gfxCommandPool( )->
        allocateCommandBuffer( vk::CommandBufferLevel::eSecondary );

      butterflies.cmd->begin(
        vk::CommandBufferUsageFlagBits::eSimultaneousUse |
        vk::CommandBufferUsageFlagBits::eRenderPassContinue,
        renderPass );

      butterflies.cmd->bindGraphicsPipeline( butterflies.pipeline );
      butterflies.cmd->bindDescriptorSets( vk::PipelineBindPoint::eGraphics,
        butterflies.pipelineLayout, 0, { butterflies.descriptorSet }, nullptr );
      butterflies.cmd->bindVertexBuffer( 0, butterflies.vertexBuffer, 0 );

      butterflies.cmd->setViewportScissors( size );
      butterflies.cmd->draw( butterflies.numElements, 1, 0, 0 );

      butterflies.cmd->end( );
    }

    // Skybox rendering
    {
      skybox.cmd = _window->gfxCommandPool( )->
        allocateCommandBuffer( vk::CommandBufferLevel::eSecondary );

      skybox.cmd->begin(
        vk::CommandBufferUsageFlagBits::eSimultaneousUse |
        vk::CommandBufferUsageFlagBits::eRenderPassContinue,
        renderPass );

      skybox.cmd->bindGraphicsPipeline( skybox.pipeline );
      skybox.cmd->bindDescriptorSets( vk::PipelineBindPoint::eGraphics,
        skybox.pipelineLayout, 0, { skybox.descriptorSet }, nullptr );
      skybox.cmd->bindVertexBuffer( 0, skybox.vertexBuffer, 0 );
      skybox.cmd->bindIndexBuffer( skybox.indexBuffer, 0, vk::IndexType::eUint16 );

      skybox.cmd->setViewportScissors( size );
      skybox.cmd->drawIndexed( indices.size( ), 1, 0, 0, 1 );

      skybox.cmd->end( );
    }
  }

  void updateUniforms( void )
  {
    auto size = _window->swapchainImageSize( );

    uint32_t width = size.width, height = size.height;

    static auto startTime = std::chrono::high_resolution_clock::now( );

    auto currentTime = std::chrono::high_resolution_clock::now( );
    float time = std::chrono::duration_cast< std::chrono::milliseconds >( currentTime - startTime ).count( ) / 1000.0f;

    {
      grass.ubo.model = glm::mat4( 1.0f );

      grass.ubo.view = camera.GetViewMatrix( );
      grass.ubo.proj = glm::perspective(
        glm::radians( camera.Zoom ), ( float ) width / ( float ) height,
        0.1f, 1000.0f
      );
      grass.ubo.proj[ 1 ][ 1 ] *= -1;

      grass.ubo.time = time;
      grass.uboBuffer->set( &grass.ubo );

      grass.uboBuffer2->set( &grass.ubo2 );
    }

    {
      butterflies.ubo.model = glm::mat4( 1.0f );

      butterflies.ubo.view = grass.ubo.view;
      butterflies.ubo.proj = grass.ubo.proj;

      butterflies.ubo.time = time;

      butterflies.uboBuffer->set( &butterflies.ubo );
    }

    {
      skybox.ubo.model = glm::mat4( 1.0f );
      skybox.ubo.view = grass.ubo.view;
      skybox.ubo.proj = grass.ubo.proj;

      skybox.uniformMVP->set( &skybox.ubo );
    }
  }

  void nextFrame( void ) override
  {
    /*if ( Input::isKeyPressed( pompeii::Keyboard::Key::Esc ) )
    {
      _window->_window->close( );
    }
    if ( Input::isKeyPressed( pompeii::Keyboard::Key::Z ) )
    {
      grass.ubo2.fAlphaTest -= 0.01f;
    }
    else if ( Input::isKeyPressed( pompeii::Keyboard::Key::X ) )
    {
      grass.ubo2.fAlphaTest += 0.01f;
    }
    else if ( Input::isKeyPressed( pompeii::Keyboard::Key::C ) )
    {
      grass.ubo2.fAlphaMultiplier -= 0.01f;
    }
    else if ( Input::isKeyPressed( pompeii::Keyboard::Key::V ) )
    {
      grass.ubo2.fAlphaMultiplier += 0.01f;
    }*/

    // Mouse event
    /*{
      if ( Input::MouseButtonPress( MouseButton::Left ) )
      {
        int xPos = Input::MouseX( );
        int yPos = Input::MouseY( );
        if ( firstMouse )
        {
          lastX = xPos;
          lastY = yPos;
          firstMouse = false;
        }

        float xoffset = xPos - lastX;
        float yoffset = lastY - yPos; // reversed since y-coordinates go from bottom to top

        lastX = xPos;
        lastY = yPos;

        camera.ProcessMouseMovement( xoffset, yoffset );
      }
    }*/

    updateUniforms( );

    std::array<vk::ClearValue, 2 > clearValues;
    std::array<float, 4> ccv = { 0.0f, 0.0f, 0.0f, 1.0f };
    clearValues[ 0 ].color = vk::ClearColorValue( ccv );
    clearValues[ 1 ].depthStencil = vk::ClearDepthStencilValue( 1.0f, 0 );

    const auto size = _window->swapchainImageSize( );
    auto cmd = _window->currentCommandBuffer( );
    vk::Rect2D rect;
    rect.extent = size;
    cmd->beginRenderPass(
      _window->renderPass( ),
      _window->framebuffer( ),
      rect, clearValues, vk::SubpassContents::eSecondaryCommandBuffers
    );
    //cmd->setViewportScissors( size );
    cmd->executeCommands( { grass.cmd, butterflies.cmd, skybox.cmd } );

    cmd->endRenderPass( );

    _window->frameReady( );
  }
private:
  struct
  {
    std::shared_ptr<CommandBuffer> cmd;
    std::shared_ptr<Pipeline> pipeline;
    std::shared_ptr<PipelineLayout> pipelineLayout;
    std::shared_ptr<DescriptorSet> descriptorSet;
    std::shared_ptr<DescriptorSetLayout> descriptorSetLayout;
    std::shared_ptr<Texture2D> tex;
    std::shared_ptr<Buffer> uboBuffer;
    std::shared_ptr<Buffer> uboBuffer2;
    std::shared_ptr<Buffer> vertexBuffer;
    uint32_t numElements;

    struct
    {
      glm::mat4 model;
      glm::mat4 view;
      glm::mat4 proj;
      float time;
    } ubo;

    struct
    {
      float fAlphaTest = 0.25f;
      float fAlphaMultiplier = 1.5f;
    } ubo2;
  } grass;

  struct
  {
    std::shared_ptr<CommandBuffer> cmd;
    std::shared_ptr<Pipeline> pipeline;
    std::shared_ptr<PipelineLayout> pipelineLayout;
    std::shared_ptr<DescriptorSet> descriptorSet;
    std::shared_ptr<DescriptorSetLayout> descriptorSetLayout;
    std::shared_ptr<Texture2D> tex;
    std::shared_ptr<Buffer> uboBuffer;
    std::shared_ptr<Buffer> vertexBuffer;
    uint32_t numElements;

    struct
    {
      glm::mat4 model;
      glm::mat4 view;
      glm::mat4 proj;
      float time;
      int numPrimitives;
    } ubo;

  } butterflies;

  struct
  {
    std::shared_ptr<CommandBuffer> cmd;
    std::shared_ptr<Buffer> vertexBuffer;
    std::shared_ptr<Buffer> indexBuffer;
    std::shared_ptr<Pipeline> pipeline;
    std::shared_ptr<PipelineLayout> pipelineLayout;
    std::shared_ptr<DescriptorSet> descriptorSet;

    std::shared_ptr<TextureCubemap> tex;
    std::shared_ptr<DescriptorPool> descriptorPool;

    struct
    {
      glm::mat4 model;
      glm::mat4 view;
      glm::mat4 proj;
    } ubo;

    std::shared_ptr<Buffer> uniformMVP;
  } skybox;
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
  VulkanWindow app( 500, 500, "GLFWR Glassland", true );
  app.show( );
  return EXIT_SUCCESS;
}