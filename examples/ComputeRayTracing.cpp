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

class MyApp : public VulkanApp
{
public:
  std::shared_ptr<Texture> textureComputeTarget;

  struct
  {
    std::shared_ptr<Pipeline> pipeline;
    std::shared_ptr<PipelineLayout> pipelineLayout;
    std::shared_ptr<DescriptorSet> descriptorSet;
    std::shared_ptr<DescriptorSetLayout> descriptorSetLayout;
  } graphics;

  struct
  {
    std::shared_ptr<Buffer> uniformBuffer;
    std::shared_ptr<Queue> queue;
    std::shared_ptr<CommandPool> commandPool;
    std::shared_ptr<CommandBuffer> commandBuffer;
    std::shared_ptr<Fence> fence;
    std::shared_ptr<Pipeline> pipeline;
    std::shared_ptr<PipelineLayout> pipelineLayout;
    std::shared_ptr<DescriptorSet> descriptorSet;
    std::shared_ptr<DescriptorSetLayout> descriptorSetLayout;

    struct
    {
      float time;
    } ubo;

    struct Light
    {
      glm::vec3 color;
      glm::vec3 direction;
    } ssboLight;

    struct
    {
      std::shared_ptr<Buffer> light;
    } storageBuffers;
  } compute;

  void prepareStorageBuffers( )
  {
    // Light
    vk::DeviceSize lightBufferSize = sizeof( compute.ssboLight );

    // TODO: Move to GPU using Staging Buffer
    compute.storageBuffers.light = _device->createBuffer( lightBufferSize,
      vk::BufferUsageFlagBits::eStorageBuffer,
      vk::SharingMode::eExclusive, nullptr,
      vk::MemoryPropertyFlagBits::eHostVisible |
      vk::MemoryPropertyFlagBits::eHostCoherent );

    compute.ssboLight.color = glm::vec3( 1.0f, 1.0f, 1.0f );
    compute.ssboLight.direction = glm::normalize( glm::vec3( -1.0, 0.75, 1.0 ) );
    
    compute.storageBuffers.light->writeData( 0, lightBufferSize, &compute.ssboLight );
  }

  std::shared_ptr<DescriptorPool> descriptorPool;

  void prepareTextureTarget( std::shared_ptr<Texture>& tex, uint32_t w, 
    uint32_t h, vk::Format format, std::shared_ptr<CommandPool>& cmdPool )
  {
    auto formatProps = getPhysicalDevice( )->getFormatProperties( format );
    assert( formatProps.optimalTilingFeatures & vk::FormatFeatureFlagBits::eStorageImage );

    // Prepare blit texture
    tex->width = w;
    tex->height = h;

    vk::ImageCreateInfo ici;
    ici.imageType = vk::ImageType::e2D;
    ici.format = format;
    ici.extent = vk::Extent3D( tex->width, tex->height, 1 );
    ici.mipLevels = 1;
    ici.arrayLayers = 1;
    ici.samples = vk::SampleCountFlagBits::e1;
    ici.tiling = vk::ImageTiling::eOptimal;
    // Image will be sampled in the fragment shader and used as storage target in the compute shader
    ici.usage = vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eStorage;

    vk::Device dev = static_cast<vk::Device>( *_device );

    tex->image = dev.createImage( ici );

    tex->deviceMemory = _device->allocateImageMemory( tex->image,
      vk::MemoryPropertyFlagBits::eDeviceLocal );  // Allocate + bind

    std::shared_ptr<CommandBuffer> layoutCmd = cmdPool->allocateCommandBuffer( );
    layoutCmd->beginSimple( vk::CommandBufferUsageFlagBits::eOneTimeSubmit );

    tex->imageLayout = vk::ImageLayout::eGeneral;
    lava::utils::setImageLayout( 
      layoutCmd, tex->image, vk::ImageAspectFlagBits::eColor, 
      vk::ImageLayout::eUndefined, tex->imageLayout
    );

    // Send command buffer
    layoutCmd->end( );

    _graphicsQueue->submitAndWait( layoutCmd ); // TODO: Use another kind of quee

    // Create sampler
    vk::SamplerCreateInfo sci;
    sci.setMagFilter( vk::Filter::eLinear );
    sci.setMinFilter( vk::Filter::eLinear );
    sci.setMipmapMode( vk::SamplerMipmapMode::eLinear );
    sci.setAddressModeU( vk::SamplerAddressMode::eClampToBorder );
    sci.setAddressModeV( vk::SamplerAddressMode::eClampToBorder );
    sci.setAddressModeW( vk::SamplerAddressMode::eClampToBorder );
    sci.setMipLodBias( 0.0f );
    sci.setCompareOp( vk::CompareOp::eNever );
    sci.setMinLod( 0.0f );
    sci.setMaxLod( 0.0f );
    sci.setMaxAnisotropy( 1.0f );
    sci.setAnisotropyEnable( VK_TRUE );
    sci.setBorderColor( vk::BorderColor::eFloatOpaqueWhite );

    tex->sampler = dev.createSampler( sci );


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
    vci.setSubresourceRange( { vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 } );
    vci.subresourceRange.levelCount = 1;
    vci.image = tex->image;

    tex->view = dev.createImageView( vci );

    // Initialize a descriptor for later use
    tex->updateDescriptor( );
  }

