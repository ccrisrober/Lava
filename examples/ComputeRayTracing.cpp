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

class CustomRenderer : public VulkanWindowRenderer
{
public:
  CustomRenderer( lava::VulkanWindow *w )
    : VulkanWindowRenderer( )
    , _window( w )
  {
  }

  struct Light
  {
    glm::vec3 position;
    float ambience;
  };

  struct Sphere
  {
    glm::vec3 center;
    float radius;
    glm::vec3 color;
  };

  void prepareUniformsBuffers( void )
  {
    auto device = _window->device( );

    compute.ubo.light.ambience = 0.1f;

    compute.uniformBuffer = device->createUniformBuffer( sizeof( compute.ubo ) );
  }

  std::shared_ptr<Texture> textureComputeTarget;

  void initResources( void ) override
  {
    auto device = _window->device( );

    prepareStorageBuffers( );
    prepareUniformsBuffers( );
    auto sc = _window->getExtent( );

    textureComputeTarget = std::make_shared< Texture >( device );

    prepareTextureTarget( textureComputeTarget, sc.width, sc.height, 
      vk::Format::eR8G8B8A8Unorm );
    
    // Setup descriptor set layout
    {
      std::vector<DescriptorSetLayoutBinding> dslbs =
      {
        // Binding 0: Fragment shader image sampler
        DescriptorSetLayoutBinding(
          0, vk::DescriptorType::eCombinedImageSampler,
          vk::ShaderStageFlagBits::eFragment
        )
      };

      graphics.descriptorSetLayout = device->createDescriptorSetLayout( dslbs );
      graphics.pipelineLayout = device->createPipelineLayout( 
        graphics.descriptorSetLayout );
    }

    // Prepare pipeliens
    {
      auto vertexStage = device->createShaderPipelineShaderStage(
        LAVA_EXAMPLES_SPV_ROUTE + std::string( "fullquad_vert.spv" ),
        vk::ShaderStageFlagBits::eVertex
      );
      auto fragmentStage = device->createShaderPipelineShaderStage(
        LAVA_EXAMPLES_SPV_ROUTE + std::string( "fullquad_frag.spv" ),
        vk::ShaderStageFlagBits::eFragment
      );

      PipelineVertexInputStateCreateInfo vertexInput( {}, {} );
      vk::PipelineInputAssemblyStateCreateInfo assembly( {},
        vk::PrimitiveTopology::eTriangleList, VK_FALSE );
      PipelineViewportStateCreateInfo viewport( 1, 1 );
      vk::PipelineRasterizationStateCreateInfo rasterization( {}, true,
        false, vk::PolygonMode::eFill, vk::CullModeFlagBits::eNone,
        vk::FrontFace::eCounterClockwise, false, 0.0f, 0.0f, 0.0f, 1.0f
      );
      PipelineMultisampleStateCreateInfo multisample(
        vk::SampleCountFlagBits::e1, false, 0.0f, nullptr, false, false
      );
      vk::StencilOpState stencilOpState( vk::StencilOp::eKeep,
        vk::StencilOp::eKeep, vk::StencilOp::eKeep,
        vk::CompareOp::eAlways, 0, 0, 0
      );
      vk::PipelineDepthStencilStateCreateInfo depthStencil( {},
        true, true, vk::CompareOp::eLessOrEqual, false, false,
        stencilOpState, stencilOpState, 0.0f, 0.0f
      );
      vk::PipelineColorBlendAttachmentState colorBlendAttachment( false,
        vk::BlendFactor::eZero, vk::BlendFactor::eZero, vk::BlendOp::eAdd,
        vk::BlendFactor::eZero, vk::BlendFactor::eZero, vk::BlendOp::eAdd,
        vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
        vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA
      );
      PipelineColorBlendStateCreateInfo colorBlend( false,
        vk::LogicOp::eNoOp, colorBlendAttachment, { 1.0f, 1.0f, 1.0f, 1.0f }
      );
      PipelineDynamicStateCreateInfo dynamic( {
        vk::DynamicState::eViewport, vk::DynamicState::eScissor
      } );

      graphics.pipeline = device->createGraphicsPipeline( _window->pipelineCache, {},
      { vertexStage, fragmentStage }, vertexInput, assembly, nullptr,
        viewport, rasterization, multisample, depthStencil, colorBlend, dynamic,
        graphics.pipelineLayout, _window->defaultRenderPass( ) );
    }

    // Setup descriptor pool
    std::shared_ptr< DescriptorPool > descriptorPool;
    {
      std::array<vk::DescriptorPoolSize, 4> poolSize =
      {
        // Compute UBO
        vk::DescriptorPoolSize( vk::DescriptorType::eUniformBuffer, 1 ),
        // Graphics image samplers
        vk::DescriptorPoolSize( vk::DescriptorType::eCombinedImageSampler, 1 ),
        // Storage image for ray traced image output
        vk::DescriptorPoolSize( vk::DescriptorType::eStorageImage, 1 ),
        // Storage buffer for sphere buffer
        vk::DescriptorPoolSize( vk::DescriptorType::eStorageBuffer, 1 )
      };

      descriptorPool = device->createDescriptorPool( 2, poolSize );
    }

    // Setup descriptor set
    {
      graphics.descriptorSet = device->allocateDescriptorSet(
        descriptorPool, graphics.descriptorSetLayout );

      std::vector<WriteDescriptorSet> wdss =
      {
        WriteDescriptorSet(
          graphics.descriptorSet, 0, 0, vk::DescriptorType::eCombinedImageSampler,
          1, textureComputeTarget->descriptor, nullptr
        )
      };
      device->updateDescriptorSets( wdss, {} );
    }

    // Prepare compute
    {
      // Search for a compute queue in the array of 
      //    queue families, try to find one that support
      auto queueFamilyIndices = _window->physicalDevice( )->
        getComputeQueueFamilyIndices( );
      assert( !queueFamilyIndices.empty( ) );
      const uint32_t queueFamilyIndex = queueFamilyIndices[ 0 ];
      compute.queue = device->getQueue( queueFamilyIndex, 0 );

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
        // Binding 2: Spheres SSBO
        DescriptorSetLayoutBinding(
          2, vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eCompute
        ),
      };

      compute.descriptorSetLayout = device->createDescriptorSetLayout( dslbs );
      compute.pipelineLayout = device->createPipelineLayout( compute.descriptorSetLayout );

      compute.descriptorSet = device->allocateDescriptorSet( 
        descriptorPool, compute.descriptorSetLayout );

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
        // Binding 2: Spheres SSBO
        WriteDescriptorSet(
          compute.descriptorSet, 2, 0, vk::DescriptorType::eStorageBuffer,
          1, nullptr, DescriptorBufferInfo( compute.sphereSSBO, 0,
            spheres.size( ) * sizeof( Sphere ) )
        )
      };

