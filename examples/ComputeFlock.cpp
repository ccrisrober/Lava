#include <lava/lava.h>
using namespace lava;
#include <routes.h>

class CustomRenderer : public VulkanWindowRenderer
{
public:
  CustomRenderer( lava::VulkanWindow *w )
    : VulkanWindowRenderer( )
    , _window( w )
  {
  }

  struct Particle
  {
    glm::vec3	pos;
    glm::vec3	ppos;
    glm::vec3	home;
    glm::vec4	color;
    float	damping;
  };

  struct ComputeStructBuffer
  {
    //Steering radii
    float uSeparationRadius;
    float uCohesionRadius;
    float uAlignRadius;

    //Steering strengths
    float uSeparationStrength;
    float uCohesionStrength;
    float uAlignStrength;

    //Misc. Uniforms
    float uTimeStep;
    float uBoidSpeed;
    glm::vec2  uWindowCenter;
    float uColorRadius;
  } comp;

  struct VertexStructBuffer
  {
    glm::mat4 mvp;
  } vert;

  std::shared_ptr< StorageBuffer > particleBuffer;
  std::shared_ptr< UniformBuffer > computeBuffer;
  std::shared_ptr< UniformBuffer > vertexBuffer;
  
  void initResources( void ) override
  {
    auto device = _window->device( );

    std::cout << "Allocate buffers...";
    particleBuffer = device->createStorageBuffer( sizeof(Particle) * NUM_PARTICLES );
    device->createUniformBuffer( sizeof( comp ) );
    device->createUniformBuffer( sizeof( vert ) );
    std::cout << "OK" << std::endl;

    std::vector<DescriptorSetLayoutBinding> dslbs =
    {
      DescriptorSetLayoutBinding(
        0, vk::DescriptorType::eStorageBuffer,
        vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eCompute
      ),
      DescriptorSetLayoutBinding(
        1, vk::DescriptorType::eUniformBuffer,
        vk::ShaderStageFlagBits::eCompute
      ),
      DescriptorSetLayoutBinding(
        2, vk::DescriptorType::eUniformBuffer,
        vk::ShaderStageFlagBits::eVertex
      )
    };

    descriptorSetLayout = device->createDescriptorSetLayout( dslbs );

    pipelineLayout = device->createPipelineLayout( descriptorSetLayout, nullptr );

    {
      auto vertexStage = device->createShaderPipelineShaderStage(
        LAVA_EXAMPLES_SPV_ROUTE + std::string( "flock_vert.spv" ),
        vk::ShaderStageFlagBits::eVertex
      );
      auto fragmentStage = device->createShaderPipelineShaderStage(
        LAVA_EXAMPLES_SPV_ROUTE + std::string( "flock_frag.spv" ),
        vk::ShaderStageFlagBits::eFragment
      );

      PipelineVertexInputStateCreateInfo vertexInput( {}, {} );

      vk::PipelineInputAssemblyStateCreateInfo assembly( {},
        vk::PrimitiveTopology::eTriangleStrip, VK_FALSE );
      PipelineViewportStateCreateInfo viewport( 1, 1 );
      vk::PipelineRasterizationStateCreateInfo rasterization( {}, true, false,
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
      vk::PipelineDepthStencilStateCreateInfo depthStencil( {}, true, true,
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

      pipeline = device->createGraphicsPipeline( _window->pipelineCache, {},
      { vertexStage, fragmentStage }, vertexInput, assembly, nullptr,
        viewport, rasterization, multisample, depthStencil, colorBlend, dynamic,
        pipelineLayout, _window->defaultRenderPass( )
      );
    }

    std::shared_ptr<DescriptorPool> descriptorPool =
      device->createDescriptorPool( 1, {
        { vk::DescriptorType::eUniformBuffer, 2 },
        { vk::DescriptorType::eStorageBuffer, 1 },
      } );

    // Init descriptor set
    descriptorSet = device->allocateDescriptorSet( descriptorPool, descriptorSetLayout );
    std::vector<WriteDescriptorSet> wdss =
    {
      WriteDescriptorSet( descriptorSet, 0, 0,
        vk::DescriptorType::eStorageBuffer, 1, nullptr,
        DescriptorBufferInfo( particleBuffer, 0, sizeof( Particle ) * NUM_PARTICLES )
      ),
      WriteDescriptorSet( descriptorSet, 1, 0,
        vk::DescriptorType::eUniformBuffer, 1, nullptr,
        DescriptorBufferInfo( computeBuffer, 0, sizeof( comp ) )
      ),
      WriteDescriptorSet( descriptorSet, 2, 0,
        vk::DescriptorType::eUniformBuffer, 1, nullptr,
        DescriptorBufferInfo( vertexBuffer, 0, sizeof( vert ) )
      )
    };

    device->updateDescriptorSets( wdss, {} );

    // Compute stage
    {
      std::cout << "Loading shader... ";
      auto computeStage = device->createShaderPipelineShaderStage(
        LAVA_EXAMPLES_SPV_ROUTE + std::string( "flock_comp.spv" ),
        vk::ShaderStageFlagBits::eCompute
      );

      compute.pipeline = device->createComputePipeline(
        nullptr, {}, computeStage, pipelineLayout );

      std::cout << "OK" << std::endl;

      auto queueFamilyIndices = _window->physicalDevice( )->getComputeQueueFamilyIndices( );
      assert( !queueFamilyIndices.empty( ) );
      const uint32_t queueFamilyIndex = queueFamilyIndices[ 0 ];

      compute.queue = device->getQueue( queueFamilyIndex, 0 );

      compute.commandPool = device->createCommandPool( {}, compute.queue->getQueueFamilyIndex( ) );
      compute.commandBuffer = compute.commandPool->allocateCommandBuffer( );

      // Build compute command buffer
      {
        compute.commandBuffer->beginSimple( );
        compute.commandBuffer->bindComputePipeline( compute.pipeline );
        compute.commandBuffer->bindDescriptorSets( vk::PipelineBindPoint::eCompute,
          pipelineLayout, 0, { descriptorSet }, {} );
        compute.commandBuffer->dispatch( NUM_PARTICLES / WORK_GROUP_SIZE, 1, 1 );
        compute.commandBuffer->end( );
      }
    }
  }

  void releaseResources( void ) override
  {
    if ( descriptorSetLayout )
    {
      descriptorSetLayout.reset( );
    }
    if ( descriptorSet )
    {
      descriptorSet.reset( );
    }
    if ( pipelineLayout )
    {
      pipelineLayout.reset( );
    }
    if ( pipeline )
    {
      pipeline.reset( );
    }
    if ( tex )
    {
      tex.reset( );
    }
  }

  void nextFrame( void ) override
  {
    if ( Input::isKeyPressed( lava::Keyboard::Key::Esc ) )
    {
      _window->_window->close( );
    }
    static int i = 0;

    _window->setWindowTitle( std::string( "Step: " ) + std::to_string( i ) );

    ++i;

    static auto startTime = std::chrono::high_resolution_clock::now( );

    auto currentTime = std::chrono::high_resolution_clock::now( );
    float time = std::chrono::duration_cast<std::chrono::milliseconds>(
      currentTime - startTime ).count( ) / 1000.0f;

    _green = sin( time ) / 2.0f + 0.5f;

    std::array<vk::ClearValue, 2 > clearValues;
    std::array<float, 4> ccv = { 0.0f, _green, 0.0f, 1.0f };
    clearValues[ 0 ].color = vk::ClearColorValue( ccv );
    clearValues[ 1 ].depthStencil  = vk::ClearDepthStencilValue(  1.0f, 0 );

    secondaryCmd = _window->_cmdPool->allocateCommandBuffer( vk::CommandBufferLevel::eSecondary );

    vk::CommandBufferInheritanceInfo inheritInfo;
    inheritInfo.renderPass = *_window->defaultRenderPass( );
    inheritInfo.framebuffer = *_window->currentFramebuffer( );

    secondaryCmd->beginSimple( vk::CommandBufferUsageFlagBits::eOneTimeSubmit | vk::CommandBufferUsageFlagBits::eRenderPassContinue, &inheritInfo );
    secondaryCmd->bindGraphicsPipeline( pipeline );

    secondaryCmd->bindDescriptorSets( vk::PipelineBindPoint::eGraphics,
      pipelineLayout, 0, { descriptorSet }, nullptr );

    secondaryCmd->setViewportScissors( _window->getExtent( ) );
    secondaryCmd->draw( 4, 1, 0, 0 );

    secondaryCmd->end( );

    const glm::ivec2 size = _window->swapChainImageSize( );
    auto cmd = _window->currentCommandBuffer( );
    vk::Rect2D rect;
    rect.extent.width = size.x;
    rect.extent.height = size.y;
    cmd->beginRenderPass( 
      _window->defaultRenderPass( ), 
      _window->currentFramebuffer( ), 
      rect, clearValues, vk::SubpassContents::eSecondaryCommandBuffers
    );

    /*cmd->bindGraphicsPipeline( pipeline );

    cmd->bindDescriptorSets( vk::PipelineBindPoint::eGraphics,
      pipelineLayout, 0, { descSet }, nullptr );

    cmd->setViewportScissors( _window->getExtent( ) );
    cmd->draw( 4, 1, 0, 0 );*/

    cmd->executeCommands( { secondaryCmd } );

    cmd->endRenderPass( );

    _window->frameReady( );
    //_window->requestUpdate( );
  }

private:
  VulkanWindow *_window;
  float _green = 0;

  const int NUM_PARTICLES = 60000;
  const int WORK_GROUP_SIZE = 128;

  struct
  {
    std::shared_ptr<Queue> queue;
    std::shared_ptr< CommandPool > commandPool;
    CommandBufferPtr commandBuffer;

    std::shared_ptr< Pipeline > pipeline;
  } compute;

  std::shared_ptr< DescriptorSetLayout > descriptorSetLayout;
  std::shared_ptr< DescriptorSet > descriptorSet;
  std::shared_ptr< PipelineLayout > pipelineLayout;
  std::shared_ptr< Pipeline > pipeline;
  std::shared_ptr<Texture2D> tex;

  CommandBufferPtr secondaryCmd;
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
    VK_EXT_DEBUG_REPORT_EXTENSION_NAME,
#endif
  };
  std::vector<const char*> extensions =
  {
    VK_KHR_SURFACE_EXTENSION_NAME,  // Surface extension
    "VK_KHR_win32_surface", //LAVA_KHR_EXT, // OS specific surface extension
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
  w.resize( 500, 500 );

  w.show( );

  auto res = w.supportedSampleCounts( );
  return 0;
}