  void setupDescriptorPool( void )
  {
    std::array<vk::DescriptorPoolSize, 4> poolSize =
    {
      // Compute UBO
      vk::DescriptorPoolSize( vk::DescriptorType::eUniformBuffer, 1 ),
      // Graphics image samplers
      vk::DescriptorPoolSize( vk::DescriptorType::eCombinedImageSampler, 1 ),
      // Storage image for ray traced image output
      vk::DescriptorPoolSize( vk::DescriptorType::eStorageImage, 1 ),
      // Storage buffer for light buffer
      vk::DescriptorPoolSize( vk::DescriptorType::eStorageBuffer, 1 )
    };

    descriptorPool = _device->createDescriptorPool( { }, 2, poolSize );
  }

  void setupDescriptorPoolSetLayout( void )
  {
    std::vector<DescriptorSetLayoutBinding> dslbs =
    {
      // Binding 0: Fragment shader image sampler
      DescriptorSetLayoutBinding(
        0,
        vk::DescriptorType::eCombinedImageSampler,
        vk::ShaderStageFlagBits::eFragment
      )
    };

    graphics.descriptorSetLayout = _device->createDescriptorSetLayout( dslbs );
    graphics.pipelineLayout = _device->createPipelineLayout( graphics.descriptorSetLayout );
  }

  void setupDescriptorSet( void )
  {
    graphics.descriptorSet = _device->allocateDescriptorSet(
      descriptorPool, graphics.descriptorSetLayout );

    std::vector<WriteDescriptorSet> wdss =
    {
      WriteDescriptorSet(
        graphics.descriptorSet, 0, 0, vk::DescriptorType::eCombinedImageSampler,
        1, textureComputeTarget->descriptor, nullptr
      )
    };
    _device->updateDescriptorSets( wdss, { } );
  }

