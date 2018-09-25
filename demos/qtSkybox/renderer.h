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

#pragma once

#include <qtPompeii/qtPompeii.h>

#include "../utils/Camera.h"
#include <pompeiiUtils/pompeiiUtils.h>
using namespace pompeii;

#include <routes.h>
#include "vulkanwindow.h"

#include <unordered_map>

class Renderer: public pompeii::qt::VulkanWindowRenderer
{
public:
	Renderer( VulkanWindow* w );

  bool modeReflect = true;
protected:
  // camera
  Camera camera;
  // timing
  float deltaTime = 0.0f; // time between current frame and last frame
  float lastFrame = 0.0f;

  struct
  {
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
  } uboVS;

  struct
  {
    glm::vec3 viewPos;
  } uboFS;

  struct Vertex
  {
    glm::vec3 pos;
  };

  const float side = 5.0f;
  const float side2 = side * 0.5f;
  const std::vector<Vertex> vertices =
  {
    { { -side2, -side2,  side2 } },
    { { side2, -side2,  side2 } },
    { { -side2,  side2,  side2 } },
    { { side2,  side2,  side2 } },

    { { -side2, -side2, -side2 } },
    { { side2, -side2, -side2 } },
    { { -side2,  side2, -side2 } },
    { { side2,  side2, -side2 } },

    { { side2, -side2, -side2 } },
    { { side2, -side2,  side2 } },
    { { side2,  side2, -side2 } },
    { { side2,  side2,  side2 } },

    { { -side2, -side2, -side2 } },
    { { -side2, -side2,  side2 } },
    { { -side2,  side2, -side2 } },
    { { -side2,  side2,  side2 } },

    { { -side2,  side2, -side2 } },
    { { -side2,  side2,  side2 } },
    { { side2,  side2, -side2 } },
    { { side2,  side2,  side2 } },

    { { -side2, -side2, -side2 } },
    { { -side2, -side2,  side2 } },
    { { side2, -side2, -side2 } },
    { { side2, -side2,  side2 } }
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

    // Vertex buffer
    {
      uint32_t vertexBufferSize = vertices.size( ) * sizeof( Vertex );
      auto cmd = _window->gfxCommandPool( )->allocateCommandBuffer( );
      cmd->begin( );

      skybox.vertexBuffer = device->createBuffer( vertexBufferSize,
        vk::BufferUsageFlagBits::eVertexBuffer |
        vk::BufferUsageFlagBits::eTransferDst,
        vk::MemoryPropertyFlagBits::eDeviceLocal );
      skybox.vertexBuffer->update<Vertex>( cmd, 0, { uint32_t( vertices.size( ) ),
        vertices.data( ) } );
      cmd->end( );
      _window->gfxQueue( )->submitAndWait( cmd );
    }

    // Index buffer
    {
      uint32_t indexBufferSize = indices.size( ) * sizeof( uint32_t );
      auto cmd = _window->gfxCommandPool( )->allocateCommandBuffer( );
      cmd->begin( );

      skybox.indexBuffer = device->createBuffer( indexBufferSize,
        vk::BufferUsageFlagBits::eIndexBuffer |
        vk::BufferUsageFlagBits::eTransferDst,
        vk::MemoryPropertyFlagBits::eDeviceLocal );
      skybox.indexBuffer->update<uint16_t>( cmd, 0, { uint32_t( indices.size( ) ),
        indices.data( ) } );
      cmd->end( );
      _window->gfxQueue( )->submitAndWait( cmd );
    }

    // MVP buffer
    uniformMVP = device->createUniformBuffer( sizeof( uboVS ) );

    uniformViewPos = device->createUniformBuffer( sizeof( uboFS ) );

    setModel( "wolf" );

    std::array< std::string, 6 > cubeImages =
    {
      POMPEII_EXAMPLES_IMAGES_ROUTE + std::string( "/spaceCubemap/right.png" ),
      POMPEII_EXAMPLES_IMAGES_ROUTE + std::string( "/spaceCubemap/left.png" ),
      POMPEII_EXAMPLES_IMAGES_ROUTE + std::string( "/spaceCubemap/top.png" ),
      POMPEII_EXAMPLES_IMAGES_ROUTE + std::string( "/spaceCubemap/bottom.png" ),
      POMPEII_EXAMPLES_IMAGES_ROUTE + std::string( "/spaceCubemap/back.png" ),
      POMPEII_EXAMPLES_IMAGES_ROUTE + std::string( "/spaceCubemap/front.png" ),
    };

    tex = device->createTextureCubemap( cubeImages,
      _window->gfxCommandPool( ), _window->gfxQueue( ),
      vk::Format::eR8G8B8A8Unorm );

    std::array<vk::DescriptorPoolSize, 2> poolSize =
    {
      vk::DescriptorPoolSize( vk::DescriptorType::eUniformBuffer, 3 ),
      vk::DescriptorPoolSize( vk::DescriptorType::eCombinedImageSampler, 2 )
    };

    descriptorPool = device->createDescriptorPool( 2, poolSize );

    {
      // Init descriptor and pipeline layouts
      std::vector<DescriptorSetLayoutBinding> dslbs =
      {
        DescriptorSetLayoutBinding( 0, vk::DescriptorType::eUniformBuffer,
        vk::ShaderStageFlagBits::eVertex
        ),
        DescriptorSetLayoutBinding( 1, vk::DescriptorType::eCombinedImageSampler,
          vk::ShaderStageFlagBits::eFragment
        ),
        DescriptorSetLayoutBinding( 2, vk::DescriptorType::eUniformBuffer,
          vk::ShaderStageFlagBits::eFragment
        ),
      };
      auto descriptorSetLayout = device->createDescriptorSetLayout( dslbs );

      model.pipelineLayout = device->createPipelineLayout( descriptorSetLayout );

      // init pipeline
      auto vertexStage = device->createShaderPipelineShaderStage(
        POMPEII_EXAMPLES_SPV_ROUTE + std::string( "reflect_vert.spv" ),
        vk::ShaderStageFlagBits::eVertex
      );
      auto fragmentStage = device->createShaderPipelineShaderStage(
        POMPEII_EXAMPLES_SPV_ROUTE + std::string( "reflect_frag.spv" ),
        vk::ShaderStageFlagBits::eFragment
      );

      PipelineVertexInputStateCreateInfo vertexInput(
        vk::VertexInputBindingDescription( 0, sizeof( pompeii::utility::Vertex ),
          vk::VertexInputRate::eVertex ),
          {
            vk::VertexInputAttributeDescription( 0, 0, vk::Format::eR32G32B32Sfloat,
            offsetof( pompeii::utility::Vertex, position )
            ),
        vk::VertexInputAttributeDescription( 1, 0, vk::Format::eR32G32B32Sfloat,
          offsetof( pompeii::utility::Vertex, normal )
        )
          }
      );
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
        vk::CompareOp::eLessOrEqual, false, false, stencilOpState, stencilOpState,
        0.0f, 0.0f );
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

      model.pipelines.reflect = device->createGraphicsPipeline(
        _window->pipelineCache( ), { }, { vertexStage, fragmentStage },
        vertexInput, assembly, nullptr, viewport, rasterization, multisample,
        depthStencil, colorBlend, dynamic, model.pipelineLayout,
        _window->renderPass( ) );

      vertexStage = device->createShaderPipelineShaderStage(
        POMPEII_EXAMPLES_SPV_ROUTE + std::string( "refract_vert.spv" ),
        vk::ShaderStageFlagBits::eVertex
      );
      fragmentStage = device->createShaderPipelineShaderStage(
        POMPEII_EXAMPLES_SPV_ROUTE + std::string( "refract_frag.spv" ),
        vk::ShaderStageFlagBits::eFragment
      );

      model.pipelines.refract = device->createGraphicsPipeline(
        _window->pipelineCache( ), { }, { vertexStage, fragmentStage },
        vertexInput, assembly, nullptr, viewport, rasterization, multisample,
        depthStencil, colorBlend, dynamic, model.pipelineLayout,
        _window->renderPass( ) );

      // Init descriptor set
      model.descriptorSet = device->allocateDescriptorSet( descriptorPool,
        descriptorSetLayout );
      std::vector<WriteDescriptorSet> wdss =
      {
        WriteDescriptorSet( model.descriptorSet, 0, 0,
        vk::DescriptorType::eUniformBuffer, 1, nullptr,
        DescriptorBufferInfo( uniformMVP, 0, sizeof( uboVS ) )
        ),
        WriteDescriptorSet( model.descriptorSet, 1, 0,
          vk::DescriptorType::eCombinedImageSampler, 1,
          tex->descriptor, nullptr
        ),
        WriteDescriptorSet( model.descriptorSet, 2, 0,
          vk::DescriptorType::eUniformBuffer, 1, nullptr,
          DescriptorBufferInfo( uniformViewPos, 0, sizeof( uboFS ) )
        ),
      };
      device->updateDescriptorSets( wdss, { } );
    }

