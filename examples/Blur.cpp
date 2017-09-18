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

// Framebuffer for offscreen rendering
struct FrameBufferAttachment {
  vk::Image image;
  vk::DeviceMemory mem;
  vk::ImageView view;
};
struct OffscreenPass {
  int32_t width, height;
  vk::Framebuffer framebuffer;
  FrameBufferAttachment color, depth;
  vk::RenderPass renderPass;
  vk::Sampler sampler;
  vk::DescriptorImageInfo descriptor;
  vk::CommandBuffer commandBuffer = VK_NULL_HANDLE;
  // Semaphore used to synchronize between offscreen and final scene render pass
  vk::Semaphore semaphore = VK_NULL_HANDLE;
} offscreenPass;

#define FB_COLOR_FORMAT vk::Format::eR8G8B8A8Unorm

class MyApp : public VulkanApp
{
public:
  std::shared_ptr<Pipeline> _pipeline;
  std::shared_ptr<PipelineLayout> _pipelineLayout;
  std::shared_ptr<vk::ImageView> _textureImageView;
  std::shared_ptr<Sampler> _textureSampler;
  std::shared_ptr<DescriptorSet> _descriptorSet;
  std::shared_ptr<Texture2D> tex;

  struct
  {
    std::shared_ptr<Buffer> scene;
  } uniformBuffers;

  struct UboVS
  {
    glm::mat4 proj;
    glm::mat4 view;
    glm::mat4 model;
  } uboScene;

  struct
  {
    std::shared_ptr<lava::Pipeline> offscrenDisplay;
  };

