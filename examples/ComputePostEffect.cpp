#include <lava/lava.h>
using namespace lava;

#include <routes.h>

class MyApp : public VulkanApp
{
public:
  std::shared_ptr<lava::DescriptorPool> descriptorPool;
  struct
  {
    std::shared_ptr<DescriptorSetLayout> descriptorSetLayout;
    std::shared_ptr<DescriptorSet> descriptorSet;
    std::shared_ptr<Pipeline> pipeline;
    std::shared_ptr<PipelineLayout> pipelineLayout;
  } graphics;

  struct
  {
    std::shared_ptr<Queue> queue;
    std::shared_ptr<CommandPool> commandPool;
    std::shared_ptr<CommandBuffer> commandBuffer;
    std::shared_ptr<Fence> fence;
    std::shared_ptr<DescriptorSetLayout> descriptorSetLayout;
    std::shared_ptr<DescriptorSet> descriptorSet;
    std::shared_ptr<Pipeline> pipeline;
    std::shared_ptr<PipelineLayout> pipelineLayout;
  } compute;

  struct TextureTarget
  {
    std::shared_ptr<lava::Image> image;
    std::shared_ptr<lava::ImageView> view;
    std::shared_ptr<lava::Sampler> sampler;
    uint32_t width;
    uint32_t height;
    vk::Format format;
  };
  TextureTarget textureComputeTarget;
  std::shared_ptr<Texture2D> textureColorMap;

  void getComputeQueue( void )
  {
    // Search for a compute queue in the array of 
    //    queue families, try to find one that support
    std::vector<uint32_t> queueFamilyIndices =
      _physicalDevice->getComputeQueueFamilyIndices( _surface );
    assert( !queueFamilyIndices.empty( ) );
    uint32_t _queueComputeFamilyIndex = queueFamilyIndices[ 0 ];

    compute.queue = _device->getQueue( _queueComputeFamilyIndex, 0 );

    std::cout << "Compute queue created" << std::endl;
  }

  void prepareCompute( void )
  {
    getComputeQueue( );

    std::vector<lava::DescriptorSetLayoutBinding> dslb;
    // Binding 0 : Sampled image (read)
    dslb.push_back( lava::DescriptorSetLayoutBinding( 0,
      vk::DescriptorType::eStorageImage, 
      vk::ShaderStageFlagBits::eCompute ) );
    // Binding 1 : Sampled image (write)
    dslb.push_back( lava::DescriptorSetLayoutBinding( 1,
      vk::DescriptorType::eStorageImage, 
      vk::ShaderStageFlagBits::eCompute ) );

    compute.descriptorSetLayout = _device->createDescriptorSetLayout( dslb );

    compute.pipelineLayout = _device->createPipelineLayout( compute.descriptorSetLayout );

    compute.descriptorSet = _device->allocateDescriptorSet( descriptorPool, 
      compute.descriptorSetLayout );

    std::vector<lava::WriteDescriptorSet> wds;
    // Binding 0 : Sampled image (read)
    wds.push_back(
      lava::WriteDescriptorSet(
        graphics.descriptorSet, 0, 0,
        vk::DescriptorType::eStorageImage, 1,
        lava::DescriptorImageInfo(
          vk::ImageLayout::eGeneral,
          std::make_shared<vk::ImageView>( textureColorMap->view ),
          std::make_shared<vk::Sampler>( textureColorMap->sampler )
        ), nullptr
      )
    );
    // Binding 1 : Sampled image (write)
    wds.push_back(
      lava::WriteDescriptorSet(
        graphics.descriptorSet, 1, 0,
        vk::DescriptorType::eStorageImage, 1,
        lava::DescriptorImageInfo(
          vk::ImageLayout::eGeneral,
          std::make_shared<vk::ImageView>( *textureComputeTarget.view ),
          std::make_shared<vk::Sampler>( *textureComputeTarget.sampler )
        ), nullptr
      )
    );

    _device->updateDescriptorSets( wds, {} );

    // Create compute shader pipelines
    std::shared_ptr<PipelineCache> pipelineCache =
      _device->createPipelineCache( 0, nullptr );

    std::shared_ptr<ShaderModule> computeShaderModule =
      _device->createShaderModule(
        LAVA_EXAMPLES_SPV_ROUTE + std::string( "posteffect_comp.spv" ),
        vk::ShaderStageFlagBits::eCompute
      );

    PipelineShaderStageCreateInfo computeStage(
      vk::ShaderStageFlagBits::eCompute, computeShaderModule );

    std::cout << "CREATE PIPELINE" << std::endl;

    compute.pipeline = _device->createComputePipeline(
      pipelineCache, {}, computeStage, compute.pipelineLayout );

    compute.fence = _device->createFence( true );

    buildComputeCommandBuffer( );
  }

