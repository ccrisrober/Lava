/**
 * Copyright (c) 2017, Lava
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
using namespace lava;

#include <routes.h>

#include "utils/Camera.h"

const unsigned int SCR_WIDTH = 500;
const unsigned int SCR_HEIGHT = 500;

class CustomRenderer : public VulkanWindowRenderer
{
public:
  CustomRenderer( lava::VulkanWindow *w )
    : VulkanWindowRenderer( )
    , _window( w )
  {
    _window->setWindowTitle( "Heightmap Tesselation" );
    camera = Camera( glm::vec3( 0.0f, 2.0f, 3.5f ), 
      glm::vec3( 0.0f, 1.0f, 0.0f), YAW, -25.0f );
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
    float amount = 0.5f;
    float tess_level = 5.0f;
  } ubo;

  std::vector<glm::vec3> positions;
  std::vector<glm::vec2> texCoords;
  std::vector<uint32_t> indices;

  void generatePlane( float width = 1.0f, float height = 1.0f,
    unsigned int gridX = 1,
    unsigned int gridY = 1 )
  {
    float width_half = width / 2.0f;
    float height_half = height / 2.0f;

    unsigned int gridX1 = gridX + 1;
    unsigned int gridY1 = gridY + 1;

    float segment_width = width / gridX;
    float segment_height = height / gridY;

    unsigned int ix, iy;

    for ( iy = 0; iy < gridY1; ++iy )
    {
      float y = iy * segment_height - height_half;
      for ( ix = 0; ix < gridX1; ++ix )
      {
        float x = ix * segment_width - width_half;

        positions.push_back( glm::vec3( x, 0.0f, -y ) ),
        texCoords.push_back( glm::vec2( 
            ( ( float ) ix ) / gridX, 
            1.0f - ( ( ( float ) iy ) / gridY )
          ) 
        );
      }
    }

    const int TOTAL_INDICES = gridX * gridY * 2 * 3;
    indices = std::vector<uint32_t>( TOTAL_INDICES, 0 );
    uint32_t* id = &indices[ 0 ];
    for ( uint32_t i = 0; i < gridY; ++i )
    {
      for ( uint32_t j = 0; j < gridX; ++j )
      {
        uint32_t i0 = i * ( gridX + 1 ) + j;
        uint32_t i1 = i0 + 1;
        uint32_t i2 = i0 + ( gridX + 1 );
        uint32_t i3 = i2 + 1;
        if ( ( j + i ) % 2 )
        {
          *id++ = i0; *id++ = i2; *id++ = i1;
          *id++ = i1; *id++ = i2; *id++ = i3;
        }
        else
        {
          *id++ = i0; *id++ = i2; *id++ = i3;
          *id++ = i0; *id++ = i3; *id++ = i1;
        }
      }
    }
  }

  void initResources( void ) override
  {
    auto device = _window->device( );

    generatePlane( 2.5f, 2.5f, 5, 5 );

    // Vertex positions buffer
    {
      uint32_t vertexBufferPositionsSize = positions.size( ) * sizeof( glm::vec3 );
      auto stagingBuffer = device->createBuffer( vertexBufferPositionsSize,
        vk::BufferUsageFlagBits::eTransferSrc,
        vk::MemoryPropertyFlagBits::eHostVisible | 
        vk::MemoryPropertyFlagBits::eHostCoherent );
      stagingBuffer->writeData( 0, vertexBufferPositionsSize, positions.data( ) );

      vertexBufferPositions = device->createBuffer( vertexBufferPositionsSize,
        vk::BufferUsageFlagBits::eVertexBuffer | 
        vk::BufferUsageFlagBits::eTransferDst,
        vk::MemoryPropertyFlagBits::eDeviceLocal );

      auto cmd = _window->graphicsCommandPool( )->allocateCommandBuffer( );
      cmd->beginSimple( );
        stagingBuffer->copy( cmd, vertexBufferPositions, 0, 0, vertexBufferPositionsSize );
      cmd->end( );

      _window->graphicQueue( )->submitAndWait( cmd );

    }

    // Vertex texCoords buffer
    {
      uint32_t vertexBufferTexCoordsSize = texCoords.size( ) * sizeof( glm::vec2 );
      auto stagingBuffer = device->createBuffer( vertexBufferTexCoordsSize,
        vk::BufferUsageFlagBits::eTransferSrc,
        vk::MemoryPropertyFlagBits::eHostVisible | 
        vk::MemoryPropertyFlagBits::eHostCoherent );
      stagingBuffer->writeData( 0, vertexBufferTexCoordsSize, texCoords.data( ) );

      vertexBufferTexCoords = device->createBuffer( vertexBufferTexCoordsSize,
        vk::BufferUsageFlagBits::eVertexBuffer | 
        vk::BufferUsageFlagBits::eTransferDst,
        vk::MemoryPropertyFlagBits::eDeviceLocal );

      auto cmd = _window->graphicsCommandPool( )->allocateCommandBuffer( );
      cmd->beginSimple( );
        stagingBuffer->copy( cmd, vertexBufferTexCoords, 0, 0, vertexBufferTexCoordsSize );
      cmd->end( );

      _window->graphicQueue( )->submitAndWait( cmd );

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

      auto cmd = _window->graphicsCommandPool( )->allocateCommandBuffer( );
      cmd->beginSimple( );
        stagingBuffer->copy( cmd, indexBuffer, 0, 0, indexBufferSize );
      cmd->end( );

      _window->graphicQueue( )->submitAndWait( cmd );
    }

    mvpBuffer = device->createUniformBuffer( sizeof( ubo ) );


    texHeightmap = device->createTexture2D( LAVA_EXAMPLES_IMAGES_ROUTE +
      std::string( "DisplacementMapEarth.png" ), _window->graphicsCommandPool( ),
      _window->graphicQueue( ), vk::Format::eR8G8B8A8Unorm );

    texAlbedo = device->createTexture2D( LAVA_EXAMPLES_IMAGES_ROUTE +
      std::string( "earth_diffuse.jpg" ), _window->graphicsCommandPool( ),
      _window->graphicQueue( ), vk::Format::eR8G8B8A8Unorm );


    // Init descriptor and pipeline layouts
    std::vector<DescriptorSetLayoutBinding> dslbs = 
    {
      DescriptorSetLayoutBinding( 0, vk::DescriptorType::eUniformBuffer,
        vk::ShaderStageFlagBits::eVertex | 
        vk::ShaderStageFlagBits::eTessellationEvaluation | 
          vk::ShaderStageFlagBits::eTessellationControl
      ),
      DescriptorSetLayoutBinding( 1, vk::DescriptorType::eCombinedImageSampler,
        vk::ShaderStageFlagBits::eTessellationEvaluation
      ),
      DescriptorSetLayoutBinding( 2, vk::DescriptorType::eCombinedImageSampler,
        vk::ShaderStageFlagBits::eFragment
      )
    };
    auto descriptorSetLayout = device->createDescriptorSetLayout( dslbs );

    pipelineLayout = device->createPipelineLayout( descriptorSetLayout, nullptr );

    auto vertexStage = device->createShaderPipelineShaderStage( 
      LAVA_EXAMPLES_SPV_ROUTE + std::string( "terrain_tess_vert.spv" ), 
        vk::ShaderStageFlagBits::eVertex
      );
    auto ctrlStage = device->createShaderPipelineShaderStage( 
      LAVA_EXAMPLES_SPV_ROUTE + std::string( "terrain_tess_tesc.spv" ), 
        vk::ShaderStageFlagBits::eTessellationControl
      );
    auto evalStage = device->createShaderPipelineShaderStage( 
      LAVA_EXAMPLES_SPV_ROUTE + std::string( "terrain_tess_tese.spv" ), 
        vk::ShaderStageFlagBits::eTessellationEvaluation
      );
    auto fragmentStage = device->createShaderPipelineShaderStage( 
      LAVA_EXAMPLES_SPV_ROUTE + std::string( "terrain_tess_frag.spv" ), 
        vk::ShaderStageFlagBits::eFragment
      );

    PipelineVertexInputStateCreateInfo vertexInput( {
        vk::VertexInputBindingDescription( 0, sizeof( glm::vec3 ), 
          vk::VertexInputRate::eVertex ),
        vk::VertexInputBindingDescription( 1, sizeof( glm::vec2 ), 
          vk::VertexInputRate::eVertex )
      }, {
        vk::VertexInputAttributeDescription( 0, 0, 
          vk::Format::eR32G32B32Sfloat, 0 ),
        vk::VertexInputAttributeDescription( 1, 1, 
          vk::Format::eR32G32Sfloat, 0 )
      } );
    vk::PipelineInputAssemblyStateCreateInfo assembly( { }, 
      vk::PrimitiveTopology::ePatchList, VK_FALSE );
    PipelineViewportStateCreateInfo viewport( 1, 1 );
    vk::PipelineRasterizationStateCreateInfo rasterization( { }, true,
      false, vk::PolygonMode::eFill, vk::CullModeFlagBits::eNone,
      vk::FrontFace::eCounterClockwise, false, 0.0f, 0.0f, 0.0f, 1.0f );
    PipelineMultisampleStateCreateInfo multisample( vk::SampleCountFlagBits::e1, false, 0.0f, nullptr, false, false );
    vk::StencilOpState stencilOpState( vk::StencilOp::eKeep, vk::StencilOp::eKeep, vk::StencilOp::eKeep, vk::CompareOp::eAlways, 0, 0, 0 );
    vk::PipelineDepthStencilStateCreateInfo depthStencil( { }, true, true, vk::CompareOp::eLessOrEqual, false, false, stencilOpState, stencilOpState, 0.0f, 0.0f );
    vk::PipelineColorBlendAttachmentState colorBlendAttachment( false, vk::BlendFactor::eZero, vk::BlendFactor::eZero, vk::BlendOp::eAdd, vk::BlendFactor::eZero, vk::BlendFactor::eZero, vk::BlendOp::eAdd,
      vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA );
    PipelineColorBlendStateCreateInfo colorBlend( false, vk::LogicOp::eNoOp, colorBlendAttachment, { 1.0f, 1.0f, 1.0f, 1.0f } );
    PipelineDynamicStateCreateInfo dynamic( { vk::DynamicState::eViewport, vk::DynamicState::eScissor } );

    vk::PipelineTessellationStateCreateInfo tessState( { }, 3 );

    pipelines.solid = device->createGraphicsPipeline( _window->pipelineCache,
      // Specify that we will be creating a derivative of this pipeline.
      vk::PipelineCreateFlagBits::eAllowDerivatives,
      { vertexStage, fragmentStage, ctrlStage, evalStage },
      vertexInput, assembly, tessState, viewport, rasterization, multisample,
      depthStencil, colorBlend, dynamic, pipelineLayout, _window->defaultRenderPass( ) );

    // Wireframe rendering pipeline
    if ( _window->physicalDevice( )->getDeviceFeatures( ).fillModeNonSolid )
    {
      rasterization.polygonMode = vk::PolygonMode::eLine;
      rasterization.lineWidth = 1.0f;

      rasterization.cullMode = vk::CullModeFlagBits::eNone;

      pipelines.wireframe = device->createGraphicsPipeline( _window->pipelineCache,
        // Modify pipeline info to reflect derivation
        vk::PipelineCreateFlagBits::eDerivative,
        { vertexStage, fragmentStage, ctrlStage, evalStage },
        vertexInput, assembly, tessState, viewport, rasterization, multisample,
        depthStencil, colorBlend, dynamic, pipelineLayout, 
        _window->defaultRenderPass( ), 0, pipelines.solid, -1 );
    }

    std::array<vk::DescriptorPoolSize, 2> poolSize =
    {
      vk::DescriptorPoolSize( vk::DescriptorType::eUniformBuffer, 1 ),
      vk::DescriptorPoolSize( vk::DescriptorType::eCombinedImageSampler, 2 )
    };
    auto descriptorPool = device->createDescriptorPool( 1, poolSize );

    // Init descriptor set
    descriptorSet = device->allocateDescriptorSet( descriptorPool, 
      descriptorSetLayout );
    std::vector<WriteDescriptorSet> wdss =
    {
      WriteDescriptorSet( descriptorSet, 0, 0, 
        vk::DescriptorType::eUniformBuffer, 1, nullptr,
        DescriptorBufferInfo( mvpBuffer, 0, sizeof( ubo ) )
      ),
      WriteDescriptorSet( descriptorSet, 1, 0,
        vk::DescriptorType::eCombinedImageSampler, 1,
        texHeightmap->descriptor, nullptr
      ),
      WriteDescriptorSet( descriptorSet, 2, 0,
        vk::DescriptorType::eCombinedImageSampler, 1,
        texAlbedo->descriptor, nullptr
      )
    };
    device->updateDescriptorSets( wdss, { } );
  }

  void updateUniformBuffers( void )
  {
    auto size = _window->getExtent( );

    uint32_t width = size.width, height = size.height;

    static auto startTime = std::chrono::high_resolution_clock::now( );

    auto currentTime = std::chrono::high_resolution_clock::now( );
    float time = std::chrono::duration_cast<std::chrono::milliseconds>( currentTime - startTime ).count( ) / 1000.0f;

    float currentFrame = time;
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    ubo.model = glm::mat4( 1.0f );
    ubo.view = camera.GetViewMatrix( );
    ubo.proj = glm::perspective( 
      glm::radians( camera.Zoom ), ( float ) width / ( float ) height, 
      0.1f, 100.0f
    );
    ubo.proj[ 1 ][ 1 ] *= -1;

    mvpBuffer->update( &ubo );
  }

  void nextFrame( void ) override
  {
    if ( Input::isKeyPressed( lava::Keyboard::Key::Esc ) )
    {
      _window->_window->close( );
    }

    if ( Input::isKeyPressed( lava::Keyboard::Key::E ) )
    {
      enable_wire = true;
    }
    else if ( Input::isKeyPressed( lava::Keyboard::Key::R ) )
    {
      enable_wire = false;
    }
    else if ( Input::isKeyPressed( lava::Keyboard::Key::V ) )
    {
      ubo.amount += 0.01f;
    }
    else if ( Input::isKeyPressed( lava::Keyboard::Key::C ) )
    {
      ubo.amount -= 0.01f;
    }
    else if ( Input::isKeyPressed( lava::Keyboard::Key::X ) )
    {
      ubo.tess_level += 0.01f;
    }
    else if ( Input::isKeyPressed( lava::Keyboard::Key::Z ) )
    {
      ubo.tess_level -= 0.01f;
      if ( ubo.tess_level < 1.0f )
      {
        ubo.tess_level = 1.0f;
      }
    }
    
    updateUniformBuffers( );

    std::array<vk::ClearValue, 2 > clearValues;
    std::array<float, 4> ccv = { 0.0f, 0.0f, 0.0f, 1.0f };
    clearValues[ 0 ].color = vk::ClearColorValue( ccv );
    clearValues[ 1 ].depthStencil = vk::ClearDepthStencilValue( 1.0f, 0 );

    const glm::ivec2 size = _window->swapChainImageSize( );
    auto cmd = _window->currentCommandBuffer( );
    vk::Rect2D rect;
    rect.extent.width = size.x;
    rect.extent.height = size.y;
    cmd->beginRenderPass(
      _window->defaultRenderPass( ),
      _window->currentFramebuffer( ),
      rect, clearValues, vk::SubpassContents::eInline
    );

    if ( enable_wire )
    {
      cmd->bindGraphicsPipeline( pipelines.wireframe );
    }
    else
    {
      cmd->bindGraphicsPipeline( pipelines.solid );
    }
    cmd->bindDescriptorSets( vk::PipelineBindPoint::eGraphics,
      pipelineLayout, 0, { descriptorSet }, nullptr );

    // Binding point 0 : Mesh vertex buffer
    cmd->bindVertexBuffer( 0, vertexBufferPositions, 0 );
    // Binding point 1 : Instance data buffer
    cmd->bindVertexBuffer( 1, vertexBufferTexCoords, 0 );
    cmd->bindIndexBuffer( indexBuffer, 0, vk::IndexType::eUint32 );

    cmd->setViewportScissors( _window->getExtent( ) );
    cmd->drawIndexed( indices.size( ), 1, 0, 0, 1 );

    cmd->endRenderPass( );

    _window->frameReady( );
  }
private:
  VulkanWindow *_window;
  std::shared_ptr<Buffer> vertexBufferPositions;
  std::shared_ptr<Buffer> vertexBufferTexCoords;
  std::shared_ptr<Buffer> indexBuffer;
  std::shared_ptr< UniformBuffer > mvpBuffer;
  std::shared_ptr<Texture2D> texAlbedo;
  std::shared_ptr<Texture2D> texHeightmap;

  bool enable_wire = true;

  struct
  {
    std::shared_ptr<Pipeline> solid;
    std::shared_ptr<Pipeline> wireframe;
  } pipelines;

  std::shared_ptr<PipelineLayout> pipelineLayout;
  std::shared_ptr<DescriptorSet> descriptorSet;
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