  void prepareOffscreen( uint32_t width, uint32_t height )
  {
    offscreenPass.width = width;
    offscreenPass.height = height;

    // Find a suitable depth format
    vk::Format depthFormat = _depthFormat;
    //VkBool32 validDepthFormat = vks::tools::getSupportedDepthFormat( physicalDevice, &fbDepthFormat );
    //assert( validDepthFormat );

    // Create a separate render pass for the offscreen rendering as it may differ from the one used for scene rendering

    std::array<vk::AttachmentDescription, 2> attchmentDescriptions = {};
    // Color attachment
    attchmentDescriptions[ 0 ].format = FB_COLOR_FORMAT;
    attchmentDescriptions[ 0 ].samples = vk::SampleCountFlagBits::e1;
    attchmentDescriptions[ 0 ].loadOp = vk::AttachmentLoadOp::eClear;
    attchmentDescriptions[ 0 ].storeOp = vk::AttachmentStoreOp::eStore;
    attchmentDescriptions[ 0 ].stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
    attchmentDescriptions[ 0 ].stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
    attchmentDescriptions[ 0 ].initialLayout = vk::ImageLayout::eUndefined;
    attchmentDescriptions[ 0 ].finalLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
    // Depth attachment
    attchmentDescriptions[ 1 ].format = depthFormat;
    attchmentDescriptions[ 1 ].samples = vk::SampleCountFlagBits::e1;
    attchmentDescriptions[ 1 ].loadOp = vk::AttachmentLoadOp::eClear;
    attchmentDescriptions[ 1 ].storeOp = vk::AttachmentStoreOp::eDontCare;
    attchmentDescriptions[ 1 ].stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
    attchmentDescriptions[ 1 ].stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
    attchmentDescriptions[ 1 ].initialLayout = vk::ImageLayout::eUndefined;
    attchmentDescriptions[ 1 ].finalLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

    vk::AttachmentReference colorRef = { 0, vk::ImageLayout::eColorAttachmentOptimal };
    vk::AttachmentReference depthRef = { 1, vk::ImageLayout::eDepthStencilAttachmentOptimal };

    vk::SubpassDescription subpassDescription;
    subpassDescription.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
    subpassDescription.colorAttachmentCount = 1;
    subpassDescription.pColorAttachments = &colorRef;
    subpassDescription.pDepthStencilAttachment = &depthRef;


    // Use subpass dependencies for attachment layout transitions
    std::array<vk::SubpassDependency, 2> dependencies;

    dependencies[ 0 ].srcSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[ 0 ].dstSubpass = 0;
    dependencies[ 0 ].srcStageMask = vk::PipelineStageFlagBits::eBottomOfPipe;
    dependencies[ 0 ].dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
    dependencies[ 0 ].srcAccessMask = vk::AccessFlagBits::eMemoryRead;
    dependencies[ 0 ].dstAccessMask = vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite;
    dependencies[ 0 ].dependencyFlags = vk::DependencyFlagBits::eByRegion;

    dependencies[ 1 ].srcSubpass = 0;
    dependencies[ 1 ].dstSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[ 1 ].srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
    dependencies[ 1 ].dstStageMask = vk::PipelineStageFlagBits::eBottomOfPipe;
    dependencies[ 1 ].srcAccessMask = vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite;
    dependencies[ 1 ].dstAccessMask = vk::AccessFlagBits::eMemoryRead;
    dependencies[ 1 ].dependencyFlags = vk::DependencyFlagBits::eByRegion;

    // Create render pass
    vk::RenderPassCreateInfo renderPassInfo;
    renderPassInfo.pAttachments = attchmentDescriptions.data( );
    renderPassInfo.attachmentCount = static_cast<uint32_t>( attchmentDescriptions.size( ) );
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpassDescription;
    renderPassInfo.dependencyCount = 2;
    renderPassInfo.pDependencies = dependencies.data( );

    offscreenPass.renderPass = static_cast< vk::Device >( *_device ).createRenderPass( renderPassInfo );


    // Create sampler to sample from the color attachments
    vk::SamplerCreateInfo sampler;
    sampler.magFilter = vk::Filter::eLinear;
    sampler.minFilter = vk::Filter::eLinear;
    sampler.mipmapMode = vk::SamplerMipmapMode::eLinear;
    sampler.addressModeU = vk::SamplerAddressMode::eClampToEdge;
    sampler.addressModeV = sampler.addressModeU;
    sampler.addressModeW = sampler.addressModeU;
    sampler.mipLodBias = 0.0f;
    sampler.maxAnisotropy = 1.0f;
    sampler.minLod = 0.0f;
    sampler.maxLod = 1.0f;
    sampler.borderColor = vk::BorderColor::eFloatOpaqueWhite;

    offscreenPass.sampler = static_cast< vk::Device >( *_device ).createSampler( sampler );

    // CREATE OFFSCREEN FBO
    vk::ImageCreateInfo image;
    // Color attachment
    {
      image.imageType = vk::ImageType::e2D;
      image.format = FB_COLOR_FORMAT;
      image.extent.width = width;
      image.extent.height = height;
      image.extent.depth = 1;
      image.mipLevels = 1;
      image.arrayLayers = 1;
      image.samples = vk::SampleCountFlagBits::e1;
      image.tiling = vk::ImageTiling::eOptimal;
      // We will sample directly from the color attachment
      image.usage = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled;


      vk::ImageViewCreateInfo colorImageView;
      colorImageView.viewType = vk::ImageViewType::e2D;
      colorImageView.format = FB_COLOR_FORMAT;
      colorImageView.subresourceRange = {};
      colorImageView.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
      colorImageView.subresourceRange.baseMipLevel = 0;
      colorImageView.subresourceRange.levelCount = 1;
      colorImageView.subresourceRange.baseArrayLayer = 0;
      colorImageView.subresourceRange.layerCount = 1;

      offscreenPass.color.image = static_cast< vk::Device >( *_device ).createImage( image );
      offscreenPass.color.mem = _device->allocateImageMemory(
        offscreenPass.color.image, vk::MemoryPropertyFlagBits::eDeviceLocal );
      colorImageView.image = offscreenPass.color.image;
      offscreenPass.color.view = static_cast< vk::Device >( *_device ).createImageView( colorImageView );
    }
    // Depth stencil attachment
    {
      image.format = depthFormat;
      image.usage = vk::ImageUsageFlagBits::eDepthStencilAttachment;

      vk::ImageViewCreateInfo depthStencilView;
      depthStencilView.viewType = vk::ImageViewType::e2D;
      depthStencilView.format = depthFormat;
      depthStencilView.subresourceRange = {};
      depthStencilView.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil;
      depthStencilView.subresourceRange.baseMipLevel = 0;
      depthStencilView.subresourceRange.levelCount = 1;
      depthStencilView.subresourceRange.baseArrayLayer = 0;
      depthStencilView.subresourceRange.layerCount = 1;

      offscreenPass.depth.image = static_cast< vk::Device >( *_device ).createImage( image );
      offscreenPass.depth.mem = _device->allocateImageMemory(
        offscreenPass.depth.image, vk::MemoryPropertyFlagBits::eDeviceLocal );
      depthStencilView.image = offscreenPass.depth.image;

      offscreenPass.depth.view = static_cast< vk::Device >( *_device ).createImageView( depthStencilView );
    }

    vk::ImageView attachments[ 2 ];
    attachments[ 0 ] = offscreenPass.color.view;
    attachments[ 1 ] = offscreenPass.depth.view;

    vk::FramebufferCreateInfo fbufCreateInfo;
    fbufCreateInfo.renderPass = offscreenPass.renderPass;
    fbufCreateInfo.attachmentCount = 2;
    fbufCreateInfo.pAttachments = attachments;
    fbufCreateInfo.width = width;
    fbufCreateInfo.height = height;
    fbufCreateInfo.layers = 1;

    offscreenPass.framebuffer = static_cast< vk::Device >( *_device ).createFramebuffer( fbufCreateInfo );

    // Fill a descriptor for later use in a descriptor set 
    offscreenPass.descriptor.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
    offscreenPass.descriptor.imageView = offscreenPass.color.view;
    offscreenPass.descriptor.sampler = offscreenPass.sampler;
  }

