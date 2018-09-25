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
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#ifndef M_PI
  #define M_PI 3.14159
#endif

#include <routes.h>

std::shared_ptr<PipelineLayout> pipelineLayout;

struct Vertex
{
  glm::vec3 pos;
  glm::vec3 normal;

  Vertex( const glm::vec3& p, const glm::vec3& n )
  {
    pos = p;
    normal = n;
  }
};

struct GearInfo
{
  float innerRadius;
  float outerRadius;
  float width;
  int numTeeth;
  float toothDepth;
  glm::vec3 color;
  glm::vec3 pos;
  float rotSpeed;
  float rotOffset;
};


struct PushCtesVS
{
  glm::mat4 model;
  glm::vec3 color;
} pushCtesVS;


class VulkanGear: public pompeii::VulkanResource
{
public:
  glm::mat4 model;
  glm::vec3 color;
  VulkanGear( const std::shared_ptr<Device>& device )
    : pompeii::VulkanResource( device )
  {
  }
  void draw( std::shared_ptr<CommandBuffer> cmd, float timer )
  {
    glm::vec3 rotation = glm::vec3( -23.75f, 41.25f, 21.0f );
    model = glm::mat4( 1.0f );
    model = glm::translate( model, pos );
    rotation.z = ( rotSpeed * timer ) + rotOffset;
    model = glm::rotate( model, glm::radians( rotation.z ), glm::vec3( 0.0f, 0.0f, 1.0f ) );

    pushCtesVS.model = model;
    pushCtesVS.color = color;

    cmd->pushConstants<PushCtesVS>( pipelineLayout, 
      vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment, 0, pushCtesVS );
    cmd->bindVertexBuffer( 0, vertexBuffer, 0 );
    cmd->bindIndexBuffer( indexBuffer, 0, vk::IndexType::eUint32 );
    cmd->drawIndexed( indexCount, 1, 0, 0, 1 );
  }
protected:
  int32_t newVertex( std::vector<Vertex>* vBuffer, float x, float y, float z, 
    const glm::vec3& normal )
  {
    Vertex v( glm::vec3( x, y, z ), normal/*, color*/ );
    vBuffer->push_back( v );
    return static_cast<int32_t>( vBuffer->size( ) ) - 1;
  }

