#include <lava/lava.h>
using namespace lava;

#include <routes.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <routes.h>

class MyApp : public VulkanApp
{
public:
  // Resources for the compute pipeline
  struct
  {
    std::shared_ptr<Buffer> uniformBuffer;
    std::shared_ptr<Queue> queue;
    std::shared_ptr<CommandPool> commandPool;
    std::shared_ptr<CommandBuffer> commandBuffer;
    std::shared_ptr<Fence> fence;
    std::shared_ptr<DescriptorSetLayout> descriptorSetLayout;
    std::shared_ptr<DescriptorSet> descriptorSet;
    std::shared_ptr<PipelineLayout> pipelineLayout;
    std::shared_ptr<Pipeline> pipeline;
    struct UboCompute
    {
      float iTime;
    } ubo;
  } compute;

  // Resources for the graphic pipeline
  struct
  {
    std::shared_ptr<DescriptorSetLayout> descriptorSetLayout;
    std::shared_ptr<DescriptorSet> descriptorSet;
    std::shared_ptr<Pipeline> pipeline;
    std::shared_ptr<PipelineLayout> pipelineLayout;
  } graphics;

  std::shared_ptr<Texture> textureComputeTarget;
  std::shared_ptr<vk::ImageView> _textureImageView;
  std::shared_ptr<Sampler> _textureSampler;

  void buildComputeCommandBuffer( void )
  {
    compute.commandBuffer->beginSimple( );

    compute.commandBuffer->bindComputePipeline( compute.pipeline );
    compute.commandBuffer->bindDescriptorSets( vk::PipelineBindPoint::eCompute,
      compute.pipelineLayout, 0, { compute.descriptorSet }, nullptr );

    compute.commandBuffer->dispatch( textureComputeTarget->width / 16, 
      textureComputeTarget->height / 16, 1 );

    compute.commandBuffer->end( );
  }

  void createTextureTarget( std::shared_ptr<Texture> tex, uint32_t width, 
    uint32_t height, vk::Format format )
  {
    // Get device properties for the requested texture format
    vk::FormatProperties formatProperties = _physicalDevice->getFormatProperties( format );
    // Check if requested image format supports image storage operations
    assert( formatProperties.optimalTilingFeatures & vk::FormatFeatureFlagBits::eStorageImage );

    // Prepare blit target texture
    tex->width = width;
    tex->height = height;

    vk::ImageCreateInfo imageCreateInfo;
    imageCreateInfo.imageType = vk::ImageType::e2D;
    imageCreateInfo.format = format;
    imageCreateInfo.extent = { width, height, 1 };
    imageCreateInfo.mipLevels = 1;
    imageCreateInfo.arrayLayers = 1;
    imageCreateInfo.samples = vk::SampleCountFlagBits::e1;
    imageCreateInfo.tiling = vk::ImageTiling::eOptimal;
    imageCreateInfo.initialLayout = vk::ImageLayout::eUndefined;
    // Image will be sampled in the fragment shader and used as storage target in the compute shader
    imageCreateInfo.usage = vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eSampled;
  
    vk::Device dev = static_cast< vk::Device >( *_device );
    tex->image = dev.createImage( imageCreateInfo );

    vk::MemoryRequirements memReqs = dev.getImageMemoryRequirements( tex->image );
    
    vk::MemoryAllocateInfo memAllocInfo;
    memAllocInfo.allocationSize = memReqs.size;
    memAllocInfo.memoryTypeIndex = lava::findMemoryType( 
      _physicalDevice->getMemoryProperties( ), memReqs.memoryTypeBits, 
      vk::MemoryPropertyFlagBits::eDeviceLocal );
    
    tex->deviceMemory = dev.allocateMemory( memAllocInfo );
    dev.bindImageMemory( tex->image, tex->deviceMemory, 0 );

    std::shared_ptr<CommandBuffer> layoutCmd = commandPool->allocateCommandBuffer( );
    layoutCmd->beginSimple( vk::CommandBufferUsageFlagBits::eOneTimeSubmit );

    tex->imageLayout = vk::ImageLayout::eGeneral;
    utils::setImageLayout(
      layoutCmd,
      tex->image,
      vk::ImageAspectFlagBits::eColor,
      vk::ImageLayout::eUndefined,
      tex->imageLayout
    );
    // Send command buffer
    layoutCmd->end( );

    _graphicsQueue->submitAndWait( layoutCmd );

    // Create sampler
    vk::SamplerCreateInfo sampler;
    sampler.magFilter = vk::Filter::eLinear;
    sampler.minFilter = vk::Filter::eLinear;
    sampler.mipmapMode = vk::SamplerMipmapMode::eLinear;
    sampler.addressModeU = vk::SamplerAddressMode::eClampToEdge;
    sampler.addressModeV = sampler.addressModeU;
    sampler.addressModeW = sampler.addressModeU;
    sampler.mipLodBias = 0.0f;
    sampler.maxAnisotropy = 1.0f;
    sampler.compareOp = vk::CompareOp::eNever;
    sampler.minLod = 0.0f;
    sampler.maxLod = 0.0f;
    sampler.borderColor = vk::BorderColor::eFloatOpaqueWhite;
    
    tex->sampler = dev.createSampler( sampler );

    // Create image view
    vk::ImageViewCreateInfo vci;
    vci.setViewType( vk::ImageViewType::e2D );
    vci.setFormat( format );
    vci.setComponents( {
      vk::ComponentSwizzle::eR,
      vk::ComponentSwizzle::eG,
      vk::ComponentSwizzle::eB,
      vk::ComponentSwizzle::eA
    } );
    vci.setSubresourceRange( { vk::ImageAspectFlagBits::eColor, 0,  1, 0, 1 } );
    vci.subresourceRange.levelCount = 1; // useStaging ? mipLevels : 1;
    vci.image = tex->image;

    tex->view = dev.createImageView( vci );
  }
  