      device->updateDescriptorSets( wdss, {} );

      // Create compute shader pipelines
      std::shared_ptr<ShaderModule> computeShaderModule =
        device->createShaderModule(
          LAVA_EXAMPLES_SPV_ROUTE + std::string( "raytracing_comp.spv" ),
          vk::ShaderStageFlagBits::eCompute
        );

      PipelineShaderStageCreateInfo computeStage(
        vk::ShaderStageFlagBits::eCompute, computeShaderModule );

      std::cout << "CREATE PIPELINE" << std::endl;

      compute.pipeline = device->createComputePipeline(
        _window->pipelineCache, {}, computeStage, compute.pipelineLayout );

      // Fence for compute CB sync
      compute.fence = device->createFence( true );

      // Separate command pool as queue family for compute may be different than graphics
      compute.commandPool = device->createCommandPool( {}, compute.queue->getQueueFamilyIndex( ) );

      // Create a command buffer for compute operations
      compute.cmd = compute.commandPool->allocateCommandBuffer( );

      // Build compute command buffer
      {
        // Flush the queue if we're rebuilding the command buffer after 
        //  a pipeline change to ensure it's not currently in use
        compute.queue->waitIdle( );

        compute.cmd->beginSimple( );
        compute.cmd->bindComputePipeline( compute.pipeline );
        compute.cmd->bindDescriptorSets(
          vk::PipelineBindPoint::eCompute, compute.pipelineLayout, 0,
          { compute.descriptorSet }, {}
        );

        compute.cmd->dispatch( textureComputeTarget->width / 16,
          textureComputeTarget->height / 16, 1 );

        compute.cmd->end( );
      }
    }
  }

  std::vector<Sphere> spheres;

  void prepareStorageBuffers( void )
  {
    auto device = _window->device( );

    spheres.push_back( { glm::vec3( 0.25f,   1.5f, -0.25f ),  0.3f, 
      glm::vec3( 1.0f, 0.0f, 0.0f ) } );
    spheres.push_back( { glm::vec3(  0.5f,  0.25f,  0.75f ),  0.2f, 
      glm::vec3( 0.0f, 1.0f, 0.0f ) } );
    spheres.push_back( { glm::vec3( -0.75f,  0.0f,   0.5f ),  0.2f, 
      glm::vec3( 0.0f, 0.0f, 1.0f ) } );
    spheres.push_back( { glm::vec3( -0.25f,  0.4f,   0.1f ), 0.25f, 
      glm::vec3( 0.0f, 1.0f, 1.0f ) } );
    spheres.push_back( { glm::vec3(  -1.5f, 0.15f,  0.15f ), 0.35f, 
      glm::vec3( 1.0f, 1.0f, 0.0f ) } );

    vk::DeviceSize storageBufferSize = spheres.size( ) * sizeof( Sphere );

    auto stagingBuffer = device->createBuffer( storageBufferSize,
      vk::BufferUsageFlagBits::eTransferSrc, vk::SharingMode::eExclusive, nullptr,
      vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent );
    stagingBuffer->writeData( 0, storageBufferSize, spheres.data( ) );

    compute.sphereSSBO = device->createBuffer( storageBufferSize,
      vk::BufferUsageFlagBits::eStorageBuffer |
      vk::BufferUsageFlagBits::eTransferDst, vk::SharingMode::eExclusive,
      nullptr, vk::MemoryPropertyFlagBits::eDeviceLocal );

    auto cmd = _window->graphicsCommandPool( )->allocateCommandBuffer( );
    cmd->beginSimple( );
      stagingBuffer->copy( cmd, compute.sphereSSBO, 0, 0, storageBufferSize );
    cmd->end( );

    _window->graphicQueue( )->submitAndWait( cmd );
  }
  
  void prepareTextureTarget( std::shared_ptr< Texture >& texture, 
    uint32_t w, uint32_t h, vk::Format format )
  {
    auto device = _window->device( );

    auto formatProps = _window->physicalDevice( )->getFormatProperties( format );
    assert( formatProps.optimalTilingFeatures & vk::FormatFeatureFlagBits::eStorageImage );

    texture->width = w;
    texture->height = h;

    vk::ImageCreateInfo ici;
    ici.imageType = vk::ImageType::e2D;
    ici.format = format;
    ici.extent = vk::Extent3D( texture->width, texture->height, 1 );
    ici.mipLevels = 1;
    ici.arrayLayers = 1;
    ici.samples = vk::SampleCountFlagBits::e1;
    ici.tiling = vk::ImageTiling::eOptimal;

    // Image will be sampled in the fragment shader and used as storage target in the compute shader
    ici.usage = vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eStorage;

    vk::Device dev = static_cast< vk::Device > ( *device );
    texture->image = dev.createImage( ici );
    texture->deviceMemory = device->allocateImageMemory( texture->image, 
      vk::MemoryPropertyFlagBits::eDeviceLocal ); // Allocate + bind

    auto cmd = _window->graphicsCommandPool( )->allocateCommandBuffer( );
    cmd->beginSimple( vk::CommandBufferUsageFlagBits::eOneTimeSubmit );
      texture->imageLayout = vk::ImageLayout::eGeneral;
      lava::utils::setImageLayout(
        cmd, texture->image, vk::ImageAspectFlagBits::eColor,
        vk::ImageLayout::eUndefined, texture->imageLayout
      );
    cmd->end( );
    _window->graphicQueue( )->submitAndWait( cmd );


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

    texture->sampler = dev.createSampler( sci );


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
    vci.image = texture->image;

    texture->view = dev.createImageView( vci );

    // Initialize a descriptor for later use
    texture->updateDescriptor( );
  }

  void updateUniformBuffers( void )
  {
    auto sc = _window->getExtent( );
    uint32_t width = sc.width;
    uint32_t height = sc.height;

    static auto startTime = std::chrono::high_resolution_clock::now( );

    auto currentTime = std::chrono::high_resolution_clock::now( );
    float time = std::chrono::duration_cast<std::chrono::milliseconds>(
      currentTime - startTime ).count( ) / 1000.0f;

    compute.ubo.light.position = glm::vec3( 0.0f );

    compute.uniformBuffer->writeData( 0, sizeof( compute.ubo ), &compute.ubo );
  }

  void nextFrame( void ) override
  {
    if ( Input::isKeyPressed( lava::Keyboard::Key::Esc ) )
    {
      _window->_window->close( );
    }

    updateUniformBuffers( );

    // Submit compute commands
    uint32_t timeout = std::numeric_limits<uint64_t>::max( );
    lava::Fence::waitForFences( { compute.fence }, true, timeout );
    lava::Fence::resetFences( { compute.fence } );

    compute.queue->submit( compute.cmd, compute.fence );

    std::array<vk::ClearValue, 2 > clearValues;
    std::array<float, 4> ccv = { 0.2f, 0.3f, 0.3f, 1.0f };
    clearValues[ 0 ].color = vk::ClearColorValue( ccv );
    clearValues[ 1 ].depthStencil = vk::ClearDepthStencilValue( 1.0f, 0 );

    const glm::ivec2 size = _window->swapChainImageSize( );
    auto cmd = _window->currentCommandBuffer( );

    std::shared_ptr< Device > device = _window->device( );

    ImageMemoryBarrier imb( vk::AccessFlagBits::eShaderWrite,
      vk::AccessFlagBits::eShaderRead, vk::ImageLayout::eGeneral,
      vk::ImageLayout::eGeneral, 0, 0,
      std::make_shared< lava::Image >( device, textureComputeTarget->image ),
      vk::ImageSubresourceRange( vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 )
    );

    cmd->pipelineBarrier( vk::PipelineStageFlagBits::eComputeShader,
      vk::PipelineStageFlagBits::eFragmentShader, { }, { }, { }, imb
    );

    vk::Rect2D rect;
    rect.extent.width = size.x;
    rect.extent.height = size.y;
    cmd->beginRenderPass(
      _window->defaultRenderPass( ),
      _window->currentFramebuffer( ),
      rect, clearValues, vk::SubpassContents::eInline
    );

    cmd->bindGraphicsPipeline( graphics.pipeline );
    cmd->bindDescriptorSets( vk::PipelineBindPoint::eGraphics,
      graphics.pipelineLayout, 0, { graphics.descriptorSet }, nullptr );

    cmd->setViewportScissors( _window->getExtent( ) );
    cmd->draw( 4, 1, 0, 0 );

    cmd->endRenderPass( );

    _window->frameReady( );
  }