  void preparePipelines( void )
  {
    // init pipeline
    PipelineShaderStageCreateInfo vertexStage = _device->createShaderPipelineShaderStage(
      LAVA_EXAMPLES_SPV_ROUTE + std::string( "fullquad_vert.spv" ),
      vk::ShaderStageFlagBits::eVertex
    );
    PipelineShaderStageCreateInfo fragmentStage = _device->createShaderPipelineShaderStage(
      LAVA_EXAMPLES_SPV_ROUTE + std::string( "fullquad_frag.spv" ),
      vk::ShaderStageFlagBits::eFragment
    );
    PipelineVertexInputStateCreateInfo vertexInput( { }, { } );
    vk::PipelineInputAssemblyStateCreateInfo assembly( { },
      vk::PrimitiveTopology::eTriangleStrip, VK_FALSE );
    PipelineViewportStateCreateInfo viewport( 1, 1 );
    vk::PipelineRasterizationStateCreateInfo rasterization( { }, false, false,
      vk::PolygonMode::eFill, vk::CullModeFlagBits::eBack,
      vk::FrontFace::eClockwise, false, 0.0f, 0.0f, 0.0f, 1.0f );
    PipelineMultisampleStateCreateInfo multisample(
      vk::SampleCountFlagBits::e1, false, 0.0f, nullptr, false, false );
    vk::StencilOpState stencilOpState( vk::StencilOp::eKeep,
      vk::StencilOp::eKeep, vk::StencilOp::eKeep, vk::CompareOp::eAlways,
      0, 0, 0 );
    vk::PipelineDepthStencilStateCreateInfo depthStencil( { }, true, true,
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


    graphics.pipeline = _device->createGraphicsPipeline( pipelineCache, { },
      { vertexStage, fragmentStage }, vertexInput, assembly, nullptr,
      viewport, rasterization, multisample, depthStencil, colorBlend, dynamic,
      graphics.pipelineLayout, _renderPass );
  }

  void prepareCompute( void )
  {
    // Search for a compute queue in the array of 
    //    queue families, try to find one that support
    std::vector<uint32_t> queueFamilyIndices =
      getPhysicalDevice( )->getComputeQueueFamilyIndices( _surface );
    assert( !queueFamilyIndices.empty( ) );
    uint32_t _queueComputeFamilyIndex = queueFamilyIndices[ 0 ];

    compute.queue = _device->getQueue( _queueComputeFamilyIndex, 0 );

    std::cout << "Compute queue created" << std::endl;

    std::vector<DescriptorSetLayoutBinding> dslbs =
    {
      // Binding 0: Storage image (raytracer output)
      DescriptorSetLayoutBinding(
        0, vk::DescriptorType::eStorageImage, vk::ShaderStageFlagBits::eCompute
      ),
      // Binding 1: Uniform buffer block
      DescriptorSetLayoutBinding(
        1, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eCompute
      ),
      // Binding 2: Light SSBO
      DescriptorSetLayoutBinding(
        2, vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eCompute
      ),
    };

    compute.descriptorSetLayout = _device->createDescriptorSetLayout( dslbs );
    compute.pipelineLayout = _device->createPipelineLayout( compute.descriptorSetLayout );

    compute.descriptorSet = _device->allocateDescriptorSet( descriptorPool, compute.descriptorSetLayout );

    std::vector<WriteDescriptorSet> wdss =
    {
      // Binding 0: Storage image (raytracer output)
      WriteDescriptorSet(
        compute.descriptorSet, 0, 0, vk::DescriptorType::eStorageImage,
        1, textureComputeTarget->descriptor, nullptr
      ),
      // Binding 1: Uniform buffer block
      WriteDescriptorSet(
        compute.descriptorSet, 1, 0, vk::DescriptorType::eUniformBuffer,
        1, nullptr, DescriptorBufferInfo( compute.uniformBuffer, 0,
          sizeof( compute.ubo ) )
      ),
      // Binding 2: Light SSBO
      WriteDescriptorSet(
        compute.descriptorSet, 2, 0, vk::DescriptorType::eStorageBuffer,
        1, nullptr, DescriptorBufferInfo( compute.storageBuffers.light, 0,
          sizeof( compute.ssboLight ) )
      )
    };

    _device->updateDescriptorSets( wdss, { } );

    // Create compute shader pipelines
    std::shared_ptr<ShaderModule> computeShaderModule =
      _device->createShaderModule(
        LAVA_EXAMPLES_SPV_ROUTE + std::string( "raytracing_comp.spv" ),
        vk::ShaderStageFlagBits::eCompute
      );

    PipelineShaderStageCreateInfo computeStage(
      vk::ShaderStageFlagBits::eCompute, computeShaderModule );

    std::cout << "CREATE PIPELINE" << std::endl;

    compute.pipeline = _device->createComputePipeline(
      pipelineCache, { }, computeStage, compute.pipelineLayout );

    // Fence for compute CB sync
    compute.fence = _device->createFence( true );

    // Separate command pool as queue family for compute may be different than graphics
    compute.commandPool = _device->createCommandPool( { }, compute.queue->getQueueFamilyIndex( ) );

    // Create a command buffer for compute operations
    compute.commandBuffer = compute.commandPool->allocateCommandBuffer( );
    buildComputeCommandBuffer( );
  }

  void buildComputeCommandBuffer( void )
  {
    // Flush the queue if we're rebuilding the command buffer after 
    //  a pipeline change to ensure it's not currently in use
    compute.queue->waitIdle( );

    compute.commandBuffer->beginSimple( );
    compute.commandBuffer->bindComputePipeline( compute.pipeline );
    compute.commandBuffer->bindDescriptorSets(
      vk::PipelineBindPoint::eCompute, compute.pipelineLayout, 0,
      { compute.descriptorSet }, { }
    );

    compute.commandBuffer->dispatch( textureComputeTarget->width / 16,
      textureComputeTarget->height / 16, 1 );

    compute.commandBuffer->end( );
  }

  void updateUniformBuffers( void )
  {
    uint32_t width = _window->getWidth( );
    uint32_t height = _window->getHeight( );

    static auto startTime = std::chrono::high_resolution_clock::now( );

    auto currentTime = std::chrono::high_resolution_clock::now( );
    float time = std::chrono::duration_cast<std::chrono::milliseconds>(
      currentTime - startTime ).count( ) / 1000.0f;

    compute.ubo.time = time;

    compute.uniformBuffer->writeData( 0, sizeof( compute.ubo ), &compute.ubo );

    compute.ssboLight.direction = glm::normalize(
      glm::vec3(
        -1.0f + 4.0f * cos( time ),
        4.75f,
        1.0f + 4.0f * sin( time ) ) );
      compute.storageBuffers.light->writeData( 0, sizeof( compute.ssboLight ), &compute.ssboLight );
  }

  std::shared_ptr<CommandPool> commandPool;
  std::shared_ptr<CommandBuffer> commandBuffer;

  void buildCommandBuffers( void )
  {
    commandBuffer = commandPool->allocateCommandBuffer( );
    commandBuffer->beginSimple( );

    ImageMemoryBarrier imb = ImageMemoryBarrier( vk::AccessFlagBits::eShaderWrite, 
      vk::AccessFlagBits::eShaderRead, vk::ImageLayout::eGeneral, 
      vk::ImageLayout::eGeneral, 0, 0,
      std::make_shared< lava::Image >( _device, textureComputeTarget->image ), 
      vk::ImageSubresourceRange( vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 )
    );

    commandBuffer->pipelineBarrier( vk::PipelineStageFlagBits::eComputeShader, 
      vk::PipelineStageFlagBits::eFragmentShader, { }, { }, { }, imb
    );

    std::array<float, 4> ccv = { 0.2f, 0.3f, 0.3f, 1.0f };
    commandBuffer->beginRenderPass( _renderPass,
      _defaultFramebuffer->getFramebuffer( ), vk::Rect2D( { 0, 0 },
        _defaultFramebuffer->getExtent( ) ),
        { vk::ClearValue( ccv ), vk::ClearValue(
          vk::ClearDepthStencilValue( 1.0f, 0 ) ) }, vk::SubpassContents::eInline );
    commandBuffer->bindGraphicsPipeline( graphics.pipeline );
    commandBuffer->bindDescriptorSets( vk::PipelineBindPoint::eGraphics,
      graphics.pipelineLayout, 0, { graphics.descriptorSet }, nullptr );

    commandBuffer->setViewportScissors( _defaultFramebuffer->getExtent( ) );

    commandBuffer->draw( 4, 1, 0, 0 );
    commandBuffer->endRenderPass( );

    commandBuffer->end( );
  }
  void prepareUniformsBuffers( void )
  {
    compute.uniformBuffer = _device->createUniformBuffer( sizeof( compute.ubo ) );
  }

  void setupDescriptorSetLayout( void )
  {
    std::vector<DescriptorSetLayoutBinding> dslbs =
    {
      // Binding 0: Fragment shader image sampler
      DescriptorSetLayoutBinding(
        0, vk::DescriptorType::eCombinedImageSampler,
        vk::ShaderStageFlagBits::eFragment
      )
    };

    graphics.descriptorSetLayout = _device->createDescriptorSetLayout( dslbs );
    graphics.pipelineLayout = _device->createPipelineLayout( graphics.descriptorSetLayout );
  }

  MyApp( char const* title, uint32_t width, uint32_t height )
    : VulkanApp( title, width, height )
  {
    commandPool = _device->createCommandPool(
      vk::CommandPoolCreateFlagBits::eResetCommandBuffer, _queueFamilyIndex );

    textureComputeTarget = std::make_shared<Texture>( _device );
    prepareStorageBuffers( );
    prepareUniformsBuffers( );
    prepareTextureTarget( textureComputeTarget, width, height, 
      vk::Format::eR8G8B8A8Unorm, commandPool );
    setupDescriptorSetLayout( );
    preparePipelines( );
    setupDescriptorPool( );
    setupDescriptorSet( );
    prepareCompute( );
    buildCommandBuffers( );
  }
  void doPaint( void ) override
  {
    updateUniformBuffers( );
    buildCommandBuffers( );
    _graphicsQueue->submit( SubmitInfo{
      { _defaultFramebuffer->getPresentSemaphore( ) },
      { vk::PipelineStageFlagBits::eColorAttachmentOutput },
      commandBuffer,
      _renderComplete
    } );

    // Submit compute commands
    uint32_t timeout = std::numeric_limits<uint64_t>::max();
    lava::Fence::waitForFences( { compute.fence }, true, timeout );
    lava::Fence::resetFences( { compute.fence } );

    compute.queue->submit( compute.commandBuffer, compute.fence );
  }
  void keyEvent( int key, int scancode, int action, int mods )
  {
    switch ( key )
    {
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
  virtual void doResize( uint32_t width, uint32_t height ) override
  {
    prepareTextureTarget( textureComputeTarget, width, height,
      vk::Format::eR8G8B8A8Unorm, commandPool );

    compute.commandBuffer.reset( );
    compute.commandBuffer = compute.commandPool->allocateCommandBuffer( );
    buildComputeCommandBuffer( );
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
    VulkanApp* app = new MyApp( "Compute Raytracing", 736, 512 );

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