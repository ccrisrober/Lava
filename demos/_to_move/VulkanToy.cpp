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

#include <pompeii/pompeii.h>
#include <pompeiiRenderer/pompeiiRenderer.h>
using namespace pompeii;

#include <routes.h>

#include <glm/glm.hpp>

class Timer
{
public:
  Timer( void ) = default;
  ~Timer( void ) = default;

  Timer( const Timer& ) = default;
  Timer& operator=( const Timer& ) = default;
  Timer( Timer&& ) = default;
  Timer& operator=( Timer&& ) = default;

  void update( )
  {
    using namespace std::chrono;

    const high_resolution_clock::time_point currTime = high_resolution_clock::now( );
    const double deltaTimeMillis = 0.001 * ( double ) duration_cast<microseconds>( currTime - _prevTime ).count( );

    const double timePointMillis = 0.001 * ( double ) duration_cast<microseconds>( currTime - _prevFpsTime ).count( );
    if ( timePointMillis > _updateInterval )
    {
      fps = ( float ) std::round( _fpsFrameCounter / ( 0.001 * timePointMillis ) );
      _fpsFrameCounter = 0;
      _prevFpsTime = currTime;
      _fpsUpdated = true;
    }
    ++_fpsFrameCounter;

    deltaTimeSeconds = ( float ) ( 0.001 * deltaTimeMillis );
    timeSeconds += deltaTimeSeconds;
    _prevTime = currTime;

    {
      const time_t tmpTime = system_clock::to_time_t( system_clock::now( ) );
      tm localTm;
      localtime_s( &localTm, &tmpTime );
      year = ( float ) localTm.tm_year + 1900.0f;
      month = ( float ) localTm.tm_mon;
      day = ( float ) localTm.tm_mday;
      secs = ( float ) localTm.tm_sec;
    }
  }

  bool isFpsUpdated( void )
  {
    const bool val = _fpsUpdated;
    _fpsUpdated = false;
    return val;
  }

  float deltaTimeSeconds = 0.0f;
  float timeSeconds = 0.0f;
  float fps = 0.0f;

  float year = 0.0f;
  float month = 0.0f;
  float day = 0.0f;
  float secs = 0.0f;

private:
  const double _updateInterval = 16.6 * 10.0;

  std::chrono::high_resolution_clock::time_point _prevTime
  { std::chrono::high_resolution_clock::now( ) };

  std::chrono::high_resolution_clock::time_point _prevFpsTime
  { std::chrono::high_resolution_clock::now( ) };

  uint32_t _fpsFrameCounter = 0;
  bool _fpsUpdated = false;

};

class CustomRenderer : public VulkanWindowRenderer
{
  struct
  {
    glm::vec4 iMouse;
    glm::vec4 iDate;
    glm::vec4 iResolution;
    glm::vec4 iChannelTime;
    glm::vec4 iChannelResolution[ 4 ];

    float iGlobalDelta;
    float iGlobalFrame;
    float iGlobalTime;
    float iSampleRate;
  } shaderUniforms;

  float deltaTime = 0.0f;
  uint32_t frameIndex = 0;
  float globalTime = 0.0f;