    {
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
        vk::Format::eR32G32B32Sfloat, offsetof( Vertex, pos ) )
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
        DescriptorBufferInfo( uniformMVP, 0, sizeof( uboVS ) )
        ),
        WriteDescriptorSet( skybox.descriptorSet, 1, 0,
          vk::DescriptorType::eCombinedImageSampler, 1,
          DescriptorImageInfo(
            vk::ImageLayout::eGeneral, tex->view, tex->sampler
          ), nullptr
        )
      };

      device->updateDescriptorSets( wdss, { } );
    }
  }

  void updateMVP( void )
  {
    auto size = _window->swapchainImageSize( );

    uint32_t width = size.width, height = size.height;

    static auto startTime = std::chrono::high_resolution_clock::now( );

    /*auto currentTime = std::chrono::high_resolution_clock::now( );
    float time = std::chrono::duration_cast<std::chrono::milliseconds>(
      currentTime - startTime ).count( ) / 1000.0f;

    float currentFrame = time;
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;*/

    static float time = 0.0f;
    if ( _window->isContinuosRendering( ) )
    {
      time += 0.0025f;
    }

    uboVS.model = glm::rotate( glm::mat4( 1.0f ), time * glm::radians( 25.0f ),
      glm::vec3( 0.0f, 1.0f, 0.0f ) );
    uboVS.view = camera.GetViewMatrix( );
    uboVS.proj = glm::perspective( glm::radians( camera.Zoom ),
      ( float ) width / ( float ) height, 0.1f, 100.0f );
    uboVS.proj[ 1 ][ 1 ] *= -1;

    uniformMVP->set( &uboVS );

    uboFS.viewPos = camera.Position;
    uniformViewPos->set( &uboFS );
  }

  void nextFrame( void ) override
  {
    updateMVP( );

    std::array<vk::ClearValue, 2 > clearValues;
    std::array<float, 4> ccv = { 0.2f, 0.3f, 0.3f, 1.0f };
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

    cmd->bindGraphicsPipeline( skybox.pipeline );
    cmd->bindDescriptorSets( vk::PipelineBindPoint::eGraphics,
      skybox.pipelineLayout, 0, { skybox.descriptorSet }, nullptr );
    cmd->bindVertexBuffer( 0, skybox.vertexBuffer, 0 );
    cmd->bindIndexBuffer( skybox.indexBuffer, 0, vk::IndexType::eUint16 );
    cmd->setViewportScissors( _window->swapchainImageSize( ) );

    cmd->drawIndexed( indices.size( ), 1, 0, 0, 1 );

    if ( modeReflect )
    {
      cmd->bindGraphicsPipeline( model.pipelines.reflect );
    }
    else
    {
      cmd->bindGraphicsPipeline( model.pipelines.refract );
    }
    cmd->bindDescriptorSets( vk::PipelineBindPoint::eGraphics,
      model.pipelineLayout, 0, { model.descriptorSet }, nullptr );

    geometry->render( cmd );

    cmd->endRenderPass( );

    _window->frameReady( );
  }