  void setupVertexDescriptions( )
  {
    uint32_t width = _window->getWidth( );
    uint32_t height = _window->getHeight( );

    static auto startTime = std::chrono::high_resolution_clock::now( );

    auto currentTime = std::chrono::high_resolution_clock::now( );
    float time = std::chrono::duration_cast<std::chrono::milliseconds>( currentTime - startTime ).count( ) / 1000.0f;

    uboScene.model = glm::rotate( glm::mat4( 1.0f ), time * glm::radians( 90.0f ), glm::vec3( 0.0f, 0.0f, 1.0f ) );
    uboScene.view = glm::lookAt( glm::vec3( 2.0f, 2.0f, 2.0f ), glm::vec3( 0.0f, 0.0f, 0.0f ), glm::vec3( 0.0f, 0.0f, 1.0f ) );
    uboScene.proj = glm::perspective( glm::radians( 45.0f ), width / ( float ) height, 0.1f, 10.0f );
    uboScene.proj[ 1 ][ 1 ] *= -1;

    uint32_t mvpBufferSize = sizeof( uboScene );
    void* data = uniformBuffers.scene->map( 0, mvpBufferSize );
    memcpy( data, &uboScene, sizeof( uboScene ) );
    uniformBuffers.scene->unmap( );
  }

  void prepareUniformBuffers( )
  {
    uint32_t mvpBufferSize = sizeof( uboScene );
    uniformBuffers.scene = _device->createBuffer( mvpBufferSize,
      vk::BufferUsageFlagBits::eUniformBuffer,
      vk::SharingMode::eExclusive, nullptr,
      vk::MemoryPropertyFlagBits::eHostVisible |
      vk::MemoryPropertyFlagBits::eHostCoherent );

    updateUniformsScene( );
  }
  void updateUniformsScene( )
  {

  }
  MyApp(char const* title, uint32_t width, uint32_t height)
    : VulkanApp( title, width, height )
  {
    // PREPARE OFFSCREEN
    prepareOffscreen( width, height );

    std::shared_ptr<CommandPool> commandPool = _device->createCommandPool(
      vk::CommandPoolCreateFlagBits::eResetCommandBuffer, _queueFamilyIndex );
    tex = std::make_shared<Texture2D>( _device, LAVA_EXAMPLES_RESOURCES_ROUTE + 
      std::string( "/uv_checker.png" ), commandPool, _graphicsQueue );

    // init descriptor and pipeline layouts
    std::vector<DescriptorSetLayoutBinding> dslbs;
    DescriptorSetLayoutBinding mvpDescriptor( 0, vk::DescriptorType::eCombinedImageSampler, 
      vk::ShaderStageFlagBits::eFragment );
    dslbs.push_back( mvpDescriptor );
    std::shared_ptr<DescriptorSetLayout> descriptorSetLayout = _device->createDescriptorSetLayout( dslbs );

    _pipelineLayout = _device->createPipelineLayout( descriptorSetLayout, nullptr );


    std::shared_ptr<DescriptorPool> descriptorPool =
      _device->createDescriptorPool( {}, 1, { { vk::DescriptorType::eCombinedImageSampler, 1 } } );

    // Init descriptor set
    _descriptorSet = _device->allocateDescriptorSet( descriptorPool, descriptorSetLayout );
    std::vector<WriteDescriptorSet> wdss;

    WriteDescriptorSet w( _descriptorSet, 0, 0, vk::DescriptorType::eCombinedImageSampler, 1, 
      DescriptorImageInfo( 
        vk::ImageLayout::eGeneral, 
        std::make_shared<vk::ImageView>( tex->view ), 
        std::make_shared<vk::Sampler>( tex->sampler )
      ), nullptr
    );
    wdss.push_back( w );
    _device->updateDescriptorSets( wdss, {} );


    // init shaders
    std::shared_ptr<ShaderModule> vertexShaderModule =_device->createShaderModule( 
      LAVA_EXAMPLES_RESOURCES_ROUTE + std::string("/fullquad_vert.spv"), 
      vk::ShaderStageFlagBits::eVertex );
    std::shared_ptr<ShaderModule> fragmentShaderModule = _device->createShaderModule( 
      LAVA_EXAMPLES_RESOURCES_ROUTE + std::string( "/fullquad_frag.spv" ), 
      vk::ShaderStageFlagBits::eFragment );

    // init pipeline
    std::shared_ptr<PipelineCache> pipelineCache = 
      _device->createPipelineCache( 0, nullptr );
    PipelineShaderStageCreateInfo vertexStage( 
      vk::ShaderStageFlagBits::eVertex, vertexShaderModule, "main" );
    PipelineShaderStageCreateInfo fragmentStage( 
      vk::ShaderStageFlagBits::eFragment, fragmentShaderModule, "main" );
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


    _pipeline = _device->createGraphicsPipeline( pipelineCache, {}, 
    { vertexStage, fragmentStage }, vertexInput, assembly, nullptr, 
      viewport, rasterization, multisample, depthStencil, colorBlend, dynamic,
      _pipelineLayout, _renderPass );
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
    commandBuffer->bindGraphicsPipeline( _pipeline );
    commandBuffer->bindDescriptorSets( vk::PipelineBindPoint::eGraphics,
      _pipelineLayout, 0, { _descriptorSet }, nullptr );

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
    VulkanApp* app = new MyApp( "FullQuadTexture", 800, 600 );

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