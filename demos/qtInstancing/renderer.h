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

using namespace pompeii;

#include <routes.h>
#include "vulkanwindow.h"


#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <random>

#define VERTEX_BUFFER_BIND_ID 0
#define INSTANCE_BUFFER_BIND_ID 1

class Renderer: public pompeii::qt::VulkanWindowRenderer
{
public:
	Renderer( VulkanWindow* w );

  bool modeReflect = true;
protected:

  struct Vertex
  {
    glm::vec3 position;
  };

  struct InstanceData
  {
    glm::vec3 offset;
    glm::vec4 color;
    glm::vec4 orientationStart;
    glm::vec4 orientationEnd;
  };

  struct
  {
    glm::mat4 modelViewMatrix;
    glm::mat4 projectionMatrix;
    float sineTime;
  } uboVS;

  struct
  {
    float time;
  } uboFS;

  std::shared_ptr< Buffer > vertexBuffer;
  std::shared_ptr< Buffer > indexBuffer;
  std::shared_ptr< Buffer > instanceBuffer;

  std::shared_ptr< Texture2D > tex;
  std::shared_ptr< DescriptorSetLayout > descriptorSetLayout;
  std::shared_ptr< DescriptorSet > descriptorSet;
  std::shared_ptr< PipelineLayout > pipelineLayout;
  std::shared_ptr< Pipeline > pipeline;
  std::shared_ptr< Buffer> vsBuffer;
  std::shared_ptr< Buffer> fsBuffer;

  int INSTANCE_COUNT = 50000;
  std::vector<Vertex> vertices;
  std::vector<uint16_t> indices;