  void newFace( std::vector<uint32_t> *iBuffer, int a, int b, int c )
  {
    iBuffer->push_back( a );
    iBuffer->push_back( b );
    iBuffer->push_back( c );
  }
public:
  void generate( GearInfo *gearinfo, std::shared_ptr<CommandPool> cmdPool, 
    std::shared_ptr<Queue> queue )
  {
    this->color = gearinfo->color;
    this->pos = gearinfo->pos;
    this->rotOffset = gearinfo->rotOffset;
    this->rotSpeed = gearinfo->rotSpeed;

    std::vector<Vertex> vBuffer;
    std::vector<uint32_t> iBuffer;

    int i;// , j;
    float r0, r1, r2;
    float ta, da;
    float u1, v1, u2, v2, len;
    float cos_ta, cos_ta_1da, cos_ta_2da, cos_ta_3da, cos_ta_4da;
    float sin_ta, sin_ta_1da, sin_ta_2da, sin_ta_3da, sin_ta_4da;
    int32_t ix0, ix1, ix2, ix3, ix4, ix5;

    r0 = gearinfo->innerRadius;
    r1 = gearinfo->outerRadius - gearinfo->toothDepth / 2.0f;
    r2 = gearinfo->outerRadius + gearinfo->toothDepth / 2.0f;
    da = 2.0f * M_PI / gearinfo->numTeeth / 4.0f;

    glm::vec3 normal;

    for ( i = 0; i < gearinfo->numTeeth; i++ )
    {
      ta = i * 2.0f * M_PI / gearinfo->numTeeth;

      cos_ta = cos( ta );
      cos_ta_1da = cos( ta + da );
      cos_ta_2da = cos( ta + 2.0f * da );
      cos_ta_3da = cos( ta + 3.0f * da );
      cos_ta_4da = cos( ta + 4.0f * da );
      sin_ta = sin( ta );
      sin_ta_1da = sin( ta + da );
      sin_ta_2da = sin( ta + 2.0f * da );
      sin_ta_3da = sin( ta + 3.0f * da );
      sin_ta_4da = sin( ta + 4.0f * da );

      u1 = r2 * cos_ta_1da - r1 * cos_ta;
      v1 = r2 * sin_ta_1da - r1 * sin_ta;
      len = sqrt( u1 * u1 + v1 * v1 );
      u1 /= len;
      v1 /= len;
      u2 = r1 * cos_ta_3da - r2 * cos_ta_2da;
      v2 = r1 * sin_ta_3da - r2 * sin_ta_2da;

      // front face
      normal = glm::vec3( 0.0f, 0.0f, 1.0f );
      ix0 = newVertex( &vBuffer, r0 * cos_ta, r0 * sin_ta, gearinfo->width * 0.5f, normal );
      ix1 = newVertex( &vBuffer, r1 * cos_ta, r1 * sin_ta, gearinfo->width * 0.5f, normal );
      ix2 = newVertex( &vBuffer, r0 * cos_ta, r0 * sin_ta, gearinfo->width * 0.5f, normal );
      ix3 = newVertex( &vBuffer, r1 * cos_ta_3da, r1 * sin_ta_3da, gearinfo->width * 0.5f, normal );
      ix4 = newVertex( &vBuffer, r0 * cos_ta_4da, r0 * sin_ta_4da, gearinfo->width * 0.5f, normal );
      ix5 = newVertex( &vBuffer, r1 * cos_ta_4da, r1 * sin_ta_4da, gearinfo->width * 0.5f, normal );
      newFace( &iBuffer, ix0, ix1, ix2 );
      newFace( &iBuffer, ix1, ix3, ix2 );
      newFace( &iBuffer, ix2, ix3, ix4 );
      newFace( &iBuffer, ix3, ix5, ix4 );

      // front sides of teeth
      normal = glm::vec3( 0.0f, 0.0f, 1.0f );
      ix0 = newVertex( &vBuffer, r1 * cos_ta, r1 * sin_ta, gearinfo->width * 0.5f, normal );
      ix1 = newVertex( &vBuffer, r2 * cos_ta_1da, r2 * sin_ta_1da, gearinfo->width * 0.5f, normal );
      ix2 = newVertex( &vBuffer, r1 * cos_ta_3da, r1 * sin_ta_3da, gearinfo->width * 0.5f, normal );
      ix3 = newVertex( &vBuffer, r2 * cos_ta_2da, r2 * sin_ta_2da, gearinfo->width * 0.5f, normal );
      newFace( &iBuffer, ix0, ix1, ix2 );
      newFace( &iBuffer, ix1, ix3, ix2 );

      // back face 
      normal = glm::vec3( 0.0f, 0.0f, -1.0f );
      ix0 = newVertex( &vBuffer, r1 * cos_ta, r1 * sin_ta, -gearinfo->width * 0.5f, normal );
      ix1 = newVertex( &vBuffer, r0 * cos_ta, r0 * sin_ta, -gearinfo->width * 0.5f, normal );
      ix2 = newVertex( &vBuffer, r1 * cos_ta_3da, r1 * sin_ta_3da, -gearinfo->width * 0.5f, normal );
      ix3 = newVertex( &vBuffer, r0 * cos_ta, r0 * sin_ta, -gearinfo->width * 0.5f, normal );
      ix4 = newVertex( &vBuffer, r1 * cos_ta_4da, r1 * sin_ta_4da, -gearinfo->width * 0.5f, normal );
      ix5 = newVertex( &vBuffer, r0 * cos_ta_4da, r0 * sin_ta_4da, -gearinfo->width * 0.5f, normal );
      newFace( &iBuffer, ix0, ix1, ix2 );
      newFace( &iBuffer, ix1, ix3, ix2 );
      newFace( &iBuffer, ix2, ix3, ix4 );
      newFace( &iBuffer, ix3, ix5, ix4 );

      // back sides of teeth 
      normal = glm::vec3( 0.0f, 0.0f, -1.0f );
      ix0 = newVertex( &vBuffer, r1 * cos_ta_3da, r1 * sin_ta_3da, -gearinfo->width * 0.5f, normal );
      ix1 = newVertex( &vBuffer, r2 * cos_ta_2da, r2 * sin_ta_2da, -gearinfo->width * 0.5f, normal );
      ix2 = newVertex( &vBuffer, r1 * cos_ta, r1 * sin_ta, -gearinfo->width * 0.5f, normal );
      ix3 = newVertex( &vBuffer, r2 * cos_ta_1da, r2 * sin_ta_1da, -gearinfo->width * 0.5f, normal );
      newFace( &iBuffer, ix0, ix1, ix2 );
      newFace( &iBuffer, ix1, ix3, ix2 );

      // draw outward faces of teeth 
      normal = glm::vec3( v1, -u1, 0.0f );
      ix0 = newVertex( &vBuffer, r1 * cos_ta, r1 * sin_ta, gearinfo->width * 0.5f, normal );
      ix1 = newVertex( &vBuffer, r1 * cos_ta, r1 * sin_ta, -gearinfo->width * 0.5f, normal );
      ix2 = newVertex( &vBuffer, r2 * cos_ta_1da, r2 * sin_ta_1da, gearinfo->width * 0.5f, normal );
      ix3 = newVertex( &vBuffer, r2 * cos_ta_1da, r2 * sin_ta_1da, -gearinfo->width * 0.5f, normal );
      newFace( &iBuffer, ix0, ix1, ix2 );
      newFace( &iBuffer, ix1, ix3, ix2 );

      normal = glm::vec3( cos_ta, sin_ta, 0.0f );
      ix0 = newVertex( &vBuffer, r2 * cos_ta_1da, r2 * sin_ta_1da, gearinfo->width * 0.5f, normal );
      ix1 = newVertex( &vBuffer, r2 * cos_ta_1da, r2 * sin_ta_1da, -gearinfo->width * 0.5f, normal );
      ix2 = newVertex( &vBuffer, r2 * cos_ta_2da, r2 * sin_ta_2da, gearinfo->width * 0.5f, normal );
      ix3 = newVertex( &vBuffer, r2 * cos_ta_2da, r2 * sin_ta_2da, -gearinfo->width * 0.5f, normal );
      newFace( &iBuffer, ix0, ix1, ix2 );
      newFace( &iBuffer, ix1, ix3, ix2 );

      normal = glm::vec3( v2, -u2, 0.0f );
      ix0 = newVertex( &vBuffer, r2 * cos_ta_2da, r2 * sin_ta_2da, gearinfo->width * 0.5f, normal );
      ix1 = newVertex( &vBuffer, r2 * cos_ta_2da, r2 * sin_ta_2da, -gearinfo->width * 0.5f, normal );
      ix2 = newVertex( &vBuffer, r1 * cos_ta_3da, r1 * sin_ta_3da, gearinfo->width * 0.5f, normal );
      ix3 = newVertex( &vBuffer, r1 * cos_ta_3da, r1 * sin_ta_3da, -gearinfo->width * 0.5f, normal );
      newFace( &iBuffer, ix0, ix1, ix2 );
      newFace( &iBuffer, ix1, ix3, ix2 );

      normal = glm::vec3( cos_ta, sin_ta, 0.0f );
      ix0 = newVertex( &vBuffer, r1 * cos_ta_3da, r1 * sin_ta_3da, gearinfo->width * 0.5f, normal );
      ix1 = newVertex( &vBuffer, r1 * cos_ta_3da, r1 * sin_ta_3da, -gearinfo->width * 0.5f, normal );
      ix2 = newVertex( &vBuffer, r1 * cos_ta_4da, r1 * sin_ta_4da, gearinfo->width * 0.5f, normal );
      ix3 = newVertex( &vBuffer, r1 * cos_ta_4da, r1 * sin_ta_4da, -gearinfo->width * 0.5f, normal );
      newFace( &iBuffer, ix0, ix1, ix2 );
      newFace( &iBuffer, ix1, ix3, ix2 );

      // draw inside radius cylinder 
      ix0 = newVertex( &vBuffer, r0 * cos_ta, r0 * sin_ta, -gearinfo->width * 0.5f, glm::vec3( -cos_ta, -sin_ta, 0.0f ) );
      ix1 = newVertex( &vBuffer, r0 * cos_ta, r0 * sin_ta, gearinfo->width * 0.5f, glm::vec3( -cos_ta, -sin_ta, 0.0f ) );
      ix2 = newVertex( &vBuffer, r0 * cos_ta_4da, r0 * sin_ta_4da, -gearinfo->width * 0.5f, glm::vec3( -cos_ta_4da, -sin_ta_4da, 0.0f ) );
      ix3 = newVertex( &vBuffer, r0 * cos_ta_4da, r0 * sin_ta_4da, gearinfo->width * 0.5f, glm::vec3( -cos_ta_4da, -sin_ta_4da, 0.0f ) );
      newFace( &iBuffer, ix0, ix1, ix2 );
      newFace( &iBuffer, ix1, ix3, ix2 );
    }

    size_t vertexBufferSize = vBuffer.size( ) * sizeof( Vertex );
    size_t indexBufferSize = iBuffer.size( ) * sizeof( uint32_t );

    {
      auto cmd = cmdPool->allocateCommandBuffer( );
      cmd->begin( );

      auto stagingBuffer = _device->createBuffer( vertexBufferSize,
        vk::BufferUsageFlagBits::eTransferSrc,
        vk::MemoryPropertyFlagBits::eHostVisible |
        vk::MemoryPropertyFlagBits::eHostCoherent );
      stagingBuffer->writeData( 0, vertexBufferSize, vBuffer.data( ) );

      vertexBuffer = _device->createBuffer( vertexBufferSize,
        vk::BufferUsageFlagBits::eVertexBuffer |
        vk::BufferUsageFlagBits::eTransferDst,
        vk::MemoryPropertyFlagBits::eDeviceLocal );
      stagingBuffer->copy( cmd, vertexBuffer, 0, 0, vertexBufferSize );

      cmd->end( );
      queue->submitAndWait( cmd );
    }
    {
      auto cmd = cmdPool->allocateCommandBuffer( );
      cmd->begin( );

      auto stagingBuffer = _device->createBuffer( indexBufferSize,
        vk::BufferUsageFlagBits::eTransferSrc,
        vk::MemoryPropertyFlagBits::eHostVisible |
        vk::MemoryPropertyFlagBits::eHostCoherent );
      stagingBuffer->writeData( 0, indexBufferSize, iBuffer.data( ) );

      indexBuffer = _device->createBuffer( indexBufferSize,
        vk::BufferUsageFlagBits::eIndexBuffer |
        vk::BufferUsageFlagBits::eTransferDst,
        vk::MemoryPropertyFlagBits::eDeviceLocal );

      stagingBuffer->copy( cmd, indexBuffer, 0, 0, indexBufferSize );

      cmd->end( );
      queue->submitAndWait( cmd );
    }

    indexCount = iBuffer.size( );
  }
//protected:
  //glm::vec3 color;
  glm::vec3 pos;
  float rotSpeed;
  float rotOffset;
public:
  std::shared_ptr<Buffer> vertexBuffer, indexBuffer;
  uint32_t indexCount;
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