  void buildComputeCommandBuffer( void )
  {		
    // Flush the queue if we're rebuilding the command buffer after 
    //  a pipeline change to ensure it's not currently in use
    compute.queue->waitIdle( );

    compute.commandPool = _device->createCommandPool( {}, compute.queue->getQueueFamilyIndex( ) );

    compute.commandBuffer = compute.commandPool->allocateCommandBuffer( );

    compute.commandBuffer->beginSimple( );
    compute.commandBuffer->bindComputePipeline( compute.pipeline );
    compute.commandBuffer->bindDescriptorSets( 
      vk::PipelineBindPoint::eCompute, compute.pipelineLayout, 0, 
      { compute.descriptorSet }, { }
    );

    compute.commandBuffer->dispatch( textureComputeTarget.width / 16, 
      textureComputeTarget.height / 16, 1 );

    compute.commandBuffer->end( );

  }
  
  void prepareTextureTarget( TextureTarget& tex,
    vk::Format format, uint32_t width, uint32_t height )
  {
    // Prepare blit target texture
    tex.width = width;
    tex.height = height;

    auto formatProps = _device->_physicalDevice->getFormatProperties( format );
    assert( formatProps.optimalTilingFeatures & vk::FormatFeatureFlagBits::eStorageImage );

    vk::ImageCreateInfo ici;
    ici.imageType = vk::ImageType::e2D;
    ici.format = format;
    ici.extent.width = width;
    ici.extent.height = height;
    ici.extent.depth = 1;
    ici.mipLevels = 1;
    ici.arrayLayers = 1;
    ici.samples = vk::SampleCountFlagBits::e1;
    ici.tiling = vk::ImageTiling::eOptimal;
    ici.usage = vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eStorage;
    ici.sharingMode = vk::SharingMode::eExclusive;


    tex.image = _device->createImage( {}, ici.imageType, ici.format,
      ici.extent, 1, 1, ici.samples, ici.tiling, ici.usage,
      ici.sharingMode, {}, ici.initialLayout, {} ); // auto allocate + bind

    std::shared_ptr<CommandPool> commandPool =
      _device->createCommandPool(
        vk::CommandPoolCreateFlagBits::eResetCommandBuffer, _queueFamilyIndex );

    std::shared_ptr<CommandBuffer> cmd = commandPool->allocateCommandBuffer( );

    cmd->beginSimple( vk::CommandBufferUsageFlagBits::eOneTimeSubmit );
    lava::utils::setImageLayout( cmd, *tex.image, 
      vk::ImageAspectFlagBits::eColor, vk::ImageLayout::eUndefined, 
      vk::ImageLayout::eGeneral );

    // Send command buffer
    cmd->end( );

    _graphicsQueue->submitAndWait( cmd );


    lava::SamplerStateDesc samplerDesc;
    samplerDesc.mipMin = samplerDesc.mipMax = 0.0f;
    samplerDesc.addressMode.u = samplerDesc.addressMode.v =
      samplerDesc.addressMode.w = lava::TextureAddressingMode::CLAMP;
    tex.sampler = _device->createSampler( samplerDesc );



    vk::ImageSubresourceRange isr;
    isr.aspectMask = vk::ImageAspectFlagBits::eColor;
    isr.baseMipLevel = 0;
    isr.levelCount = 1;
    isr.baseArrayLayer = 0;
    isr.layerCount = 1;
    tex.view = tex.image->createImageView( 
      vk::ImageViewType::e2D, format,
      vk::ComponentMapping{ vk::ComponentSwizzle::eR,
      vk::ComponentSwizzle::eG,
      vk::ComponentSwizzle::eB,
      vk::ComponentSwizzle::eA }, isr );
  }
  void setupDescriptorPool( void )
  {
    std::array<vk::DescriptorPoolSize, 3> poolSize;
    // Graphics pipeline image sampler to display
    poolSize[ 0 ] = vk::DescriptorPoolSize( 
      vk::DescriptorType::eCombinedImageSampler, 1 );
    // Compute pipeline sampler image for reading
    poolSize[ 1 ] = vk::DescriptorPoolSize( // TODO: UNUSED
      vk::DescriptorType::eSampledImage, 1 );
    // Compute pipeline storage image for image read/write
    poolSize[ 2 ] = vk::DescriptorPoolSize(
      vk::DescriptorType::eStorageImage, 2 );

    descriptorPool = _device->createDescriptorPool( {}, 2, poolSize );
  }
  void setupDescriptorSetLayout( void )
  {
    std::vector<lava::DescriptorSetLayoutBinding> dslb;
    // Binding 0 : Fragment shader image sampler
    dslb.push_back( lava::DescriptorSetLayoutBinding(
      0, vk::DescriptorType::eCombinedImageSampler,
      vk::ShaderStageFlagBits::eFragment )
    );
    graphics.descriptorSetLayout = _device->createDescriptorSetLayout( dslb );

    graphics.pipelineLayout = _device->createPipelineLayout( { graphics.descriptorSetLayout } );
  }
  void setupDescriptorSet( void )
  {
    graphics.descriptorSet = _device->allocateDescriptorSet( descriptorPool, graphics.descriptorSetLayout );
  
    std::vector<lava::WriteDescriptorSet> wds;
    wds.push_back(
      lava::WriteDescriptorSet(
        graphics.descriptorSet, 0, 0,
        vk::DescriptorType::eCombinedImageSampler, 1,
        lava::DescriptorImageInfo(
          vk::ImageLayout::eGeneral,
          std::make_shared<vk::ImageView>( *textureComputeTarget.view ),
          std::make_shared<vk::Sampler>( *textureComputeTarget.sampler )
        ), nullptr
      )
    );
    _device->updateDescriptorSets( wds, {} );
  }
  void preparePipelines( void )
  {
    // init shaders
    std::shared_ptr<ShaderModule> vertexShaderModule = _device->createShaderModule(
      LAVA_EXAMPLES_SPV_ROUTE + std::string( "fullquad_vert.spv" ),
      vk::ShaderStageFlagBits::eVertex );
    std::shared_ptr<ShaderModule> fragmentShaderModule = _device->createShaderModule(
      LAVA_EXAMPLES_SPV_ROUTE + std::string( "fullquad_frag.spv" ),
      vk::ShaderStageFlagBits::eFragment );

    // init pipeline
    std::shared_ptr<PipelineCache> pipelineCache =
      _device->createPipelineCache( 0, nullptr );
    PipelineShaderStageCreateInfo vertexStage(
      vk::ShaderStageFlagBits::eVertex, vertexShaderModule );
    PipelineShaderStageCreateInfo fragmentStage(
      vk::ShaderStageFlagBits::eFragment, fragmentShaderModule );
    PipelineVertexInputStateCreateInfo vertexInput( {}, {} );
    vk::PipelineInputAssemblyStateCreateInfo assembly( {},
      vk::PrimitiveTopology::eTriangleStrip, VK_FALSE );
    PipelineViewportStateCreateInfo viewport( { {} }, { {} } );
    vk::PipelineRasterizationStateCreateInfo rasterization( {}, false, false,
      vk::PolygonMode::eFill, vk::CullModeFlagBits::eBack,
      vk::FrontFace::eClockwise, false, 0.0f, 0.0f, 0.0f, 1.0f );
    PipelineMultisampleStateCreateInfo multisample(
      vk::SampleCountFlagBits::e1, false, 0.0f, nullptr, false, false );
    vk::StencilOpState stencilOpState( vk::StencilOp::eKeep,
      vk::StencilOp::eKeep, vk::StencilOp::eKeep, vk::CompareOp::eAlways,
      0, 0, 0 );
    vk::PipelineDepthStencilStateCreateInfo depthStencil( {}, true, true,
      vk::CompareOp::eLessOrEqual, false, false, stencilOpState,
      stencilOpState, 0.0f, 0.0f );
    vk::PipelineColorBlendAttachmentState colorBlendAttachment( false,
      vk::BlendFactor::eZero, vk::BlendFactor::eZero, vk::BlendOp::eAdd,
      vk::BlendFactor::eZero, vk::BlendFactor::eZero, vk::BlendOp::eAdd,
      vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG
      | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA );
    PipelineColorBlendStateCreateInfo colorBlend( false, vk::LogicOp::eNoOp,
      colorBlendAttachment, { 1.0f, 1.0f, 1.0f, 1.0f } );
    PipelineDynamicStateCreateInfo dynamic( { vk::DynamicState::eViewport,
      vk::DynamicState::eScissor } );

    graphics.pipeline = _device->createGraphicsPipeline( pipelineCache, {},
    { vertexStage, fragmentStage }, vertexInput, assembly, nullptr,
      viewport, rasterization, multisample, depthStencil, colorBlend, dynamic,
      graphics.pipelineLayout, _renderPass );
  }
  MyApp( char const* title, uint32_t width, uint32_t height )
    : VulkanApp( title, width, height )
  {
    std::shared_ptr<CommandPool> commandPool = _device->createCommandPool(
      vk::CommandPoolCreateFlagBits::eResetCommandBuffer, _queueFamilyIndex );
    textureColorMap = std::make_shared<Texture2D>( _device, LAVA_EXAMPLES_IMAGES_ROUTE +
      std::string( "uv_checker.png" ), commandPool, _graphicsQueue, 
      vk::Format::eR8G8B8A8Snorm,
      vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eStorage );

    prepareTextureTarget( textureComputeTarget, textureColorMap->format, width, height );

    setupDescriptorSetLayout( );
    preparePipelines( );
    setupDescriptorPool( );
    setupDescriptorSet( );
    prepareCompute( );
  }
  std::shared_ptr<CommandBuffer> commandBuffer;
  void buildCmdBuffers( void )
  {
    uint32_t width = _defaultFramebuffer->getExtent( ).width;
    uint32_t height = _defaultFramebuffer->getExtent( ).height;

    std::shared_ptr<CommandPool> commandPool =
      _device->createCommandPool(
        vk::CommandPoolCreateFlagBits::eResetCommandBuffer, _queueFamilyIndex );

    commandBuffer = commandPool->allocateCommandBuffer( );

    commandBuffer->beginSimple( );

    lava::ImageMemoryBarrier imageMemoryBarrier(
      vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eShaderRead, 
      vk::ImageLayout::eGeneral, vk::ImageLayout::eGeneral, 0, 0, 
      textureComputeTarget.image, vk::ImageSubresourceRange( 
        vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 )
    );

    commandBuffer->pipelineBarrier( vk::PipelineStageFlagBits::eComputeShader, 
      vk::PipelineStageFlagBits::eFragmentShader, {}, {}, {}, 
      { imageMemoryBarrier } );

    std::array<float, 4> ccv = { 0.2f, 0.3f, 0.3f, 1.0f };
    commandBuffer->beginRenderPass( _renderPass,
      _defaultFramebuffer->getFramebuffer( ),
      vk::Rect2D( { 0, 0 },
        _defaultFramebuffer->getExtent( ) ),
        { vk::ClearValue( ccv ), vk::ClearValue(
          vk::ClearDepthStencilValue( 1.0f, 0 ) )
        }, vk::SubpassContents::eInline );

    commandBuffer->bindGraphicsPipeline( graphics.pipeline );
    commandBuffer->bindDescriptorSets( vk::PipelineBindPoint::eGraphics,
      graphics.pipelineLayout, 0, { graphics.descriptorSet }, nullptr );
    commandBuffer->setViewportScissors( _defaultFramebuffer->getExtent( ) );
    commandBuffer->draw( 4, 1, 0, 0 );
    commandBuffer->endRenderPass( );

    commandBuffer->end( );
  }

  void doPaint( void ) override
  {
    buildCmdBuffers( );

    _graphicsQueue->submit( SubmitInfo{
      { _defaultFramebuffer->getPresentSemaphore( ) },
      { vk::PipelineStageFlagBits::eColorAttachmentOutput },
      commandBuffer,
      _renderComplete
    } );

    // Submit compute commands
    lava::Fence::waitForFences( { compute.fence }, true, UINT64_MAX );
    lava::Fence::resetFences( { compute.fence } );

    compute.queue->submit( compute.commandBuffer, compute.fence );
  }
  void keyEvent( int key, int scancode, int action, int mods )
  {
    switch ( key )
    {
    case GLFW_KEY_ESCAPE:
      getWindow( )->close( );
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

int main( int argc, char** argv )
{
  try
  {
    VulkanApp* app = new MyApp( "Compute Post Effect", 800, 600 );

    app->getWindow( )->setErrorCallback( glfwErrorCallback );

    while ( app->isRunning( ) )
    {
      // app->waitEvents( );
      app->paint( );
    }

    delete app;
  }
  catch ( std::system_error err )
  {
    std::cout << "System Error: " << err.what( ) << std::endl;
  }
  //system( "PAUSE" );
  return 0;
}