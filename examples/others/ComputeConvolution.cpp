#include <lava/lava.h>
using namespace lava;

#include <routes.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <assert.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <routes.h>

class MyApp : public VulkanApp
{
public:
  std::shared_ptr<vk::ImageView> _textureImageView;
  std::shared_ptr<Sampler> _textureSampler;

  std::shared_ptr<Texture2D> textureColorMap;
  std::shared_ptr<Texture> textureComputeTarget;

  struct
  {
    std::shared_ptr<DescriptorSetLayout> descriptorSetLayout;	// Image display shader binding layout
    std::shared_ptr<DescriptorSet> descriptorSet;	// Image display shader bindings after compute shader image manipulation
    std::shared_ptr<Pipeline> pipeline;						// Image display pipeline
    std::shared_ptr<PipelineLayout> pipelineLayout;			// Layout of the graphics pipeline
  } graphics;

  struct
  {
    std::shared_ptr<Queue> queue;								// Separate queue for compute commands (queue family may differ from the one used for graphics)
    std::shared_ptr<CommandPool> commandPool;					// Use a separate command pool (queue family may differ from the one used for graphics)
    std::shared_ptr<CommandBuffer> commandBuffer;				// Command buffer storing the dispatch commands and barriers
    std::shared_ptr<Fence> fence;								// Synchronization fence to avoid rewriting compute CB if still in use
    std::shared_ptr<DescriptorSetLayout> descriptorSetLayout;	// Compute shader binding layout
    std::shared_ptr<DescriptorSet> descriptorSet;				// Compute shader bindings
    std::shared_ptr<PipelineLayout> pipelineLayout;			// Layout of the compute pipeline
    std::vector<std::shared_ptr<Pipeline>> pipelines;			// Compute pipelines for image filters
    uint32_t pipelineIndex = 0;					// Current image filtering compute pipeline index
    uint32_t queueFamilyIndex;					// Family index of the graphics queue, used for barriers
  } compute;

  void prepareTextureTarget( uint32_t width, uint32_t height, vk::Format format )
  {
    // Get device props for the requested texture format
    vk::FormatProperties formatProps = _physicalDevice->getFormatProperties( format );
    // Check if requested image format supports image storage operations
    //assert( formatProps.optimalTilingFeatures & VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT );

    // Prepare blit target texture
    textureComputeTarget->width = width;
    textureComputeTarget->height = height;


    vk::ImageCreateInfo imageCreateInfo;
    imageCreateInfo.imageType = vk::ImageType::e2D;
    imageCreateInfo.format = format;
    imageCreateInfo.extent = { width, height, 1 };
    imageCreateInfo.mipLevels = 1;
    imageCreateInfo.arrayLayers = 1;
    imageCreateInfo.samples = vk::SampleCountFlagBits::e1;
    imageCreateInfo.tiling = vk::ImageTiling::eOptimal;
    // Image will be sampled in the fragment shader and used as storage target in the compute shader
    imageCreateInfo.usage = vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eStorage;
    // Sharing mode exclusive means that ownership of the image does not need to be explicitly transferred between the compute and graphics queue
    imageCreateInfo.sharingMode = vk::SharingMode::eExclusive;

    vk::Device device = static_cast< vk::Device >( *_device );
    textureComputeTarget->image = device.createImage( imageCreateInfo );
    textureComputeTarget->deviceMemory = _device->allocateImageMemory( 
      textureComputeTarget->image, 
      vk::MemoryPropertyFlagBits::eDeviceLocal );  // Allocate + bind


    std::shared_ptr<CommandBuffer> layoutCmd = commandPool->allocateCommandBuffer( );

    textureComputeTarget->imageLayout = vk::ImageLayout::eGeneral;

    lava::utils::setImageLayout( layoutCmd, textureComputeTarget->image, 
      vk::ImageAspectFlagBits::eColor, vk::ImageLayout::eUndefined, 
      textureComputeTarget->imageLayout );

    // Send command buffer
    layoutCmd->end( );

    _graphicsQueue->submitAndWait( layoutCmd );

    // Create default sampler
    vk::SamplerCreateInfo sci;
    sci.setMagFilter( vk::Filter::eLinear );
    sci.setMinFilter( vk::Filter::eLinear );
    sci.setMipmapMode( vk::SamplerMipmapMode::eLinear );
    sci.setAddressModeU( vk::SamplerAddressMode::eClampToEdge );
    sci.setAddressModeV( vk::SamplerAddressMode::eClampToEdge );
    sci.setAddressModeW( vk::SamplerAddressMode::eClampToEdge );
    sci.setMipLodBias( 0.0f );
    sci.setCompareOp( vk::CompareOp::eNever );
    sci.setMinLod( 0.0f );
    sci.setMaxLod( 0.0f );
    sci.setMaxAnisotropy( 1.0f );
    sci.setAnisotropyEnable( VK_TRUE );
    sci.setBorderColor( vk::BorderColor::eFloatOpaqueWhite );

    textureComputeTarget->sampler = static_cast< vk::Device >( *_device ).createSampler( sci );
  }