  void initResources( void ) override
  {
    auto device = _window->device( );

    vsBuffer = device->createUniformBuffer( sizeof( uboVS ) );
    fsBuffer = device->createUniformBuffer( sizeof( uboFS ) );

    vertices = {
      { { 0.025f,  0.025f, 0.0f } },  // top right
      { { 0.025f, -0.025f, 0.0f } },  // bottom right
      { { -0.025f, -0.025f, 0.0f } },  // bottom left
      { { -0.025f,  0.025f, 0.0f } }   // top left 
    };

    indices = { 0, 1, 3, 1, 2, 3 };

    std::vector<InstanceData> instanceData;
    instanceData.resize( INSTANCE_COUNT );

    std::mt19937 rndGenerator( time( nullptr ) );
    std::uniform_real_distribution<float> uniformDist( 0.0f, 1.0f );
    std::uniform_real_distribution<float> uniformDist2( 0.25f, 1.0f );
    for ( int i = 0; i < INSTANCE_COUNT; ++i )
    {
      InstanceData data;
      // offsets
      data.offset = glm::vec3(
        uniformDist( rndGenerator ) - 0.5f,
        uniformDist( rndGenerator ) - 0.5f,
        uniformDist( rndGenerator ) - 0.5f
      );
      // colors

      data.color = glm::vec4(
        uniformDist( rndGenerator ), uniformDist( rndGenerator ),
        uniformDist( rndGenerator ), uniformDist2( rndGenerator ) // [0.25,1] alpha
      );

      // orientation start
      glm::vec4 vector(
        uniformDist( rndGenerator ) * 2.0f - 1.0f,
        uniformDist( rndGenerator ) * 2.0f - 1.0f,
        uniformDist( rndGenerator ) * 2.0f - 1.0f,
        uniformDist( rndGenerator ) * 2.0f - 1.0f
      );
      glm::normalize( vector );

      data.orientationStart = glm::vec4( vector.x, vector.y, vector.z, vector.w );

      // orientation end

      vector = glm::vec4(
        uniformDist( rndGenerator ) * 2.0f - 1.0f,
        uniformDist( rndGenerator ) * 2.0f - 1.0f,
        uniformDist( rndGenerator ) * 2.0f - 1.0f,
        uniformDist( rndGenerator ) * 2.0f - 1.0f
      );
      glm::normalize( vector );

      data.orientationEnd = glm::vec4( vector.x, vector.y, vector.z, vector.w );

      instanceData[ i ] = data;
    }

    // Vertex buffer
    {
      uint32_t vertexBufferSize = vertices.size( ) * sizeof( Vertex );
      auto stagingBuffer = device->createBuffer( vertexBufferSize,
        vk::BufferUsageFlagBits::eTransferSrc,
        vk::MemoryPropertyFlagBits::eHostVisible |
        vk::MemoryPropertyFlagBits::eHostCoherent );
      stagingBuffer->writeData( 0, vertexBufferSize, vertices.data( ) );

      vertexBuffer = device->createBuffer( vertexBufferSize,
        vk::BufferUsageFlagBits::eVertexBuffer |
        vk::BufferUsageFlagBits::eTransferDst,
        vk::MemoryPropertyFlagBits::eDeviceLocal );

      auto cmd = _window->gfxCommandPool( )->allocateCommandBuffer( );
      cmd->begin( );
      stagingBuffer->copy( cmd, vertexBuffer, 0, 0, vertexBufferSize );
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

      indexBuffer = device->createBuffer( indexBufferSize,
        vk::BufferUsageFlagBits::eIndexBuffer |
        vk::BufferUsageFlagBits::eTransferDst,
        vk::MemoryPropertyFlagBits::eDeviceLocal );

      auto cmd = _window->gfxCommandPool( )->allocateCommandBuffer( );
      cmd->begin( );
      stagingBuffer->copy( cmd, indexBuffer, 0, 0, indexBufferSize );
      cmd->end( );

      _window->gfxQueue( )->submitAndWait( cmd );
    }

    // Instance buffer
    {
      uint32_t instancingBufferSize = instanceData.size( ) * sizeof( InstanceData );
      instanceBuffer = device->createVertexBuffer( instancingBufferSize );
      instanceBuffer->writeData( 0, instancingBufferSize, instanceData.data( ) );
    }

    std::vector<DescriptorSetLayoutBinding> dslbs =
    {
      DescriptorSetLayoutBinding( 0, vk::DescriptorType::eUniformBuffer,
      vk::ShaderStageFlagBits::eVertex
      ),
      DescriptorSetLayoutBinding( 1, vk::DescriptorType::eUniformBuffer,
        vk::ShaderStageFlagBits::eFragment
      )
    };

    descriptorSetLayout = device->createDescriptorSetLayout( dslbs );

    pipelineLayout = device->createPipelineLayout( descriptorSetLayout, nullptr );

    auto descriptorPool = device->createDescriptorPool( 1, {
      vk::DescriptorPoolSize( vk::DescriptorType::eUniformBuffer, 2 )
    } );

    descriptorSet = device->allocateDescriptorSet( descriptorPool, descriptorSetLayout );

    std::vector<WriteDescriptorSet> wdss =
    {
      WriteDescriptorSet( descriptorSet, 0, 0,
      vk::DescriptorType::eUniformBuffer, 1, nullptr,
      DescriptorBufferInfo(
        vsBuffer, 0, sizeof( uboVS )
      )
      ),WriteDescriptorSet( descriptorSet, 1, 0,
        vk::DescriptorType::eUniformBuffer, 1, nullptr,
        DescriptorBufferInfo(
          fsBuffer, 0, sizeof( uboFS )
        )
      )
    };

    device->updateDescriptorSets( wdss, { } );

    auto vertexStage = device->createShaderPipelineShaderStage(
      POMPEII_EXAMPLES_SPV_ROUTE + std::string( "instancing_vert.spv" ),
      vk::ShaderStageFlagBits::eVertex
    );
    auto fragmentStage = device->createShaderPipelineShaderStage(
      POMPEII_EXAMPLES_SPV_ROUTE + std::string( "instancing_frag.spv" ),
      vk::ShaderStageFlagBits::eFragment
    );

    PipelineVertexInputStateCreateInfo vertexInput( {
      // Binding point 0: Mesh vertex layout description at per-vertex rate
      vk::VertexInputBindingDescription( VERTEX_BUFFER_BIND_ID,
      sizeof( Vertex ), vk::VertexInputRate::eVertex
      ),
      // Binding point 1: Instanced data at per-instance rate
      vk::VertexInputBindingDescription( INSTANCE_BUFFER_BIND_ID,
        sizeof( InstanceData ), vk::VertexInputRate::eInstance
      )
    }, {
      // Position attribute
      vk::VertexInputAttributeDescription( 0, VERTEX_BUFFER_BIND_ID,
        vk::Format::eR32G32B32Sfloat, 0
      ),
      // Offset instancing attribute
      vk::VertexInputAttributeDescription( 1, INSTANCE_BUFFER_BIND_ID,
        vk::Format::eR32G32B32Sfloat, offsetof( InstanceData, offset )
      ),
      // Color instancing attribute
      vk::VertexInputAttributeDescription( 2, INSTANCE_BUFFER_BIND_ID,
        vk::Format::eR32G32B32A32Sfloat, offsetof( InstanceData, color )
      ),
      // Orientation start instancing attribute
      vk::VertexInputAttributeDescription( 3, INSTANCE_BUFFER_BIND_ID,
        vk::Format::eR32G32B32A32Sfloat, offsetof( InstanceData, orientationStart )
      ),
      // Orientation end instancing attribute
      vk::VertexInputAttributeDescription( 4, INSTANCE_BUFFER_BIND_ID,
        vk::Format::eR32G32B32A32Sfloat, offsetof( InstanceData, orientationEnd )
      ),
    } );

    vk::PipelineInputAssemblyStateCreateInfo assembly( { },
      vk::PrimitiveTopology::eTriangleList, VK_FALSE );
    PipelineViewportStateCreateInfo viewport( 1, 1 );
    vk::PipelineRasterizationStateCreateInfo rasterization( { }, true,
      false, vk::PolygonMode::eFill, vk::CullModeFlagBits::eNone,
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
    vk::PipelineColorBlendAttachmentState colorBlendAttachment( true,
      vk::BlendFactor::eSrcAlpha, vk::BlendFactor::eOneMinusSrcAlpha, vk::BlendOp::eAdd,
      vk::BlendFactor::eOne, vk::BlendFactor::eOneMinusSrcAlpha, vk::BlendOp::eAdd,
      vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
      vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA
    );
    PipelineColorBlendStateCreateInfo colorBlend( false, vk::LogicOp::eNoOp,
      colorBlendAttachment, { 1.0f, 1.0f, 1.0f, 1.0f }
    );
    PipelineDynamicStateCreateInfo dynamic( {
      vk::DynamicState::eViewport, vk::DynamicState::eScissor
    } );

    pipeline = device->createGraphicsPipeline( _window->pipelineCache( ), { },
      { vertexStage, fragmentStage }, vertexInput, assembly, nullptr,
      viewport, rasterization, multisample, depthStencil, colorBlend, dynamic,
      pipelineLayout, _window->renderPass( )
    );
  }

  void updateBuffers( void )
  {
    auto size = _window->swapchainImageSize( );

    uint32_t width = size.width, height = size.height;

    static auto startTime = std::chrono::high_resolution_clock::now( );

    auto currentTime = std::chrono::high_resolution_clock::now( );
    float time = std::chrono::duration_cast<std::chrono::milliseconds>( currentTime - startTime ).count( ) / 1000.0f;

    uboVS.modelViewMatrix = glm::lookAt(
      glm::vec3( 2.0f, 2.0f, 0.0f ),
      glm::vec3( 0.0f, 0.0f, 0.0f ),
      glm::vec3( 0.0f, 0.0f, 1.0f )
    );

    uboVS.projectionMatrix = glm::perspective( glm::radians( 50.0f ), width / ( float ) height, 0.1f, 10.0f );
    uboVS.projectionMatrix[ 1 ][ 1 ] *= -1;

    uboFS.time = time;// *0.025f;

    uboVS.sineTime = std::sin( uboFS.time * 0.5f );

    vsBuffer->writeData( 0, sizeof( uboVS ), &uboVS );
    vsBuffer->writeData( 0, sizeof( uboVS ), &uboVS );
  }
  
  void nextFrame( void ) override
  {
    updateBuffers( );

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
      rect, clearValues, vk::SubpassContents::eInline
    );

    cmd->bindGraphicsPipeline( pipeline );
    cmd->bindDescriptorSets( vk::PipelineBindPoint::eGraphics, pipelineLayout,
      0, { descriptorSet }, { } );

    // Binding point 0 : Mesh vertex buffer
    cmd->bindVertexBuffer( VERTEX_BUFFER_BIND_ID, vertexBuffer, 0 );
    cmd->bindIndexBuffer( indexBuffer, 0, vk::IndexType::eUint16 );
    // Binding point 1 : Instance data buffer
    cmd->bindVertexBuffer( INSTANCE_BUFFER_BIND_ID, instanceBuffer, 0 );

    cmd->setViewportScissors( size );
    cmd->drawIndexed( indices.size( ), INSTANCE_COUNT, 0, 0, 1 );

    cmd->endRenderPass( );

    _window->frameReady( );
  }

private:
  VulkanWindow *_window;

public:
  void changeNumInstancing( int value )
  {
    std::cout << "Instancing: " << value << std::endl;
    INSTANCE_COUNT = value;
  }
};