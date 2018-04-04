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

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <routes.h>
#include <random>

#include <lavaEngine/lavaEngine.h>

class CustomRenderer : public VulkanWindowRenderer
{
public:
  CustomRenderer( lava::VulkanWindow *w )
    : VulkanWindowRenderer( )
    , _window( w )
  {
    _window->setWindowTitle( "Cube textured" );
  }

  struct
  {
    glm::mat4 view;
    glm::mat4 proj;
  } uboVS;

  struct PushConstant
  {
    glm::mat4 model;
  } pushConstant;

  struct Vertex
  {
    glm::vec3 pos;
    glm::vec2 texCoord;
  };

  const float side = 1.0f;
  const float side2 = side * 0.5f;

  const std::vector<Vertex> vertices =
  {
    { { -side2, -side2,  side2 },{ 0.0f, 0.0f } },
    { { side2, -side2,  side2 },{ 1.0f, 0.0f } },
    { { -side2,  side2,  side2 },{ 0.0f, 1.0f } },
    { { side2,  side2,  side2 },{ 1.0f, 1.0f } },

    { { -side2, -side2, -side2 },{ 0.0f, 0.0f } },
    { { side2, -side2, -side2 },{ 1.0f, 0.0f } },
    { { -side2,  side2, -side2 },{ 0.0f, 1.0f } },
    { { side2,  side2, -side2 },{ 1.0f, 1.0f } },

    { { side2, -side2, -side2 },{ 0.0f, 0.0f } },
    { { side2, -side2,  side2 },{ 1.0f, 0.0f } },
    { { side2,  side2, -side2 },{ 0.0f, 1.0f } },
    { { side2,  side2,  side2 },{ 1.0f, 1.0f } },

    { { -side2, -side2, -side2 },{ 0.0f, 0.0f } },
    { { -side2, -side2,  side2 },{ 1.0f, 0.0f } },
    { { -side2,  side2, -side2 },{ 0.0f, 1.0f } },
    { { -side2,  side2,  side2 },{ 1.0f, 1.0f } },

    { { -side2,  side2, -side2 },{ 0.0f, 0.0f } },
    { { -side2,  side2,  side2 },{ 1.0f, 0.0f } },
    { { side2,  side2, -side2 },{ 0.0f, 1.0f } },
    { { side2,  side2,  side2 },{ 1.0f, 1.0f } },

    { { -side2, -side2, -side2 },{ 0.0f, 0.0f } },
    { { -side2, -side2,  side2 },{ 1.0f, 0.0f } },
    { { side2, -side2, -side2 },{ 0.0f, 1.0f } },
    { { side2, -side2,  side2 },{ 1.0f, 1.0f } }
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

  lava::engine::Geometry* generateGeom( const lava::engine::Color& c )
  {
    auto geom = new lava::engine::Geometry( );
    geom->addPrimitive( std::make_shared<lava::engine::Primitive>( ) );

    return geom;
  }

  // Returns random values uniformly distributed in the range [a, b]
  float _random( void )
  {
    return static_cast <float> ( rand( ) ) / static_cast <float> ( RAND_MAX );
  }

  lava::engine::Group* addCube( void )
  {
    float cubeSize = std::ceil( _random( ) * 3.0f );
    auto cubeGeometry = generateGeom( glm::vec3( 1.0f, 0.0f, 0.0f ) );
    cubeGeometry->scale( glm::vec3( cubeSize, cubeSize, cubeSize ) );
    auto cube = new lava::engine::Group( "cube" );
    cube->addChild( cubeGeometry );

    glm::vec3 pos = cube->getAbsolutePosition( );
    pos.x = -30.0f + std::round( _random( ) * 100.0f );
    pos.y = std::round( _random( ) * 5 );
    pos.z = -20.0f + std::round( _random( ) * 100.0f );

    cube->setPosition( pos );

    return cube;
  }
  lava::engine::Camera* camera;
  lava::engine::Group* createScene( void )
  {
    auto scene = new lava::engine::Group( "scene" );

    camera = new lava::engine::Camera( 75.0f, 500 / 500, 0.03f, 1000.0f );
    camera->translate( glm::vec3( 0.0f, 10.0f, 50.0f ) );

    //camera->addComponent( new mb::FreeCameraComponent( ) );
    scene->addChild( camera );

    for ( int i = 0; i < 185; ++i )
    {
      scene->addChild( addCube( ) );
    }

    return scene;
  }

  lava::engine::Group* scene;

  void initResources( void ) override
  {
    auto device = _window->device( );

    scene = createScene( );
    scene->getTransform( );

    // Vertex buffer
    {
      uint32_t vertexBufferSize = vertices.size( ) * sizeof( Vertex );
      auto cmd = _window->gfxCommandPool( )->allocateCommandBuffer( );
      cmd->begin( );

      vertexBuffer = device->createBuffer( vertexBufferSize,
        vk::BufferUsageFlagBits::eVertexBuffer |
        vk::BufferUsageFlagBits::eTransferDst,
        vk::MemoryPropertyFlagBits::eDeviceLocal );
      vertexBuffer->update<Vertex>( cmd, 0, { uint32_t( vertices.size( ) ),
        vertices.data( ) } );
      cmd->end( );
      _window->gfxQueue( )->submitAndWait( cmd );
    }

    // Index buffer
    {
      uint32_t indexBufferSize = indices.size( ) * sizeof( uint32_t );
      auto cmd = _window->gfxCommandPool( )->allocateCommandBuffer( );
      cmd->begin( );

      indexBuffer = device->createBuffer( indexBufferSize,
        vk::BufferUsageFlagBits::eIndexBuffer |
        vk::BufferUsageFlagBits::eTransferDst,
        vk::MemoryPropertyFlagBits::eDeviceLocal );
      indexBuffer->update<uint16_t>( cmd, 0, { uint32_t( indices.size( ) ),
        indices.data( ) } );
      cmd->end( );
      _window->gfxQueue( )->submitAndWait( cmd );
    }

    // VP buffer
    {
      vpBuffer = device->createUniformBuffer( sizeof( uboVS ) );
    }

    tex = device->createTexture2D( LAVA_EXAMPLES_IMAGES_ROUTE +
      std::string( "uv_checker.png" ), _window->gfxCommandPool( ),
      _window->gfxQueue( ), vk::Format::eR8G8B8A8Unorm );

    auto vertexStage = device->createShaderPipelineShaderStage(
      LAVA_EXAMPLES_SPV_ROUTE + std::string( "cubeUVPushModel_vert.spv" ),
      vk::ShaderStageFlagBits::eVertex
    );
    auto fragmentStage = device->createShaderPipelineShaderStage(
      LAVA_EXAMPLES_SPV_ROUTE + std::string( "cubeUV_frag.spv" ),
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

    descriptorSetLayout = device->createDescriptorSetLayout( dslbs, vk::DescriptorSetLayoutCreateFlagBits::ePushDescriptorKHR );

    vk::PushConstantRange pushConstantRange(
      vk::ShaderStageFlagBits::eVertex, 0, sizeof( PushConstant )
    );
    pipelineLayout = device->createPipelineLayout( descriptorSetLayout, pushConstantRange );

    vk::VertexInputBindingDescription binding( 0, sizeof( Vertex ),
      vk::VertexInputRate::eVertex );

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

    pipeline = device->createGraphicsPipeline( _window->pipelineCache, { },
    { vertexStage, fragmentStage }, vertexInput, assembly, nullptr,
      viewport, rasterization, multisample, depthStencil, colorBlend, dynamic,
      pipelineLayout, _window->defaultRenderPass( )
    );

    std::array<vk::DescriptorPoolSize, 2> poolSize =
    {
      vk::DescriptorPoolSize( vk::DescriptorType::eUniformBuffer, 1 ),
      vk::DescriptorPoolSize( vk::DescriptorType::eCombinedImageSampler, 1 )
    };
    auto dspPool = device->createDescriptorPool( 1, poolSize );

    // Init descriptor set
    descriptorSet = device->allocateDescriptorSet( dspPool, descriptorSetLayout );

    /*std::vector<WriteDescriptorSet> wdss =
    {
      WriteDescriptorSet(
        descriptorSet, 0, 0, vk::DescriptorType::eUniformBuffer,
        1, nullptr, DescriptorBufferInfo( vpBuffer, 0, sizeof( uboVS ) )
      ),
      WriteDescriptorSet(
        descriptorSet, 1, 0, vk::DescriptorType::eCombinedImageSampler, 1,
        tex->descriptor, nullptr
      )
    };
    device->updateDescriptorSets( wdss, { } );*/

    pipelineStatNames = {
      "Input assembly vertex count        ",
      "Input assembly primitives count    ",
      "Vertex shader invocations          ",
      "Clipping stage primitives processed",
      "Clipping stage primtives output    ",
      "Fragment shader invocations        "
    };
    pipelineStats.resize( pipelineStatNames.size( ) );
    query = device->createPipelineStatisticsQuery( 6,
      vk::QueryPipelineStatisticFlagBits::eInputAssemblyVertices |
      vk::QueryPipelineStatisticFlagBits::eInputAssemblyPrimitives |
      vk::QueryPipelineStatisticFlagBits::eVertexShaderInvocations |
      vk::QueryPipelineStatisticFlagBits::eClippingInvocations |
      vk::QueryPipelineStatisticFlagBits::eClippingPrimitives |
      vk::QueryPipelineStatisticFlagBits::eFragmentShaderInvocations );
  }
 
  // Retrieves the results of the pipeline statistics query submitted to the command buffer
  void getQueryResults( void )
  {
    uint32_t count = static_cast<uint32_t>( pipelineStats.size( ) );
    pipelineStats = query->getResults<uint64_t>( 0, 1, count,
      sizeof( uint64_t ), vk::QueryResultFlagBits::e64 );
    if ( !pipelineStats.empty( ) )
    {
      for ( uint32_t i = 0; i < pipelineStats.size( ); ++i )
      {
        std::cout << pipelineStatNames[ i ] << ": " << pipelineStats[ i ] << std::endl;
      }
    }
    std::cout << "====================================" << std::endl;
  }

  // Vector for storing pipeline statistics results
  std::vector<uint64_t> pipelineStats;
  std::vector<std::string> pipelineStatNames;
  std::shared_ptr<lava::QueryPool> query;

  virtual void initSwapChainResources( void ) override
  {
    if ( camera )
    {
      uint32_t w, h;

      auto device = _window->device( );

      auto sc = _window->getExtent( );
      w = sc.width;
      h = sc.height;

      camera->setFrustum( 
        lava::engine::Frustum( 75.0f, w * 1.0f / h, 0.01f, 100.0f ) );
    }
  }

  void nextFrame( void ) override
  {
    getQueryResults( );
    if ( Input::isKeyPressed( lava::Keyboard::Key::Esc ) )
    {
      _window->_window->close( );
    }

    // updateVP( );

    std::array<vk::ClearValue, 2 > clearValues;
    std::array<float, 4> ccv = { 0.2f, 0.3f, 0.3f, 1.0f };
    clearValues[ 0 ].color = vk::ClearColorValue( ccv );
    clearValues[ 1 ].depthStencil = vk::ClearDepthStencilValue( 1.0f, 0 );

    const vk::Offset2D size = _window->swapChainImageSize( );
    auto cmd = _window->currentCommandBuffer( );

    // Reset timestamp query pool
    cmd->resetQueryPool( query, 0, pipelineStats.size( ) );

    vk::Rect2D rect;
    rect.extent.width = size.x;
    rect.extent.height = size.y;
    cmd->beginRenderPass(
      _window->defaultRenderPass( ),
      _window->currentFramebuffer( ),
      rect, clearValues, vk::SubpassContents::eInline
    );
    cmd->setViewportScissors( _window->getExtent( ) );

    // Start capture of pipeline statistics
    cmd->beginQuery( query, 0, vk::QueryControlFlagBits::ePrecise );

    cmd->bindGraphicsPipeline( pipeline );
    cmd->bindDescriptorSets( vk::PipelineBindPoint::eGraphics,
      pipelineLayout, 0, { descriptorSet }, { } );

    cmd->bindVertexBuffer( 0, vertexBuffer, 0 );
    cmd->bindIndexBuffer( indexBuffer, 0, vk::IndexType::eUint16 );
    

    static auto startTime = std::chrono::high_resolution_clock::now( );

    auto currentTime = std::chrono::high_resolution_clock::now( );
    float time = std::chrono::duration_cast<std::chrono::milliseconds>( currentTime - startTime ).count( ) / 1000.0f;

    const clock_t begin_time = clock( );
    scene->getTransform( );
    lava::engine::FetchCameras fetchCameras;
    scene->perform( fetchCameras );
    std::vector<lava::engine::Camera*> cameras;
    fetchCameras.forEachCameras( [ & ]( lava::engine::Camera* c )
    {
      if ( lava::engine::Camera::getMainCamera( ) == nullptr || c->isMainCamera( ) )
      {
        lava::engine::Camera::setMainCamera( c );
      }
      cameras.push_back( c );
    } );

    // TODO: _simulationClock.tick( );
    // TODO: scene->perform( lava::engine::UpdateComponents( _simulationClock ) );
    std::vector< std::shared_ptr< lava::engine::BatchQueue > > bqCollection;

    for ( auto c : cameras )
    {
      if ( c != nullptr && c->isEnabled( ) )
      {
        auto bq = std::make_shared<lava::engine::BatchQueue>( );
        lava::engine::ComputeBatchQueue cbq( c, bq );
        scene->perform( cbq );
        bqCollection.push_back( bq );
      }
    }

    {
      auto size = _window->getExtent( );

      uint32_t width = size.width, height = size.height;

      lava::engine::Camera::getMainCamera( )->translate( 
        glm::vec3( 0.1f, 0.2f, 0.3f ) * std::sin( time ) );

      uboVS.view = lava::engine::Camera::getMainCamera( )->getView( );//glm::lookAt( glm::vec3( 2.0f, 2.0f, 20.0f ), glm::vec3( 0.0f, 0.0f, 0.0f ), glm::vec3( 0.0f, 0.0f, 1.0f ) );
      uboVS.proj = lava::engine::Camera::getMainCamera( )->getProjection( );//glm::perspective( glm::radians( 45.0f ), width / ( float ) height, 0.1f, 10.0f );

      // Vulkan clip space has inverted Y and half Z.
      glm::mat4 clip = glm::mat4(
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, -1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.5f, 0.0f,
        0.0f, 0.0f, 0.5f, 1.0f
      );
      uboVS.proj = clip * uboVS.proj;
      //uboVS.proj[ 1 ][ 1 ] *= -1;

      vpBuffer->writeData( 0, sizeof( uboVS ), &uboVS );
    }

    auto solidRenderables = bqCollection[ 0 ]->renderables(
      lava::engine::BatchQueue::RenderableType::OPAQUE );
    for ( const auto& r : solidRenderables )
    {
      std::vector<WriteDescriptorSet> wdss =
      {
        WriteDescriptorSet(
          nullptr, 0, 0, vk::DescriptorType::eUniformBuffer,
          1, nullptr, DescriptorBufferInfo( vpBuffer, 0, sizeof( uboVS ) )
        ),
        WriteDescriptorSet(
          nullptr, 1, 0, vk::DescriptorType::eCombinedImageSampler, 1,
          tex->descriptor, nullptr
        )
      };

      cmd->pushDescriptorSetKHR( vk::PipelineBindPoint::eGraphics,
        pipelineLayout, 0, wdss );

      pushConstant.model = glm::rotate( r.modelTransform, 
        time * glm::radians( 90.0f ), glm::vec3( 0.0f, 0.0f, 1.0f ) );
      cmd->pushConstants< PushConstant>( *pipelineLayout,
        vk::ShaderStageFlagBits::eVertex, 0, pushConstant );
      cmd->drawIndexed( indices.size( ), 1, 0, 0, 1 );
    }
    std::cout << float( clock( ) - begin_time ) / CLOCKS_PER_SEC << std::endl;

    // End capture of pipeline statistics
    cmd->endQuery( query, 0 );

    cmd->endRenderPass( );

    //_window->frameReady( );
    _window->requestUpdate( );
  }

private:
  VulkanWindow *_window;
  std::shared_ptr< Texture2D > tex;
  std::shared_ptr< DescriptorSetLayout > descriptorSetLayout;
  std::shared_ptr< DescriptorSet > descriptorSet;
  std::shared_ptr< PipelineLayout > pipelineLayout;
  std::shared_ptr< Pipeline > pipeline;
  std::shared_ptr< Buffer > vertexBuffer;
  std::shared_ptr< Buffer > indexBuffer;
  std::shared_ptr< UniformBuffer > vpBuffer;
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
    VK_EXT_DEBUG_REPORT_EXTENSION_NAME,
    // Enable extension required for push descriptors
    VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME
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
  w.resize( 500, 500 );

  w.show( );

  return 0;
}
