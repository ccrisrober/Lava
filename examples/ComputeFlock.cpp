#include <lava/lava.h>
using namespace lava;
#include <routes.h>
#include <random>
#define M_PI 3.14159

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
    float uPointSize;
  } vert;

  std::shared_ptr< Buffer > particleBuffer;
  std::shared_ptr< UniformBuffer > computeBuffer;
  std::shared_ptr< UniformBuffer > vertexBuffer;
  
  void initResources( void ) override
  {
    auto device = _window->device( );

    std::cout << "Allocate buffers...";
    particleBuffer = /*device->createBuffer( sizeof( Particle ) * NUM_PARTICLES, 
      vk::BufferUsageFlagBits::eVertexBuffer |
      vk::BufferUsageFlagBits::eStorageBuffer | 
      vk::BufferUsageFlagBits::eTransferDst, vk::SharingMode::eExclusive, 
      nullptr, vk::MemoryPropertyFlagBits::eDeviceLocal );*/
    device->createStorageBuffer( sizeof( Particle ) * NUM_PARTICLES );

    computeBuffer = device->createUniformBuffer( sizeof( comp ) );
    vertexBuffer = device->createUniformBuffer( sizeof( vert ) );
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

      /*vk::VertexInputBindingDescription binding(
        0, sizeof( int ), vk::VertexInputRate::eVertex );

      PipelineVertexInputStateCreateInfo vertexInput( binding, {
          vk::VertexInputAttributeDescription( 0, 0, vk::Format::eR16Sint, 0
        ) }
      );*/
      
      PipelineVertexInputStateCreateInfo vertexInput( { }, { } );

      vk::PipelineInputAssemblyStateCreateInfo assembly( { },
        vk::PrimitiveTopology::ePointList, VK_FALSE );
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

        /*// Add memory barrier to ensure that the (graphics) vertex shader has fetched attributes before compute starts to write to the buffer
        vk::BufferMemoryBarrier bufferBarrier;
        bufferBarrier.buffer = *computeBuffer;
        bufferBarrier.size = sizeof( Particle ) * NUM_PARTICLES;
        bufferBarrier.srcAccessMask = vk::AccessFlagBits::eVertexAttributeRead;						// Vertex shader invocations have finished reading from the buffer
        bufferBarrier.dstAccessMask = vk::AccessFlagBits::eShaderWrite;								// Compute shader wants to write to the buffer
                                                                                // Compute and graphics queue may have different queue families (see VulkanDevice::createLogicalDevice)
                                                                                // For the barrier to work across different queues, we need to set their family indices
        bufferBarrier.srcQueueFamilyIndex = _window->graphicQueue()->getQueueFamilyIndex( );			// Required as compute and graphics queue may have different families
        bufferBarrier.dstQueueFamilyIndex = compute.queue->getQueueFamilyIndex( );			// Required as compute and graphics queue may have different families


        compute.commandBuffer->pipelineBarrier( 
          vk::PipelineStageFlagBits::eVertexShader, 
          vk::PipelineStageFlagBits::eComputeShader,
          { }, { }, { bufferBarrier }, { }
        );*/

        compute.commandBuffer->bindComputePipeline( compute.pipeline );
        compute.commandBuffer->bindDescriptorSets( vk::PipelineBindPoint::eCompute,
          pipelineLayout, 0, { descriptorSet }, {} );
        compute.commandBuffer->dispatch( NUM_PARTICLES / WORK_GROUP_SIZE, 1, 1 );

        /*// Add memory barrier to ensure that compute shader has finished writing to the buffer
        // Without this the (rendering) vertex shader may display incomplete results (partial data from last frame) 
        bufferBarrier.srcAccessMask = vk::AccessFlagBits::eShaderWrite;								// Compute shader has finished writes to the buffer
        bufferBarrier.dstAccessMask = vk::AccessFlagBits::eVertexAttributeRead;						// Vertex shader invocations want to read from the buffer
        bufferBarrier.buffer = *computeBuffer;
        bufferBarrier.size = sizeof( Particle ) * NUM_PARTICLES;
        // Compute and graphics queue may have different queue families (see VulkanDevice::createLogicalDevice)
        // For the barrier to work across different queues, we need to set their family indices
        bufferBarrier.srcQueueFamilyIndex = compute.queue->getQueueFamilyIndex( );			// Required as compute and graphics queue may have different families
        bufferBarrier.dstQueueFamilyIndex = _window->graphicQueue( )->getQueueFamilyIndex( );			// Required as compute and graphics queue may have different families

        compute.commandBuffer->pipelineBarrier(
          vk::PipelineStageFlagBits::eComputeShader,
          vk::PipelineStageFlagBits::eVertexShader,
          {}, {}, { bufferBarrier }, {}
        );*/

        compute.commandBuffer->end( );
      }

      compute.fence = device->createFence( true );
    }
    setup( );
  }

  void setup( void )
  {
    std::vector<Particle> particles;
    particles.assign( NUM_PARTICLES, Particle( ) );
    const float azimuth = 256.0f * static_cast<float>( M_PI ) / particles.size( );
    const float inclination = static_cast<float>( M_PI ) / particles.size( );
    const float radius = 180.0f;
    auto c = _window->getExtent( );
    glm::vec3 center( 
      glm::vec2( c.width / 2.0f, c.height / 2.0f ) + glm::vec2( 0.0f, 4.0f ), 
      0.0f
    );
    for ( unsigned int i = 0; i < NUM_PARTICLES; ++i )
    {
      // assign starting values to particles.
      float x = radius * std::sin( inclination * i ) * std::cos( azimuth * i );
      float y = radius * std::cos( inclination * i );
      float z = radius * std::sin( inclination * i ) * std::sin( azimuth * i );

      auto &p = particles.at( i );
      p.pos = glm::vec3( x, y, z );
      p.ppos = p.pos + randVec3( ) * 10.0f; // random initial velocity
      p.damping = randFloat( 0.965f, 0.985f );
      p.color = glm::vec4( 0.2f, 0.2f, 0.2f, 1.0f );
    }
    particleBuffer->writeData( 0, sizeof( Particle ) * NUM_PARTICLES, particles.data( ) );


    comp.uSeparationRadius = 3.0f;
    comp.uCohesionRadius = 322.0f;
    comp.uAlignRadius = 250.0f;
    comp.uSeparationStrength = 0.24f;
    comp.uCohesionStrength = 0.08f;
    comp.uAlignStrength = 0.30f;
    comp.uTimeStep = 1.0f;
    comp.uBoidSpeed = 1.0f;//0.5f;
    comp.uWindowCenter = glm::vec2( c.width * 0.5f, c.height * 0.5f );
    comp.uColorRadius = 2.8f;

    computeBuffer->writeData( 0, sizeof( comp ), &comp );

    vert.uPointSize = 1.0f;
    glm::mat4 view = glm::lookAt( 
      glm::vec3( comp.uWindowCenter, -600.0f ),
      glm::vec3( comp.uWindowCenter, 0.0f ),
      glm::vec3( 0.0f, 1.0f, 0.0f)
    );
    vert.mvp = glm::perspective( glm::radians( 65.0f ),
      c.width / ( float ) c.height, 0.1f, 6000.0f );
    //vert.mvp[ 1 ][ 1 ] *= -1;

    vert.mvp *= view;

    vertexBuffer->writeData( 0, sizeof( vert ), &vert );
  }

  std::mt19937 sBase;
  std::uniform_real_distribution<float> sFloatGen;

  float randFloat( float v ) { return sFloatGen( sBase ) * v; }
  float randFloat( float a, float b ) { return sFloatGen( sBase ) * ( b - a ) + a; }

  glm::vec3 randVec3( void )
  {
    float phi = randFloat( ( float ) M_PI * 2.0f );
    float costheta = randFloat( -1.0f, 1.0f );

    float rho = std::sqrt( 1.0f - costheta * costheta );
    float x = rho * std::cos( phi );
    float y = rho * std::sin( phi );
    float z = costheta;

    return glm::vec3( x, y, z );
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

    /*std::vector<Particle> p( NUM_PARTICLES );
    particleBuffer->readData( 0, sizeof( Particle ) * NUM_PARTICLES, p.data( ) );
    
    std::vector<Particle> p2( NUM_PARTICLES );*/

    uint32_t timeout = std::numeric_limits<uint64_t>::max( );
    lava::Fence::waitForFences( { compute.fence }, true, timeout );
    lava::Fence::resetFences( { compute.fence } );

    compute.queue->submit( compute.commandBuffer, compute.fence );
    compute.queue->waitIdle( );


    /*particleBuffer->readData( 0, sizeof( Particle ) * NUM_PARTICLES, p2.data( ) );

    std::cout << glm::to_string( p.at( 0 ).pos ) << " - " <<
      glm::to_string( p2.at( 0 ).pos ) << std::endl;*/

    std::array<vk::ClearValue, 2 > clearValues;
    std::array<float, 4> ccv = { 1.0f, 1.0f, 1.0f, 1.0f };
    clearValues[ 0 ].color = vk::ClearColorValue( ccv );
    clearValues[ 1 ].depthStencil  = vk::ClearDepthStencilValue(  1.0f, 0 );

    const glm::ivec2 size = _window->swapChainImageSize( );
    auto cmd = _window->currentCommandBuffer( );
    vk::Rect2D rect;
    rect.extent.width = size.x;
    rect.extent.height = size.y;
    cmd->beginRenderPass( 
      _window->defaultRenderPass( ), _window->currentFramebuffer( ), 
      rect, clearValues, vk::SubpassContents::eInline
    );

    cmd->bindGraphicsPipeline( pipeline );

    cmd->bindDescriptorSets( vk::PipelineBindPoint::eGraphics,
      pipelineLayout, 0, { descriptorSet }, nullptr );

    cmd->setViewportScissors( _window->getExtent( ) );
    cmd->draw( NUM_PARTICLES, 1, 0, 0 );

    cmd->endRenderPass( );

    _window->frameReady( );
    //_window->requestUpdate( );
  }

private:
  VulkanWindow *_window;

  const int NUM_PARTICLES = 60000;
  const int WORK_GROUP_SIZE = 128;

  struct
  {
    std::shared_ptr<Queue> queue;
    std::shared_ptr< CommandPool > commandPool;
    CommandBufferPtr commandBuffer;
    std::shared_ptr< Fence > fence;
    std::shared_ptr< Pipeline > pipeline;
  } compute;

  std::shared_ptr< DescriptorSetLayout > descriptorSetLayout;
  std::shared_ptr< DescriptorSet > descriptorSet;
  std::shared_ptr< PipelineLayout > pipelineLayout;
  std::shared_ptr< Pipeline > pipeline;
  std::shared_ptr<Texture2D> tex;
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
  w.resize( 1000, 800 );

  w.show( );
  return 0;
}