  std::shared_ptr<Pipeline> pipeline;
  std::shared_ptr<DescriptorSetLayout> descriptorSetLayout;
  std::vector<VulkanGear*> gears;

  std::shared_ptr<UniformBuffer> uboBuffer;

  struct
  {
    glm::mat4 proj;
    glm::mat4 view;
    glm::vec3 lightPos;
    glm::vec3 viewPos;
  } ubo;

  virtual void initResources( void ) override
  {
    auto device = _window->device( );
    uboBuffer = device->createUniformBuffer( sizeof( ubo ) );

    auto sc = _window->swapchainImageSize( );
    uint32_t width = sc.width, height = sc.height;

    float zoom = -16.0f;

    ubo.proj = glm::perspective( glm::radians( 60.0f ),
      ( float ) width / ( float ) height, 0.001f, 256.0f );

    ubo.lightPos = glm::vec3( 0.0f, 0.0f, 2.5f );
    ubo.viewPos = glm::vec3( 0.0f, 0.0f, -zoom );
    ubo.view = glm::lookAt(
      ubo.viewPos,
      glm::vec3( -1.0f, -1.5f, 0.0f ),
      glm::vec3( 0.0f, 1.0f, 0.0f )
    );
    uboBuffer->set( &ubo );

    // Gear definitions
    std::vector<float> innerRadiuses = { 1.0f, 0.5f, 1.3f };
    std::vector<float> outerRadiuses = { 4.0f, 2.0f, 2.0f };
    std::vector<float> widths = { 1.0f, 2.0f, 0.5f };
    std::vector<int32_t> toothCount = { 20, 10, 10 };
    std::vector<float> toothDepth = { 0.7f, 0.7f, 0.7f };
    std::vector<glm::vec3> colors = {
      glm::vec3( 1.0f, 0.0f, 0.0f ),
      glm::vec3( 0.0f, 1.0f, 0.2f ),
      glm::vec3( 0.0f, 0.0f, 1.0f )
    };
    std::vector<glm::vec3> positions = {
      glm::vec3( -3.0, 0.0, 0.0 ),
      glm::vec3( 3.1, 0.0, 0.0 ),
      glm::vec3( -3.1, -6.2, 0.0 )
    };
    std::vector<float> rotationSpeeds = { 1.0f, -2.0f, -2.0f };
    std::vector<float> rotationOffsets = { 0.0f, -9.0f, -30.0f };

    gears.resize( positions.size( ) );
    for ( size_t i = 0; i < gears.size( ); ++i )
    {
      GearInfo gearInfo = { };
      gearInfo.innerRadius = innerRadiuses[ i ];
      gearInfo.outerRadius = outerRadiuses[ i ];
      gearInfo.width = widths[ i ];
      gearInfo.numTeeth = toothCount[ i ];
      gearInfo.toothDepth = toothDepth[ i ];
      gearInfo.color = colors[ i ];
      gearInfo.pos = positions[ i ];
      gearInfo.rotSpeed = rotationSpeeds[ i ];
      gearInfo.rotOffset = rotationOffsets[ i ];

      gears[ i ] = new VulkanGear( _window->device( ) );
      gears[ i ]->generate( &gearInfo, _window->gfxCommandPool( ),
        _window->gfxQueue( ) );
    }

     descriptorPool = device->createDescriptorPool( 1, {
      vk::DescriptorPoolSize( vk::DescriptorType::eUniformBuffer, 1 )
    } );

    descriptorSetLayout = device->createDescriptorSetLayout( {
      DescriptorSetLayoutBinding( 0, vk::DescriptorType::eUniformBuffer,
        vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment
      )
    } );

    auto vertexStage = device->createShaderPipelineShaderStage(
      POMPEII_EXAMPLES_SPV_ROUTE + std::string( "gears_vert.spv" ),
      vk::ShaderStageFlagBits::eVertex
    );
    auto fragmentStage = device->createShaderPipelineShaderStage(
      POMPEII_EXAMPLES_SPV_ROUTE + std::string( "gears_frag.spv" ),
      vk::ShaderStageFlagBits::eFragment
    );

    descriptorSet = device->allocateDescriptorSet( descriptorPool, descriptorSetLayout );

    pipelineLayout = device->createPipelineLayout( descriptorSetLayout, {
      vk::PushConstantRange(
        vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment, 
        0, sizeof( PushCtesVS )
      )
    } );

    vk::VertexInputBindingDescription binding( 0, sizeof( Vertex ),
      vk::VertexInputRate::eVertex );

    PipelineVertexInputStateCreateInfo vertexInput( binding, {
      vk::VertexInputAttributeDescription( 0, 0,
        vk::Format::eR32G32B32Sfloat, offsetof( Vertex, pos )
      ), 
      vk::VertexInputAttributeDescription( 1, 0,
        vk::Format::eR32G32B32Sfloat, offsetof( Vertex, normal )
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

    std::vector<WriteDescriptorSet> wdss =
    {
      WriteDescriptorSet(
        descriptorSet, 0, 0, vk::DescriptorType::eUniformBuffer,
        1, nullptr, DescriptorBufferInfo( uboBuffer, 0, sizeof( ubo ) )
      ),
    };
    device->updateDescriptorSets( wdss, { } );
  }
  
  std::shared_ptr<DescriptorSet> descriptorSet;
  std::shared_ptr<DescriptorPool> descriptorPool;

  virtual void nextFrame( void )
  {
    auto cmd = _window->currentCommandBuffer( );

    static auto startTime = std::chrono::high_resolution_clock::now( );

    auto currentTime = std::chrono::high_resolution_clock::now( );
    float time = std::chrono::duration_cast< std::chrono::milliseconds >(
      currentTime - startTime ).count( ) / 1000.0f;

    std::array<vk::ClearValue, 2 > clearValues;
    clearValues[ 0 ] = pompeii::utils::getClearValueColor( 0.0f, 0.0f, 0.0f );
    clearValues[ 1 ] = pompeii::utils::getClearValueDepth( );

    vk::Extent2D extent = _window->swapchainImageSize( );

    cmd->beginRenderPass( _window->renderPass( ),
      _window->framebuffer( ),
      vk::Rect2D( { 0, 0 }, extent ), clearValues,
      vk::SubpassContents::eInline );

    cmd->setViewportScissors( extent );

    cmd->bindGraphicsPipeline( pipeline );
    cmd->bindDescriptorSets( vk::PipelineBindPoint::eGraphics,
      pipelineLayout, 0, { descriptorSet }, { } );

    for ( auto& gear : gears )
    {
      gear->draw( cmd, time * 5.f );
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