/**
* Copyright (c) 2017 - 2018, Lava
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

#include <lava/lava.h>
#include <lavaRenderer/lavaRenderer.h>
using namespace lava;

#include <routes.h>
#include "utils/Camera.h"

const unsigned int SCR_WIDTH = 500;
const unsigned int SCR_HEIGHT = 500;

#define PI 3.14159

class CustomRenderer : public VulkanWindowRenderer
{
public:
  CustomRenderer( lava::VulkanWindow *w )
    : VulkanWindowRenderer( )
    , _window( w )
  {
    _window->setWindowTitle( "ClipPlanes" );
    camera = Camera( glm::vec3( 0.0f, 1.0f, 25.5f ) );
  }

  // camera
  Camera camera;
  // timing
  float deltaTime = 0.0f; // time between current frame and last frame
  float lastFrame = 0.0f;

  float lastX = SCR_WIDTH * 0.5f;
  float lastY = SCR_HEIGHT * 0.5f;
  bool firstMouse = true;

  struct
  {
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
    glm::vec4 clipPlane;
  } ubo;

  struct Vertex
  {
    glm::vec3 pos;
  };

  std::vector<Vertex> vertices;
  std::vector<uint16_t> indices;

  void CreateTorus( float major, float minor,
    uint32_t slices, uint32_t stacks )
  {
    //uint32_t vertexCount = slices * stacks * 3;
    //vertices.reserve( vertexCount );

    //glm::vec3* position = &vertices;
    for ( uint32_t slice = 0; slice < slices; ++slice )
    {
      float theta = slice * 2.0f * PI / slices;
      for ( uint32_t stack = 0; stack < stacks; ++stack )
      {
        float phi = stack * 2.0f * PI / stacks;
        float beta = major + minor * cos( phi );
        vertices.push_back( Vertex{
          glm::vec3(
            cos( theta ) * beta,
            sin( theta ) * beta,
            sin( phi ) * minor
          )
        } );
      }
    }

    //uint32_t indexCount = slices * stacks * 6;
    //indices.reserve( indexCount );

    //uint32_t* index = &indices[0];
    uint32_t v = 0;
    for ( uint32_t i = 0; i < slices - 1; ++i )
    {
      for ( uint32_t j = 0; j < stacks; ++j )
      {
        uint32_t next = ( j + 1 ) % stacks;

        indices.push_back( v + next + stacks );
        indices.push_back( v + next );
        indices.push_back( v + j );

        indices.push_back( v + j );
        indices.push_back( v + j + stacks );
        indices.push_back( v + next + stacks );
      }
      v += stacks;
    }
    for ( uint32_t j = 0; j < stacks; ++j )
    {
      uint32_t next = ( j + 1 ) % stacks;
      indices.push_back( next );
      indices.push_back( v + next );
      indices.push_back( v + j );

      indices.push_back( v + j );
      indices.push_back( j );
      indices.push_back( next );
    }
  }

  void initResources( void ) override
  {
    auto device = _window->device( );

    CreateTorus( 8.0f, 2.0f, 40, 10 );

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

    // MVP buffer
    uboUniform = device->createUniformBuffer( sizeof( ubo ) );
    ubo.clipPlane = glm::vec4( 0.0f, 1.0f, 0.0f, 0.0f );

    std::array<vk::DescriptorPoolSize, 1> poolSize =
    {
      vk::DescriptorPoolSize( vk::DescriptorType::eUniformBuffer, 1 )
    };

    auto descriptorPool = device->createDescriptorPool( 1, poolSize );

    // Init descriptor and pipeline layouts
    std::vector<DescriptorSetLayoutBinding> dslbs =
    {
      DescriptorSetLayoutBinding( 0, vk::DescriptorType::eUniformBuffer,
      vk::ShaderStageFlagBits::eVertex |
      vk::ShaderStageFlagBits::eGeometry
      )
    };
    auto descriptorSetLayout = device->createDescriptorSetLayout( dslbs );

    pipelineLayout = device->createPipelineLayout( descriptorSetLayout );

    // init pipeline
    auto vertexStage = device->createShaderPipelineShaderStage(
      LAVA_EXAMPLES_SPV_ROUTE + std::string( "clipPlane_vert.spv" ),
      vk::ShaderStageFlagBits::eVertex
    );
    auto geomStage = device->createShaderPipelineShaderStage(
      LAVA_EXAMPLES_SPV_ROUTE + std::string( "clipPlane_geom.spv" ),
      vk::ShaderStageFlagBits::eGeometry
    );
    auto fragmentStage = device->createShaderPipelineShaderStage(
      LAVA_EXAMPLES_SPV_ROUTE + std::string( "clipPlane_frag.spv" ),
      vk::ShaderStageFlagBits::eFragment
    );

    vk::VertexInputBindingDescription binding(
      0, sizeof( Vertex ), vk::VertexInputRate::eVertex );

    PipelineVertexInputStateCreateInfo vertexInput( binding, {
      vk::VertexInputAttributeDescription( 0, 0, vk::Format::eR32G32B32Sfloat,
      offsetof( Vertex, pos )
      ) }
    );
    vk::PipelineInputAssemblyStateCreateInfo assembly( {},
      vk::PrimitiveTopology::eTriangleList, VK_FALSE );
    PipelineViewportStateCreateInfo viewport( 1, 1 );
    vk::PipelineRasterizationStateCreateInfo rasterization( {}, true,
      false, vk::PolygonMode::eFill, vk::CullModeFlagBits::eNone,
      vk::FrontFace::eCounterClockwise, false, 0.0f, 0.0f, 0.0f, 1.0f );
    PipelineMultisampleStateCreateInfo multisample( vk::SampleCountFlagBits::e1,
      false, 0.0f, nullptr, false, false );
    vk::StencilOpState stencilOpState( vk::StencilOp::eKeep,
      vk::StencilOp::eKeep, vk::StencilOp::eKeep, vk::CompareOp::eAlways,
      0, 0, 0 );
    vk::PipelineDepthStencilStateCreateInfo depthStencil( {}, true, true,
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

    pipelines.solid = device->createGraphicsPipeline(
      _window->pipelineCache, {}, { vertexStage, geomStage, fragmentStage },
      vertexInput, assembly, nullptr, viewport, rasterization, multisample,
      depthStencil, colorBlend, dynamic, pipelineLayout,
      _window->defaultRenderPass( ) );

    // Wireframe rendering pipeline
    if ( _window->physicalDevice( )->getDeviceFeatures( ).fillModeNonSolid )
    {
      rasterization.polygonMode = vk::PolygonMode::eLine;
      rasterization.lineWidth = 1.0f;

      rasterization.cullMode = vk::CullModeFlagBits::eNone;

      pipelines.wireframe = device->createGraphicsPipeline(
        _window->pipelineCache, {}, { vertexStage, geomStage, fragmentStage },
        vertexInput, assembly, nullptr, viewport, rasterization, multisample,
        depthStencil, colorBlend, dynamic, pipelineLayout,
        _window->defaultRenderPass( ) );
    }

    // Init descriptor set
    descriptorSet = device->allocateDescriptorSet( descriptorPool,
      descriptorSetLayout );
    std::vector<WriteDescriptorSet> wdss =
    {
      WriteDescriptorSet( descriptorSet, 0, 0,
      vk::DescriptorType::eUniformBuffer, 1, nullptr,
      DescriptorBufferInfo( uboUniform, 0, sizeof( ubo ) )
      )
    };
    device->updateDescriptorSets( wdss, {} );
  }

  void updateMVP( void )
  {
    auto size = _window->getExtent( );

    uint32_t width = size.width, height = size.height;

    static auto startTime = std::chrono::high_resolution_clock::now( );

    auto currentTime = std::chrono::high_resolution_clock::now( );
    float time = std::chrono::duration_cast<std::chrono::milliseconds>(
      currentTime - startTime ).count( ) / 1000.0f;

    float currentFrame = time;
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    ubo.model = glm::mat4( 1.0f );
    ubo.model = glm::scale( ubo.model, glm::vec3( 0.05f ) );
    ubo.model = glm::rotate( glm::mat4( 1.0f ), time * 0.25f * glm::radians( 90.0f ), glm::vec3( 0.0f, 0.0f, 1.0f ) );
    ubo.view = camera.GetViewMatrix( );
    ubo.proj = glm::perspective( glm::radians( camera.Zoom ), ( float ) width / ( float ) height, 0.1f, 100.0f );
    ubo.proj[ 1 ][ 1 ] *= -1;

    ubo.clipPlane.z = 7.0f * sin( time );

    uboUniform->set( &ubo );
  }

  bool modeReflect = true;

  void nextFrame( void ) override
  {
    if ( Input::isKeyPressed( lava::Keyboard::Key::Esc ) )
    {
      _window->_window->close( );
    }

    if ( Input::isKeyPressed( lava::Keyboard::Key::Z ) )
    {
      enable_wire = false;
    }
    else if ( Input::isKeyPressed( lava::Keyboard::Key::X ) )
    {
      enable_wire = true;
    }


    const auto size = _window->swapChainImageSize( );

    // Mouse event
    {
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
      else if ( Input::MouseButtonRelease( MouseButton::Left ) )
      {
        firstMouse = true;
      }
    }

    updateMVP( );

    std::array<vk::ClearValue, 2 > clearValues;
    std::array<float, 4> ccv = { 0.2f, 0.3f, 0.3f, 1.0f };
    clearValues[ 0 ].color = vk::ClearColorValue( ccv );
    clearValues[ 1 ].depthStencil = vk::ClearDepthStencilValue( 1.0f, 0 );

    auto cmd = _window->currentCommandBuffer( );
    vk::Rect2D rect;
    rect.extent.width = size.x;
    rect.extent.height = size.y;
    cmd->beginRenderPass(
      _window->defaultRenderPass( ),
      _window->currentFramebuffer( ),
      rect, clearValues, vk::SubpassContents::eInline
    );

    cmd->bindGraphicsPipeline( enable_wire ? pipelines.wireframe :
      pipelines.solid );
    cmd->bindDescriptorSets( vk::PipelineBindPoint::eGraphics,
      pipelineLayout, 0, { descriptorSet }, nullptr );
    cmd->bindVertexBuffer( 0, vertexBuffer, 0 );
    cmd->bindIndexBuffer( indexBuffer, 0, vk::IndexType::eUint16 );
    cmd->setViewportScissors( _window->getExtent( ) );

    cmd->drawIndexed( indices.size( ), 1, 0, 0, 1 );

    cmd->endRenderPass( );

    _window->requestUpdate( );
  }

private:
  VulkanWindow *_window;

  bool enable_wire = false;

  std::shared_ptr<Buffer> vertexBuffer;
  std::shared_ptr<Buffer> indexBuffer;

  struct Pipelines
  {
    std::shared_ptr<Pipeline> solid;
    std::shared_ptr<Pipeline> wireframe;
  } pipelines;

  std::shared_ptr<PipelineLayout> pipelineLayout;
  std::shared_ptr<DescriptorSet> descriptorSet;
  std::shared_ptr<Buffer> uboUniform;
};

class CustomVkWindow : public VulkanWindow
{
public:
  VulkanWindowRenderer* createRenderer( void ) override
  {
    return new CustomRenderer( this );
  }
};

int main( void )
{
  std::shared_ptr<Instance> instance;

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
#ifndef NDEBUG
    "VK_LAYER_LUNARG_standard_validation",
#endif
  };
  std::vector<const char*> extensions =
  {
    VK_KHR_SURFACE_EXTENSION_NAME,  // Surface extension
    LAVA_KHR_EXT, // OS specific surface extension
    VK_EXT_DEBUG_REPORT_EXTENSION_NAME
  };


  instance = Instance::create( vk::InstanceCreateInfo(
    { },
    &appInfo,
    layers.size( ),
    layers.data( ),
    extensions.size( ),
    extensions.data( )
  ) );

  CustomVkWindow w;
  w.setVulkanInstance( instance );
  w.resize( SCR_WIDTH, SCR_HEIGHT );

  w.show( );

  return 0;
}