  void updateShaderUniforms( void )
  {
    auto winSize = _window->getExtent( );
    shaderUniforms.iMouse[ 0 ] = ( float ) Input::MouseX( );
    shaderUniforms.iMouse[ 1 ] = ( float ) Input::MouseY( );
    shaderUniforms.iResolution[ 0 ] = winSize.width;
    shaderUniforms.iResolution[ 1 ] = winSize.height;
    shaderUniforms.iResolution[ 2 ] = shaderUniforms.iResolution[ 0 ] / 
      shaderUniforms.iResolution[ 1 ];
    shaderUniforms.iResolution[ 3 ] = 0.0f;
    shaderUniforms.iGlobalDelta = deltaTime;
    shaderUniforms.iGlobalFrame = ( float ) frameIndex;
    shaderUniforms.iSampleRate = 44100.0f; // don't know about this
    shaderUniforms.iGlobalTime = globalTime / 1000.0f;

    uniformBuffer->set( &shaderUniforms );

    std::cout <<
      "mouse: [" << Input::MouseX( ) << " , " 
          << Input::MouseY( ) << "]" << std::endl <<
      "delta time: " << deltaTime << std::endl <<
      "global time: " << globalTime << std::endl <<
      "global frame: " << frameIndex << std::endl << 
    std::endl;
  }
public:
  CustomRenderer( glfw::VulkanWindow *w )
    : VulkanWindowRenderer( )
    , _window( w )
  {
    _window->setWindowTitle( "Vulkan Toy" );
  }

  void initResources( void ) override
  {
    createDescriptorUniforms( );
    createGraphicPipeline( );
  }

  float lastFrame = 0.0f;

  void nextFrame( void ) override
  {
    if ( Input::isKeyPressed( pompeii::Keyboard::Key::Esc ) )
    {
      _window->_window->close( );
    }
    
    _timer.update( );

    if ( _timer.isFpsUpdated( ) )
    {
      _window->setWindowTitle( "    "
        + std::to_string( round( _timer.timeSeconds ) )
        + "    " + std::to_string( _timer.fps ) + " fps" );
    }

    render( );

    ++frameIndex;
  }
private:
 
  void render( void )
  {
    static auto startTime = std::chrono::high_resolution_clock::now( );

    auto currentTime = std::chrono::high_resolution_clock::now( );
    float time = std::chrono::duration_cast<std::chrono::milliseconds>(
      currentTime - startTime ).count( ) / 1000.0f;

    float currentFrame = time;
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    globalTime += time;

    updateShaderUniforms( );

    std::array<vk::ClearValue, 2 > clearValues;
    std::array<float, 4> ccv = { 0.0f, 0.0f, 0.0f, 1.0f };
    clearValues[ 0 ].color = vk::ClearColorValue( ccv );
    clearValues[ 1 ].depthStencil = vk::ClearDepthStencilValue( 1.0f, 0 );

    const vk::Offset2D size = _window->swapChainImageSize( );
    auto cmd = _window->currentCommandBuffer( );
    vk::Rect2D rect;
    rect.extent.width = size.x;
    rect.extent.height = size.y;
    cmd->beginRenderPass(
      _window->defaultRenderPass( ),
      _window->currentFramebuffer( ),
      rect, clearValues, vk::SubpassContents::eInline
    );

    cmd->bindGraphicsPipeline( pipeline );
    cmd->bindDescriptorSets( vk::PipelineBindPoint::eGraphics, pipelineLayout, 
      0, { descriptorSet }, nullptr );
    cmd->setViewportScissors( _window->getExtent( ) );
    cmd->draw( 4, 1, 0, 0 );

    cmd->endRenderPass( );

    _window->requestUpdate( );
  }
  
  VulkanWindow *_window;

  std::shared_ptr< DescriptorSet > descriptorSetUniform;
  std::shared_ptr< UniformBuffer > uniformBuffer;

  //PipelineShaderStageCreateInfo vertexStage;
  //PipelineShaderStageCreateInfo fragmentStage;
private:
  void createDescriptorUniforms( void )
  {
    auto device = _window->device( );
    uniformBuffer = device->createUniformBuffer( sizeof( shaderUniforms ) );
    
    std::vector<DescriptorSetLayoutBinding> dslbs = 
    {
      DescriptorSetLayoutBinding( 0,
        vk::DescriptorType::eUniformBuffer, // eUniformBufferDynamic, 
        vk::ShaderStageFlagBits::eFragment
      )
    };

    auto descriptorSetLayout = device->createDescriptorSetLayout( dslbs );
    pipelineLayout = device->createPipelineLayout( descriptorSetLayout );

    std::shared_ptr<DescriptorPool> dpool = device->createDescriptorPool( 1, {
      { vk::DescriptorType::eUniformBuffer, 1 }
    } );

    descriptorSet = device->allocateDescriptorSet( dpool, descriptorSetLayout );

    std::vector<WriteDescriptorSet> wdss =
    {
      WriteDescriptorSet( descriptorSet, 0, 0,
      vk::DescriptorType::eUniformBuffer, 1, nullptr,
        DescriptorBufferInfo(
          uniformBuffer, 0, sizeof( shaderUniforms )
        )
      )
    };
    device->updateDescriptorSets( wdss, { } );
  }