private:
  VulkanWindow *_window;
  struct
  {
    std::shared_ptr<Buffer> vertexBuffer;
    std::shared_ptr<Buffer> indexBuffer;
    std::shared_ptr<Pipeline> pipeline;
    std::shared_ptr<PipelineLayout> pipelineLayout;
    std::shared_ptr<DescriptorSet> descriptorSet;
  } skybox;

  struct
  {
    struct
    {
      std::shared_ptr<Pipeline> refract;
      std::shared_ptr<Pipeline> reflect;
    } pipelines;
    std::shared_ptr<PipelineLayout> pipelineLayout;
    std::shared_ptr<DescriptorSet> descriptorSet;
  } model;

  std::shared_ptr<TextureCubemap> tex;
  std::shared_ptr<DescriptorPool> descriptorPool;

  std::shared_ptr<pompeii::utility::Geometry> geometry;
  std::shared_ptr<Buffer> uniformMVP;
  std::shared_ptr<Buffer> uniformViewPos;

  std::unordered_map < std::string, std::shared_ptr< pompeii::utility::Geometry > > geometries;

public:
  void setModel( const std::string& name )
  {
    auto device = _window->device( );
    if ( geometries.find( name ) == geometries.end( ) )
    {
      // Model don't found
      geometries[ name ] = std::make_shared<pompeii::utility::Geometry>( device,
          POMPEII_EXAMPLES_MESHES_ROUTE + name + std::string( ".obj_" ) );
    }
    geometry = geometries[ name ];
  }
};