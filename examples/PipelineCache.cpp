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

struct
{
  glm::mat4 model;
  glm::mat4 view;
  glm::mat4 proj;
} uboVS;

#include <iomanip>

class MyApp : public VulkanApp
{
public:
  std::shared_ptr<Buffer> uniformMVP;
  std::shared_ptr<PipelineLayout> pipelineLayout;
  std::shared_ptr<DescriptorSet> descriptorSet;

  std::shared_ptr<lava::extras::Geometry> geometry;
  std::shared_ptr<CommandPool> commandPool;

  std::array<glm::vec4, 1> pushConstants;

  void print_UUID( uint8_t *pipelineCacheUUID )
  {
    for ( int j = 0; j < VK_UUID_SIZE; ++j ) {
      std::cout << std::setw( 2 ) << ( uint32_t ) pipelineCacheUUID[ j ];
      if ( j == 3 || j == 5 || j == 7 || j == 9 ) {
        std::cout << '-';
      }
    }
  }

  struct Pipelines
  {
    std::shared_ptr<Pipeline> solid;
    std::shared_ptr<Pipeline> wireframe;
  } pipelines;

  MyApp( char const* title, uint32_t width, uint32_t height )
    : VulkanApp( title, width, height )
  {
    commandPool = _device->createCommandPool( 
      vk::CommandPoolCreateFlagBits::eResetCommandBuffer, _queueFamilyIndex );

    geometry = std::make_shared<lava::extras::Geometry>( _device, 
      LAVA_EXAMPLES_MESHES_ROUTE + std::string( "monkey.obj_" ) );

    // MVP buffer
    {
      uint32_t mvpBufferSize = sizeof( uboVS );
      uniformMVP = _device->createBuffer( mvpBufferSize, 
        vk::BufferUsageFlagBits::eUniformBuffer, 
        vk::SharingMode::eExclusive, nullptr,
        vk::MemoryPropertyFlagBits::eHostVisible | 
          vk::MemoryPropertyFlagBits::eHostCoherent );
    }

    // Init descriptor and pipeline layouts
    std::vector<DescriptorSetLayoutBinding> dslbs = 
    {
      DescriptorSetLayoutBinding( 
        0, vk::DescriptorType::eUniformBuffer, 
        vk::ShaderStageFlagBits::eVertex
      )
    };
    std::shared_ptr<DescriptorSetLayout> descriptorSetLayout = 
      _device->createDescriptorSetLayout( dslbs );

    pipelineLayout = _device->createPipelineLayout( descriptorSetLayout, nullptr );

    std::string filePath = "pipelineCacheData.bin";
    std::ifstream file( filePath, std::ios::ate | std::ios::binary );

    std::shared_ptr<PipelineCache> pipelineCache;
    size_t startCacheSize = 0;
    char* startCacheData = nullptr;
    if ( !file.is_open( ) )
    {
      std::cerr << "File " << filePath << 
        " don't opened. Creating empty pipeline_cache" << std::endl;
    }
    else
    {
      size_t startCacheSize = ( size_t ) file.tellg( );
      char* startCacheData = new char[ startCacheSize ];

      file.seekg( 0 );
      file.read( startCacheData, startCacheSize );

      file.close( );

      std::cout << "  Pipeline cache HIT!\n";
      std::cout << "  cacheData loaded from " << filePath << std::endl;

      // clang-format on
      uint32_t headerLength = 0;
      uint32_t cacheHeaderVersion = 0;
      uint32_t vendorID = 0;
      uint32_t deviceID = 0;
      uint8_t pipelineCacheUUID[ VK_UUID_SIZE ] = { };

      memcpy( &headerLength, ( uint8_t * ) startCacheData + 0, 4 );
      memcpy( &cacheHeaderVersion, ( uint8_t * ) startCacheData + 4, 4 );
      memcpy( &vendorID, ( uint8_t * ) startCacheData + 8, 4 );
      memcpy( &deviceID, ( uint8_t * ) startCacheData + 12, 4 );
      memcpy( pipelineCacheUUID, ( uint8_t * ) startCacheData + 16, VK_UUID_SIZE );


      // Check each field and report bad values before freeing existing cache
      bool badCache = false;

      if ( headerLength <= 0 )
      {
        badCache = true;
        printf( "  Bad header length in %s.\n", filePath.c_str( ) );
        printf( "    Cache contains: 0x%.8x\n", headerLength );
      }

      if ( cacheHeaderVersion != VK_PIPELINE_CACHE_HEADER_VERSION_ONE )
      {
        badCache = true;
        printf( "  Unsupported cache header version in %s.\n", filePath.c_str( ) );
        printf( "    Cache contains: 0x%.8x\n", cacheHeaderVersion );
      }

      auto props = _device->_physicalDevice->getDeviceProperties( );

      if ( vendorID != props.vendorID )
      {
        badCache = true;
        printf( "  Vendor ID mismatch in %s.\n", filePath.c_str( ) );
        printf( "    Cache contains: 0x%.8x\n", vendorID );
        printf( "    Driver expects: 0x%.8x\n", props.vendorID );
      }

      if ( deviceID != props.deviceID )
      {
        badCache = true;
        printf( "  Device ID mismatch in %s.\n", filePath.c_str( ) );
        printf( "    Cache contains: 0x%.8x\n", deviceID );
        printf( "    Driver expects: 0x%.8x\n", props.deviceID );
      }

      if ( memcmp( pipelineCacheUUID, props.pipelineCacheUUID, 
        sizeof( pipelineCacheUUID ) ) != 0 )
      {
        badCache = true;
        printf( "  UUID mismatch in %s.\n", filePath.c_str( ) );
        printf( "    Cache contains: " );
        print_UUID( pipelineCacheUUID );
        printf( "\n" );
        printf( "    Driver expects: " );
        print_UUID( props.pipelineCacheUUID );
        printf( "\n" );
      }

      if ( badCache )
      {
        // Don't submit initial cache data if any version info is incorrect
        free( startCacheData );
        startCacheSize = 0;
        startCacheData = nullptr;

        // And clear out the old cache file for use in next run
        printf( "  Deleting cache entry %s to repopulate.\n", filePath.c_str( ) );
        if ( remove( filePath.c_str( ) ) != 0 ) {
          fputs( "Reading error", stderr );
          exit( EXIT_FAILURE );
        }
      }
    }

    pipelineCache = _device->createPipelineCache( startCacheSize, startCacheData );

    // init shaders
    std::shared_ptr<ShaderModule> fragmentShaderModule = _device->createShaderModule(
      LAVA_EXAMPLES_SPV_ROUTE + std::string( "mesh_ctes_frag.spv" ), vk::ShaderStageFlagBits::eFragment );

    // init pipeline
    
    vk::PipelineInputAssemblyStateCreateInfo assembly( { }, vk::PrimitiveTopology::eTriangleList, VK_FALSE );
    PipelineViewportStateCreateInfo viewport( 1, 1 ); 
    vk::PipelineRasterizationStateCreateInfo rasterization( { }, true,
      false, vk::PolygonMode::eFill, vk::CullModeFlagBits::eBack,
      vk::FrontFace::eCounterClockwise, false, 0.0f, 0.0f, 0.0f, 1.0f );
    PipelineMultisampleStateCreateInfo multisample( vk::SampleCountFlagBits::e1, false, 0.0f, nullptr, false, false );
    vk::StencilOpState stencilOpState( vk::StencilOp::eKeep, vk::StencilOp::eKeep, vk::StencilOp::eKeep, vk::CompareOp::eAlways, 0, 0, 0 );
    vk::PipelineDepthStencilStateCreateInfo depthStencil( { }, true, true, vk::CompareOp::eLessOrEqual, false, false, stencilOpState, stencilOpState, 0.0f, 0.0f );
    vk::PipelineColorBlendAttachmentState colorBlendAttachment( false, vk::BlendFactor::eZero, vk::BlendFactor::eZero, vk::BlendOp::eAdd, vk::BlendFactor::eZero, vk::BlendFactor::eZero, vk::BlendOp::eAdd,
      vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA );
    PipelineColorBlendStateCreateInfo colorBlend( false, vk::LogicOp::eNoOp, colorBlendAttachment, { 1.0f, 1.0f, 1.0f, 1.0f } );
    PipelineDynamicStateCreateInfo dynamic( { vk::DynamicState::eViewport, vk::DynamicState::eScissor } );


    struct SpecializationData
    {
      uint8_t model;
    } specData;

    
    std::array<vk::SpecializationMapEntry, 1> specMapEntries;

    vk::SpecializationMapEntry specMapEntry( 0, 0, sizeof( specData.model ) );

    PipelineVertexInputStateCreateInfo vertexInput(
      vk::VertexInputBindingDescription( 0, sizeof( lava::extras::Vertex ),
        vk::VertexInputRate::eVertex ),
        {
          vk::VertexInputAttributeDescription( 0, 0, vk::Format::eR32G32B32Sfloat, offsetof( lava::extras::Vertex, position ) ),
          vk::VertexInputAttributeDescription( 1, 0, vk::Format::eR32G32B32Sfloat, offsetof( lava::extras::Vertex, normal ) ),
          vk::VertexInputAttributeDescription( 2, 0, vk::Format::eR32G32Sfloat, offsetof( lava::extras::Vertex, texCoord ) )
        }
    );
    PipelineShaderStageCreateInfo vertexStage = 
      _device->createShaderPipelineShaderStage( LAVA_EXAMPLES_SPV_ROUTE + 
        std::string( "mesh_ctes_vert.spv" ), vk::ShaderStageFlagBits::eVertex );
    {
      specData.model = 0;
      lava::SpecializationInfo specInfo( { specMapEntry }, { specData.model } );
      PipelineShaderStageCreateInfo fragmentStage( vk::ShaderStageFlagBits::eFragment,
        fragmentShaderModule, "main", specInfo );

      pipelines.solid = _device->createGraphicsPipeline( pipelineCache, 
        // Specify that we will be creating a derivative of this pipeline.
        vk::PipelineCreateFlagBits::eAllowDerivatives,
          { vertexStage, fragmentStage }, vertexInput, assembly, nullptr,
        viewport, rasterization, multisample, depthStencil, colorBlend, dynamic,
        pipelineLayout, _renderPass );
    }
    {
      specData.model = 1;
      lava::SpecializationInfo specInfo( { specMapEntry }, { specData.model } );
      PipelineShaderStageCreateInfo fragmentStage( vk::ShaderStageFlagBits::eFragment,
        fragmentShaderModule, "main", specInfo );

      rasterization = vk::PipelineRasterizationStateCreateInfo( { }, true,
        false, vk::PolygonMode::eLine, vk::CullModeFlagBits::eNone,
        vk::FrontFace::eCounterClockwise, false, 0.0f, 0.0f, 0.0f, 1.0f );

      pipelines.wireframe = _device->createGraphicsPipeline( pipelineCache,
        // Modify pipeline info to reflect derivation
        vk::PipelineCreateFlagBits::eDerivative,
          { vertexStage, fragmentStage }, vertexInput, assembly, nullptr, 
        viewport, rasterization, multisample, depthStencil, colorBlend, dynamic,
        pipelineLayout, _renderPass, 0, pipelines.solid, -1 );
    }

    std::array<vk::DescriptorPoolSize, 1> poolSize =
    {
      vk::DescriptorPoolSize( vk::DescriptorType::eUniformBuffer, 1 )
    };
    auto descriptorPool = _device->createDescriptorPool( { }, 1, poolSize );

    // Init descriptor set
    descriptorSet = _device->allocateDescriptorSet( descriptorPool, descriptorSetLayout );
    std::vector<WriteDescriptorSet> wdss = 
    {
      WriteDescriptorSet( descriptorSet, 0, 0, 
        vk::DescriptorType::eUniformBuffer, 1, nullptr, 
        DescriptorBufferInfo( 
          uniformMVP, 0, sizeof( uboVS )
        )
      )
    };
    _device->updateDescriptorSets( wdss, { } );

    pipelineCache->saveToFile( filePath );
  }
  void updateUniformBuffers( void )
  {
    uint32_t width = _window->getWidth( );
    uint32_t height = _window->getHeight( );

    static auto startTime = std::chrono::high_resolution_clock::now( );

    auto currentTime = std::chrono::high_resolution_clock::now( );
    float time = std::chrono::duration_cast<std::chrono::milliseconds>( currentTime - startTime ).count( ) / 1000.0f;

    uboVS.model = glm::rotate( glm::mat4( 1.0f ), time * glm::radians( 90.0f ), glm::vec3( 0.0f, -1.0f, 0.0f ) );
    glm::vec3 cameraPos = glm::vec3( 0.0f, 0.0f, 0.35f );
    glm::vec3 cameraFront = glm::vec3( 0.0f, 0.0f, -1.0f );
    glm::vec3 cameraUp = glm::vec3( 0.0f, 1.0f, 0.0f );

    uboVS.view = glm::lookAt( cameraPos, cameraPos + cameraFront, cameraUp );

    uboVS.proj = glm::perspective( glm::radians( 45.0f ), width / ( float ) height, 0.1f, 10.0f );
    uboVS.proj[ 1 ][ 1 ] *= -1;

    uniformMVP->writeData( 0, sizeof( uboVS ), &uboVS );
  }