  std::shared_ptr<CommandPool> commandPool;

  void updateUniformBuffers( )
  {
    static auto startTime = std::chrono::high_resolution_clock::now( );

    auto currentTime = std::chrono::high_resolution_clock::now( );
    float time = std::chrono::duration_cast<std::chrono::milliseconds>( currentTime - startTime ).count( ) / 1000.0f;

    compute.ubo.iTime = time;

    vk::Device device = static_cast<vk::Device>( *_device );

    uint32_t bufferSize = sizeof( compute.ubo );
    void* data = compute.uniformBuffer->map( 0, bufferSize );
    memcpy( data, &compute.ubo, bufferSize );
    compute.uniformBuffer->unmap( );
  }

  MyApp(char const* title, uint32_t width, uint32_t height)
    : VulkanApp( title, width, height )
  {
    commandPool = _device->createCommandPool(
      vk::CommandPoolCreateFlagBits::eResetCommandBuffer, _queueFamilyIndex );

    // PREPARE UNIFORMBUFFERS
    {
      compute.uniformBuffer = _device->createBuffer( sizeof( compute.ubo ), 
        vk::BufferUsageFlagBits::eUniformBuffer,
        vk::SharingMode::eExclusive, nullptr,
        vk::MemoryPropertyFlagBits::eHostVisible |
        vk::MemoryPropertyFlagBits::eHostCoherent );

      updateUniformBuffers( );
    }

    textureComputeTarget = Texture::create( _device );

    createTextureTarget( textureComputeTarget, width, height, vk::Format::eR8G8B8A8Uint );

    // SETUP DESCRIPTOR SET LAYOUT
    {
      std::vector<DescriptorSetLayoutBinding> dslbs =
      {
        // Binding 0: Fragment shader image sampler
        DescriptorSetLayoutBinding( 0, vk::DescriptorType::eCombinedImageSampler,
          vk::ShaderStageFlagBits::eFragment )
      };
      graphics.descriptorSetLayout = _device->createDescriptorSetLayout( dslbs );
      graphics.pipelineLayout = _device->createPipelineLayout( 
        graphics.descriptorSetLayout, nullptr );
    }

    std::shared_ptr<PipelineCache> pipelineCache = _device->createPipelineCache( 0, nullptr );
    // PREPARE PIPELINES
    {
      // init shaders
      std::shared_ptr<ShaderModule> vertexShaderModule = _device->createShaderModule(
        LAVA_EXAMPLES_RESOURCES_ROUTE + std::string( "/fullquad_vert.spv" ),
        vk::ShaderStageFlagBits::eVertex );
      std::shared_ptr<ShaderModule> fragmentShaderModule = _device->createShaderModule(
        LAVA_EXAMPLES_RESOURCES_ROUTE + std::string( "/fullquad_frag.spv" ),
        vk::ShaderStageFlagBits::eFragment );

      // init pipeline
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

    // SETUP DESCRIPTOR POOL AND SET
    std::shared_ptr<DescriptorPool> descriptorPool;
    {
      descriptorPool = _device->createDescriptorPool( { }, 1,
        {
          { vk::DescriptorType::eUniformBuffer, 2 },
          { vk::DescriptorType::eCombinedImageSampler, 1 },
          { vk::DescriptorType::eStorageImage, 1 },
        }
      );

      // Init descriptor set
      graphics.descriptorSet = _device->allocateDescriptorSet( 
        descriptorPool, graphics.descriptorSetLayout );
      std::vector<WriteDescriptorSet> wdss;

      wdss.push_back( WriteDescriptorSet(
        graphics.descriptorSet, 0, 0, vk::DescriptorType::eCombinedImageSampler, 1,
        DescriptorImageInfo(
          vk::ImageLayout::eGeneral,
          std::make_shared<vk::ImageView>( textureComputeTarget->view ),
          std::make_shared<vk::Sampler>( textureComputeTarget->sampler )
        ), nullptr
      ) );
      _device->updateDescriptorSets( wdss, {} );
    }

    // PREPARE COMPUTE
    {
      // Search for a compute queue and a present queue in the array of 
      //    queue families, try to find one that supports both
      std::vector<uint32_t> queueFamilyIndices =
        _physicalDevice->getComputeQueueFamilyIndices( _surface );
      assert( !queueFamilyIndices.empty( ) );
      uint32_t queueFamilyIndex = queueFamilyIndices[ 0 ];

      // Create a new device with the VK_KHR_SWAPCHAIN_EXTENSION enabled.
      vk::DeviceQueueCreateInfo dqci;
      dqci.setQueueFamilyIndex( queueFamilyIndex );
      const float queuePriority = 1.0f;
      dqci.setQueueCount( 1 );
      dqci.setPQueuePriorities( &queuePriority );
      std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
      queueCreateInfos.push_back( dqci );

      compute.queue = _device->getQueue( queueFamilyIndex, 0 );

      std::vector<DescriptorSetLayoutBinding> dslbs =
      {
        // Binding 0: Storage image (raytraced output)
        DescriptorSetLayoutBinding( 0, vk::DescriptorType::eStorageBuffer,
          vk::ShaderStageFlagBits::eCompute
        ),
        // Binding 1: Uniform buffer block
        DescriptorSetLayoutBinding( 1, vk::DescriptorType::eUniformBuffer,
          vk::ShaderStageFlagBits::eCompute
        )
      };

      compute.descriptorSetLayout = _device->createDescriptorSetLayout( dslbs );

      compute.pipelineLayout = _device->createPipelineLayout( 
        compute.descriptorSetLayout, nullptr );

      compute.descriptorSet = _device->allocateDescriptorSet( 
        descriptorPool, compute.descriptorSetLayout );

      std::vector<WriteDescriptorSet> wdss;
      WriteDescriptorSet w( compute.descriptorSet, 1, 0,
        vk::DescriptorType::eStorageImage, 1,
        DescriptorImageInfo(
          vk::ImageLayout::eGeneral,
          std::make_shared<vk::ImageView>( textureComputeTarget->view ),
          std::make_shared<vk::Sampler>( textureComputeTarget->sampler )
        ), nullptr
      );
      wdss.push_back( w );
      WriteDescriptorSet w2( compute.descriptorSet, 0, 0, 
        vk::DescriptorType::eUniformBuffer, 1, nullptr,
        DescriptorBufferInfo( compute.uniformBuffer, 0, sizeof( compute.ubo ) ) );
      wdss.push_back( w2 );
      _device->updateDescriptorSets( wdss, {} );

      // Create compute shader pipeline
      std::shared_ptr<ShaderModule> computeShaderModule = _device->createShaderModule(
        LAVA_EXAMPLES_RESOURCES_ROUTE + std::string( "/raytracing_comp.spv" ),
        vk::ShaderStageFlagBits::eCompute );
      PipelineShaderStageCreateInfo computeStage(
        vk::ShaderStageFlagBits::eCompute, computeShaderModule );

      compute.pipeline = _device->createComputePipeline( pipelineCache, {}, computeStage, compute.pipelineLayout );

      // Separate command pool as queue family for compute may be different than graphics
      compute.commandPool = _device->createCommandPool(
        vk::CommandPoolCreateFlagBits::eResetCommandBuffer, queueFamilyIndex );

      // Create command buffer for compute operations
      compute.commandBuffer = compute.commandPool->allocateCommandBuffer( );

      // Fence for coompute CB sync
      compute.fence = _device->createFence( true );

      // Build a single command buffer containing the compute dispatch commands
      buildComputeCommandBuffer( );
    }
  }
  void doPaint( void ) override
  {
    updateUniformBuffers( );

    // create a command pool for command buffer allocation
    std::shared_ptr<CommandPool> commandPool = _device->createCommandPool( 
      vk::CommandPoolCreateFlagBits::eResetCommandBuffer, _queueFamilyIndex );
    std::shared_ptr<CommandBuffer> commandBuffer = commandPool->allocateCommandBuffer( );

    commandBuffer->begin( );

    std::array<float, 4> ccv = { 0.2f, 0.3f, 0.3f, 1.0f };

    // Image memory barrier to make sure that compute shader writes are 
    //  finished before sampling from the texture
    ImageMemoryBarrier imb( vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eShaderRead,
      vk::ImageLayout::eGeneral, vk::ImageLayout::eGeneral,
      0, 0, std::make_shared<Image>( _device, textureComputeTarget->image ), {} );

    commandBuffer->pipelineBarrier( vk::PipelineStageFlagBits::eComputeShader,
      vk::PipelineStageFlagBits::eFragmentShader, {}, {}, {}, { imb } );

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

    // Submit command commands
    compute.fence->wait( UINT64_MAX );
    compute.fence->reset( );

    compute.queue->submit( compute.commandBuffer, compute.fence );
  }
  void keyEvent(int key, int scancode, int action, int mods)
  {
    switch (key)
    {
    case GLFW_KEY_ESCAPE:
      switch (action)
      {
      case GLFW_PRESS:
        glfwSetWindowShouldClose(getWindow()->getWindow( ), GLFW_TRUE);
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

void glfwErrorCallback(int error, const char* description)
{
  fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

int main( void )
{
  try
  {
    //if (glfwInit())
    //{
    VulkanApp* app = new MyApp( "Ray Tracer (Compute Shader)", 800, 600 );

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
  // system( "PAUSE" );
  return 0;
}