  std::shared_ptr<CommandPool> commandPool;
  MyApp( char const* title, uint32_t width, uint32_t height )
    : VulkanApp( title, width, height )
  {
    commandPool = _device->createCommandPool(
      vk::CommandPoolCreateFlagBits::eResetCommandBuffer, _queueFamilyIndex );
    textureColorMap = std::make_shared<Texture2D>( _device, LAVA_EXAMPLES_RESOURCES_ROUTE +
      std::string( "/uv_checker.png" ), commandPool, _graphicsQueue );

    textureComputeTarget = std::make_shared<Texture>( _device );
    prepareTextureTarget( textureColorMap->width, textureColorMap->height, vk::Format::eR8G8B8A8Unorm );

    // init descriptor and pipeline layouts
    std::vector<DescriptorSetLayoutBinding> dslbs;
    DescriptorSetLayoutBinding mvpDescriptor( 0, vk::DescriptorType::eCombinedImageSampler,
      vk::ShaderStageFlagBits::eFragment );
    dslbs.push_back( mvpDescriptor );
    std::shared_ptr<DescriptorSetLayout> descriptorSetLayout = _device->createDescriptorSetLayout( dslbs );

    graphics.pipelineLayout = _device->createPipelineLayout( descriptorSetLayout, nullptr );


    std::shared_ptr<DescriptorPool> descriptorPool =
      _device->createDescriptorPool( {}, 1, { { vk::DescriptorType::eCombinedImageSampler, 1 } } );

    // Init descriptor set
    graphics.descriptorSet = _device->allocateDescriptorSet( descriptorPool, descriptorSetLayout );
    std::vector<WriteDescriptorSet> wdss;

    WriteDescriptorSet w( graphics.descriptorSet, 0, 0, vk::DescriptorType::eCombinedImageSampler, 1,
      DescriptorImageInfo(
        vk::ImageLayout::eGeneral,
        std::make_shared<vk::ImageView>( textureColorMap->view ),
        std::make_shared<vk::Sampler>( textureColorMap->sampler )
      ), nullptr
    );
    wdss.push_back( w );
    _device->updateDescriptorSets( wdss, {} );


    // init shaders
    std::shared_ptr<ShaderModule> vertexShaderModule = _device->createShaderModule(
      LAVA_EXAMPLES_RESOURCES_ROUTE + std::string( "/fullquad_vert.spv" ),
      vk::ShaderStageFlagBits::eVertex );
    std::shared_ptr<ShaderModule> fragmentShaderModule = _device->createShaderModule(
      LAVA_EXAMPLES_RESOURCES_ROUTE + std::string( "/fullquad_frag.spv" ),
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


    // PREPARE COMPUTE

    getComputeQueue( );

    std::vector<lava::DescriptorSetLayoutBinding> cdslbs;
    // Binding 0: Sampled image (read)
    cdslbs.push_back( lava::DescriptorSetLayoutBinding( 0,
      vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eCompute ) );
    // Binding 1: Sampled image (write)
    cdslbs.push_back( lava::DescriptorSetLayoutBinding( 1,
      vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eCompute ) );

    compute.descriptorSetLayout = _device->createDescriptorSetLayout( cdslbs );
    compute.pipelineLayout = _device->createPipelineLayout( compute.descriptorSetLayout, nullptr );

    compute.descriptorSet = _device->allocateDescriptorSet( descriptorPool, compute.descriptorSetLayout );
  
    std::vector<WriteDescriptorSet> baseImageWriteDescriptorSets;
    WriteDescriptorSet w( graphics.descriptorSet, 0, 0,
      vk::DescriptorType::eStorageImage, 1,
      DescriptorImageInfo(
        vk::ImageLayout::eGeneral,
        std::make_shared<vk::ImageView>( textureColorMap->view ),
        std::make_shared<vk::Sampler>( textureColorMap->sampler )
      ), nullptr
    );
    baseImageWriteDescriptorSets.push_back( w );
    WriteDescriptorSet w2( graphics.descriptorSet, 1, 0,
      vk::DescriptorType::eStorageImage, 1,
      DescriptorImageInfo(
        vk::ImageLayout::eGeneral,
        std::make_shared<vk::ImageView>( textureComputeTarget->view ),
        std::make_shared<vk::Sampler>( textureComputeTarget->sampler )
      ), nullptr
    );
    baseImageWriteDescriptorSets.push_back( w2 );
    _device->updateDescriptorSets( wdss, {} );

    std::shared_ptr<ShaderModule> computeShaderModule = _device->createShaderModule(
      LAVA_EXAMPLES_RESOURCES_ROUTE + std::string( "/convolution.spv" ),
      vk::ShaderStageFlagBits::eCompute );
    PipelineShaderStageCreateInfo computeStage(
      vk::ShaderStageFlagBits::eCompute, computeShaderModule );

    compute.pipelines[ 0 ] = _device->createComputePipeline( 
      pipelineCache, {}, computeStage, compute.pipelineLayout );

    compute.commandPool = _device->createCommandPool( 
      vk::CommandPoolCreateFlagBits::eResetCommandBuffer, compute.queueFamilyIndex );

    compute.commandBuffer = compute.commandPool->allocateCommandBuffer( );

    // Fence for compute CB sync
    compute.fence = _device->createFence( true );

    // Building compute buffer
    // Flush the queue if we're rebuilding the command buffer after a pipeline change to ensure it's not currently in use
    compute.queue->waitIdle( );

    compute.commandBuffer->beginSimple( );

    compute.commandBuffer->bindComputePipeline( compute.pipelines[ 0 ] );
    compute.commandBuffer->bindDescriptorSets( vk::PipelineBindPoint::eCompute,
      compute.pipelineLayout, 0, { compute.descriptorSet }, nullptr );

    compute.commandBuffer->dispatch( textureComputeTarget->width / 16, textureComputeTarget->height / 16, 1 );

    compute.commandBuffer->end( );
  }
  void doPaint( void ) override
  {
    // create a command pool for command buffer allocation
    std::shared_ptr<CommandPool> commandPool = _device->createCommandPool(
      vk::CommandPoolCreateFlagBits::eResetCommandBuffer, _queueFamilyIndex );
    std::shared_ptr<CommandBuffer> commandBuffer = commandPool->allocateCommandBuffer( );

    commandBuffer->begin( );






    std::array<float, 4> ccv = { 0.2f, 0.3f, 0.3f, 1.0f };
    commandBuffer->beginRenderPass( _renderPass,
      _defaultFramebuffer->getFramebuffer( ), vk::Rect2D( { 0, 0 },
        _defaultFramebuffer->getExtent( ) ),
        { vk::ClearValue( ccv ), vk::ClearValue(
          vk::ClearDepthStencilValue( 1.0f, 0 ) ) }, vk::SubpassContents::eInline );
    commandBuffer->bindGraphicsPipeline( graphics.pipeline );
    commandBuffer->bindDescriptorSets( vk::PipelineBindPoint::eGraphics,
      graphics.pipelineLayout, 0, { graphics.descriptorSet }, nullptr );

    commandBuffer->setViewport( 0, vk::Viewport( 0.0f, 0.0f,
      ( float ) _defaultFramebuffer->getExtent( ).width,
      ( float ) _defaultFramebuffer->getExtent( ).height, 0.0f, 1.0f ) );
    commandBuffer->setScissor( 0, vk::Rect2D( { 0, 0 },
      _defaultFramebuffer->getExtent( ) ) );
    commandBuffer->draw( 4, 1, 0, 0 );
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
    case GLFW_KEY_ESCAPE:
      switch ( action )
      {
      case GLFW_PRESS:
        glfwSetWindowShouldClose( getWindow( )->getWindow( ), GLFW_TRUE );
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
    //if (glfwInit())
    //{
    VulkanApp* app = new MyApp( "Compute Convolution", 800, 600 );

    app->getWindow( )->setErrorCallback( glfwErrorCallback );

    while ( app->isRunning( ) )
    {
      app->waitEvents( );
      app->paint( );
    }

    delete app;
    //}
  }
  catch ( std::system_error err )
  {
    std::cout << "System Error: " << err.what( ) << std::endl;
  }
  system( "PAUSE" );
  return 0;
}