  bool enable_wire = false;

  void doPaint( void ) override
  {
    float width = ( float ) _defaultFramebuffer->getExtent( ).width;
    float height = ( float ) _defaultFramebuffer->getExtent( ).height;

    updateUniformBuffers( );

    auto commandBuffer = commandPool->allocateCommandBuffer( );

    commandBuffer->begin( );

    std::array<float, 4> ccv = { 0.2f, 0.3f, 0.3f, 1.0f };
    commandBuffer->beginRenderPass( _renderPass, 
      _defaultFramebuffer->getFramebuffer( ), 
      vk::Rect2D( { 0, 0 }, 
      _defaultFramebuffer->getExtent( ) ),
      { vk::ClearValue( ccv ), vk::ClearValue( 
        vk::ClearDepthStencilValue( 1.0f, 0 ) )
      }, vk::SubpassContents::eInline );

    commandBuffer->bindDescriptorSets( vk::PipelineBindPoint::eGraphics,
      pipelineLayout, 0, { descriptorSet }, nullptr );

    vk::Viewport viewport( 0.0f, 0.0f, width, height, 0.0f, 1.0f );

    commandBuffer->setScissor( 0, vk::Rect2D( { 0, 0 }, _defaultFramebuffer->getExtent( ) ) );

    // Left
    viewport.width = width / 2.0f;
    commandBuffer->setViewport( 0, viewport );
    commandBuffer->bindGraphicsPipeline( pipelines.solid );
    geometry->render( commandBuffer );

    // Right
    viewport.x = width / 2.0f;
    commandBuffer->setViewport( 0, viewport );
    commandBuffer->bindGraphicsPipeline( pipelines.wireframe );
    geometry->render( commandBuffer );


    commandBuffer->endRenderPass( );

    commandBuffer->end( );

    _graphicsQueue->submit( SubmitInfo{
      { _defaultFramebuffer->getPresentSemaphore( ) },
      { vk::PipelineStageFlagBits::eColorAttachmentOutput },
      commandBuffer,
      _renderComplete
    } );
  }
  void keyEvent( int key, int scancode, int action, int mods )
  {
    switch ( key )
    {
    case GLFW_KEY_E:
      enable_wire = true;
      break;
    case GLFW_KEY_R:
      enable_wire = false;
      break;
    case GLFW_KEY_ESCAPE:
      switch ( action )
      {
      case GLFW_PRESS:
        getWindow( )->close( );
        break;
      default:
        break;
      }
      break;
    default:
      break;
    }
  }
};

void glfwErrorCallback( int error, const char* description )
{
  fprintf( stderr, "GLFW Error %d: %s\n", error, description );
}

int main( void )
{
  try
  {
    VulkanApp* app = new MyApp( "Pipeline Derivation", 1200, 600 );

    app->getWindow( )->setErrorCallback( glfwErrorCallback );

    while ( app->isRunning( ) )
    {
      //app->waitEvents( );
      app->paint( );
    }

    delete app;
  }
  catch ( std::system_error err )
  {
    std::cout << "System Error: " << err.what( ) << std::endl;
  }
  return 0;
}