  void createGraphicPipeline( void )
  {
    auto device = _window->device( );

    std::cout << "Loading shaders ... " << std::endl;

    auto vertexStage = device->createShaderPipelineShaderStage(
      POMPEII_EXAMPLES_SPV_ROUTE + std::string( "toy_vert.spv" ),
      vk::ShaderStageFlagBits::eVertex
    );
    auto fragmentStage = device->createShaderPipelineShaderStage(
      POMPEII_EXAMPLES_SPV_ROUTE + std::string( "toy_frag.spv" ),
      vk::ShaderStageFlagBits::eFragment
    );

    std::cout << "Creating pipeline ... " << std::endl;

    PipelineVertexInputStateCreateInfo vertexInput( { }, { } );

    vk::PipelineInputAssemblyStateCreateInfo assembly( { },
      vk::PrimitiveTopology::eTriangleStrip, VK_FALSE );
    PipelineViewportStateCreateInfo viewport( 1, 1 );
    vk::PipelineRasterizationStateCreateInfo rasterization( { }, true, false,
      vk::PolygonMode::eFill, vk::CullModeFlagBits::eBack,
      vk::FrontFace::eClockwise, false, 0.0f, 0.0f, 0.0f, 1.0f
    );
    PipelineMultisampleStateCreateInfo multisample( vk::SampleCountFlagBits::e1,
      false, 0.0f, nullptr, false, false
    );
    vk::StencilOpState stencilOpState( vk::StencilOp::eKeep,
      vk::StencilOp::eKeep, vk::StencilOp::eKeep, vk::CompareOp::eAlways,
      0, 0, 0
    );
    vk::PipelineDepthStencilStateCreateInfo depthStencil( { }, true, true,
      vk::CompareOp::eLessOrEqual, false, false, stencilOpState,
      stencilOpState, 0.0f, 0.0f
    );
    vk::PipelineColorBlendAttachmentState colorBlendAttachment( false,
      vk::BlendFactor::eZero, vk::BlendFactor::eZero, vk::BlendOp::eAdd,
      vk::BlendFactor::eZero, vk::BlendFactor::eZero, vk::BlendOp::eAdd,
      vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
      vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA
    );
    PipelineColorBlendStateCreateInfo colorBlend( false, vk::LogicOp::eNoOp,
      colorBlendAttachment, { 1.0f, 1.0f, 1.0f, 1.0f }
    );
    PipelineDynamicStateCreateInfo dynamic( {
      vk::DynamicState::eViewport, vk::DynamicState::eScissor
    } );

    pipeline = device->createGraphicsPipeline( _window->pipelineCache, { },
    { vertexStage, fragmentStage }, vertexInput, assembly, nullptr,
      viewport, rasterization, multisample, depthStencil, colorBlend, dynamic,
      pipelineLayout, _window->defaultRenderPass( )
    );
  }

  std::shared_ptr<DescriptorSet> descriptorSet;
  std::shared_ptr<PipelineLayout> pipelineLayout;
  std::shared_ptr<Pipeline> pipeline;
  Timer _timer;
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
    POMPEII_KHR_EXT, // OS specific surface extension
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
  w.resize( 500, 281 );

  w.show( );

  return 0;
}