private:
  VulkanWindow *_window;

  struct
  {
    std::shared_ptr< Pipeline > pipeline;
    std::shared_ptr< PipelineLayout > pipelineLayout;
    std::shared_ptr< DescriptorSet > descriptorSet;
    std::shared_ptr< DescriptorSetLayout > descriptorSetLayout;
  } graphics;

  struct
  {
    std::shared_ptr< Buffer > uniformBuffer;
    std::shared_ptr< Queue > queue;
    std::shared_ptr< CommandPool > commandPool;
    std::shared_ptr< CommandBuffer > cmd;
    std::shared_ptr< Fence > fence;

    std::shared_ptr< Pipeline > pipeline;
    std::shared_ptr< PipelineLayout > pipelineLayout;
    std::shared_ptr< DescriptorSet > descriptorSet;
    std::shared_ptr< DescriptorSetLayout > descriptorSetLayout;

    struct
    {
      Light light;
    } ubo;

    std::shared_ptr< Buffer > lightBuffer;

    std::shared_ptr< Buffer > sphereSSBO;
  } compute;

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
  {},
    &appInfo,
    layers.size( ),
    layers.data( ),
    extensions.size( ),
    extensions.data( )
  ) );

  CustomVkWindow w;
  w.setVulkanInstance( instance );
  w.resize( 768, 512 );

  w.show( );

  return 0;
}