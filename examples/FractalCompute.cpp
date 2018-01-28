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
    _window->setWindowTitle( "Cube with computed fractal" );
  }

  struct Vertex
  {
    glm::vec3 pos;
    glm::vec2 texCoord;
  };

  const std::vector<Vertex> vertices =
  {
    { { -0.5f, -0.5f,  0.5f },{ 0.0f, 0.0f } },
    { { 0.5f, -0.5f,  0.5f },{ 1.0f, 0.0f } },
    { { -0.5f,  0.5f,  0.5f },{ 0.0f, 1.0f } },
    { { 0.5f,  0.5f,  0.5f },{ 1.0f, 1.0f } },

    { { -0.5f, -0.5f, -0.5f },{ 0.0f, 0.0f } },
    { { 0.5f, -0.5f, -0.5f },{ 1.0f, 0.0f } },
    { { -0.5f,  0.5f, -0.5f },{ 0.0f, 1.0f } },
    { { 0.5f,  0.5f, -0.5f },{ 1.0f, 1.0f } },

    { { 0.5f, -0.5f, -0.5f },{ 0.0f, 0.0f } },
    { { 0.5f, -0.5f,  0.5f },{ 1.0f, 0.0f } },
    { { 0.5f,  0.5f, -0.5f },{ 0.0f, 1.0f } },
    { { 0.5f,  0.5f,  0.5f },{ 1.0f, 1.0f } },

    { { -0.5f, -0.5f, -0.5f },{ 0.0f, 0.0f } },
    { { -0.5f, -0.5f,  0.5f },{ 1.0f, 0.0f } },
    { { -0.5f,  0.5f, -0.5f },{ 0.0f, 1.0f } },
    { { -0.5f,  0.5f,  0.5f },{ 1.0f, 1.0f } },

    { { -0.5f,  0.5f, -0.5f },{ 0.0f, 0.0f } },
    { { -0.5f,  0.5f,  0.5f },{ 1.0f, 0.0f } },
    { { 0.5f,  0.5f, -0.5f },{ 0.0f, 1.0f } },
    { { 0.5f,  0.5f,  0.5f },{ 1.0f, 1.0f } },

    { { -0.5f, -0.5f, -0.5f },{ 0.0f, 0.0f } },
    { { -0.5f, -0.5f,  0.5f },{ 1.0f, 0.0f } },
    { { 0.5f, -0.5f, -0.5f },{ 0.0f, 1.0f } },
    { { 0.5f, -0.5f,  0.5f },{ 1.0f, 1.0f } }
  };
  const std::vector<uint16_t> indices =
  {
    0,  1,  2,     1,  3,  2,
    4,  6,  5,     5,  6,  7,
    8, 10,  9,     9, 10, 11,
    12, 13, 14,    13, 15, 14,
    16, 17, 18,    17, 19, 18,
    20, 22, 21,    21, 22, 23,
  };

  void prepareTextureTarget( std::shared_ptr<Texture>& tex, uint32_t w,
    uint32_t h, vk::Format format, std::shared_ptr<CommandPool>& cmdPool )
  {
    auto device = _window->device( );

    auto formatProps = _window->physicalDevice( )->getFormatProperties( format );
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

    vk::Device dev = static_cast<vk::Device>( *device );

    tex->image = dev.createImage( ici );

    tex->deviceMemory = device->allocateImageMemory( tex->image,
      vk::MemoryPropertyFlagBits::eDeviceLocal );  // Allocate + bind

    std::shared_ptr<CommandBuffer> layoutCmd = cmdPool->allocateCommandBuffer( );
    layoutCmd->begin( vk::CommandBufferUsageFlagBits::eOneTimeSubmit );

    tex->imageLayout = vk::ImageLayout::eGeneral;
    lava::utils::setImageLayout(
      layoutCmd, tex->image, vk::ImageAspectFlagBits::eColor,
      vk::ImageLayout::eUndefined, tex->imageLayout
    );

    // Send command buffer
    layoutCmd->end( );

    _window->gfxQueue( )->submitAndWait( layoutCmd ); // TODO: Use another kind of quee

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

  void prepareCompute( void )
  {
    // Search for a compute queue in the array of 
    //    queue families, try to find one that support
    auto queueFamilyIndices = _window->physicalDevice( )->getComputeQueueFamilyIndices( );
    assert( !queueFamilyIndices.empty( ) );
    const uint32_t queueComputeFamilyIndex = queueFamilyIndices[ 0 ];

    auto device = _window->device( );

    compute.queue = device->getQueue( queueComputeFamilyIndex, 0 );

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
      )
    };

    compute.descriptorSetLayout = device->createDescriptorSetLayout( dslbs );
    compute.pipelineLayout = device->createPipelineLayout( compute.descriptorSetLayout );

    compute.descriptorSet = device->allocateDescriptorSet( descriptorPool, compute.descriptorSetLayout );

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
      )
    };

    device->updateDescriptorSets( wdss, {} );

    // Create compute shader pipelines
    std::cout << "Loading shader... ";
    auto computeStage = device->createShaderPipelineShaderStage(
      LAVA_EXAMPLES_SPV_ROUTE + std::string( "fractalCompute_comp.spv" ),
      vk::ShaderStageFlagBits::eCompute
    );

    std::cout << "CREATE PIPELINE" << std::endl;

    compute.pipeline = device->createComputePipeline(
      _window->pipelineCache, {}, computeStage, compute.pipelineLayout );

    // Fence for compute CB sync
    compute.fence = device->createFence( true );

    // Separate command pool as queue family for compute may be different than graphics
    compute.commandPool = device->createCommandPool( {}, queueComputeFamilyIndex );

    // Create a command buffer for compute operations
    compute.commandBuffer = compute.commandPool->allocateCommandBuffer( );
    
    // Build compute command buffer
    {
      // Flush the queue if we're rebuilding the command buffer after 
      //  a pipeline change to ensure it's not currently in use
      compute.queue->waitIdle( );

      compute.commandBuffer->begin( );
      compute.commandBuffer->bindComputePipeline( compute.pipeline );
      compute.commandBuffer->bindDescriptorSets(
        vk::PipelineBindPoint::eCompute, compute.pipelineLayout, 0,
        { compute.descriptorSet }, {}
      );

      compute.commandBuffer->dispatch( textureComputeTarget->width / 2,
        textureComputeTarget->height / 2, 1 );

      compute.commandBuffer->end( );
    }
  }


  void initResources( void ) override
  {
    auto device = _window->device( );

    // Vertex buffer
    {
      uint32_t vertexBufferSize = vertices.size( ) * sizeof( Vertex );
      auto stagingBuffer = device->createBuffer( vertexBufferSize,
        vk::BufferUsageFlagBits::eTransferSrc,
        vk::MemoryPropertyFlagBits::eHostVisible | 
        vk::MemoryPropertyFlagBits::eHostCoherent );
      stagingBuffer->writeData( 0, vertexBufferSize, vertices.data( ) );

      graphics.vertexBuffer = device->createBuffer( vertexBufferSize,
        vk::BufferUsageFlagBits::eVertexBuffer | 
        vk::BufferUsageFlagBits::eTransferDst,
        vk::MemoryPropertyFlagBits::eDeviceLocal );

      auto cmd = _window->gfxCommandPool( )->allocateCommandBuffer( );
      cmd->begin( );
        stagingBuffer->copy( cmd, graphics.vertexBuffer, 0, 0, vertexBufferSize );
      cmd->end( );

      _window->gfxQueue( )->submitAndWait( cmd );
    }

    // Index buffer
    {
      uint32_t indexBufferSize = indices.size( ) * sizeof( uint32_t );

      auto stagingBuffer = device->createBuffer( indexBufferSize,
        vk::BufferUsageFlagBits::eTransferSrc,
        vk::MemoryPropertyFlagBits::eHostVisible |
        vk::MemoryPropertyFlagBits::eHostCoherent );
      stagingBuffer->writeData( 0, indexBufferSize, indices.data( ) );

      graphics.indexBuffer = device->createBuffer( indexBufferSize,
        vk::BufferUsageFlagBits::eIndexBuffer |
        vk::BufferUsageFlagBits::eTransferDst,
        vk::MemoryPropertyFlagBits::eDeviceLocal );

      auto cmd = _window->gfxCommandPool( )->allocateCommandBuffer( );
      cmd->begin( );
        stagingBuffer->copy( cmd, graphics.indexBuffer, 0, 0, indexBufferSize );
      cmd->end( );

      _window->gfxQueue( )->submitAndWait( cmd );
    }
  
    // Uniform buffers
    {
      graphics.mvpBuffer = device->createUniformBuffer( sizeof( graphics.uboVS ) );

      compute.uniformBuffer = device->createUniformBuffer( sizeof( compute.ubo ) );
    }

    textureComputeTarget = std::make_shared<Texture>( device );
    prepareTextureTarget( textureComputeTarget, compute.ubo.width, compute.ubo.height,
      vk::Format::eR8G8B8A8Unorm, _window->gfxCommandPool( ) );

    auto vertexStage = device->createShaderPipelineShaderStage(
      LAVA_EXAMPLES_SPV_ROUTE + std::string( "cubeUV_vert.spv" ),
      vk::ShaderStageFlagBits::eVertex
    );
    auto fragmentStage = device->createShaderPipelineShaderStage(
      LAVA_EXAMPLES_SPV_ROUTE + std::string( "cubeUV_frag.spv" ),
      vk::ShaderStageFlagBits::eFragment
    );

    std::vector<DescriptorSetLayoutBinding> dslbs =
    {
      DescriptorSetLayoutBinding( 0, vk::DescriptorType::eUniformBuffer,
        vk::ShaderStageFlagBits::eVertex
      ),
      DescriptorSetLayoutBinding( 1, vk::DescriptorType::eCombinedImageSampler,
        vk::ShaderStageFlagBits::eFragment
      )
    };

    graphics.descriptorSetLayout = device->createDescriptorSetLayout( dslbs );

    graphics.pipelineLayout = device->createPipelineLayout( graphics.descriptorSetLayout, nullptr );

    vk::VertexInputBindingDescription binding( 0, sizeof( Vertex ), vk::VertexInputRate::eVertex );

    PipelineVertexInputStateCreateInfo vertexInput( binding, {
      vk::VertexInputAttributeDescription( 
        0, 0, vk::Format::eR32G32B32Sfloat, offsetof( Vertex, pos )
      ),
      vk::VertexInputAttributeDescription( 
        1, 0, vk::Format::eR32G32Sfloat, offsetof( Vertex, texCoord )
      )
    } );

    vk::PipelineInputAssemblyStateCreateInfo assembly( { },
      vk::PrimitiveTopology::eTriangleList, VK_FALSE );
    PipelineViewportStateCreateInfo viewport( 1, 1 );
    vk::PipelineRasterizationStateCreateInfo rasterization( {}, true,
      false, vk::PolygonMode::eFill, vk::CullModeFlagBits::eBack,
      vk::FrontFace::eCounterClockwise, false, 0.0f, 0.0f, 0.0f, 1.0f );
    PipelineMultisampleStateCreateInfo multisample( vk::SampleCountFlagBits::e1, 
      false, 0.0f, nullptr, false, false );
    vk::StencilOpState stencilOpState( vk::StencilOp::eKeep, 
      vk::StencilOp::eKeep, vk::StencilOp::eKeep, vk::CompareOp::eAlways, 
      0, 0, 0 );
    vk::PipelineDepthStencilStateCreateInfo depthStencil( {}, true, true, 
      vk::CompareOp::eLessOrEqual, false, false, stencilOpState, 
      stencilOpState, 0.0f, 0.0f );
    ;
    PipelineColorBlendStateCreateInfo colorBlend( false, vk::LogicOp::eNoOp, 
      vk::PipelineColorBlendAttachmentState( 
        false, vk::BlendFactor::eZero, vk::BlendFactor::eZero, 
        vk::BlendOp::eAdd, vk::BlendFactor::eZero, vk::BlendFactor::eZero, 
        vk::BlendOp::eAdd, vk::ColorComponentFlagBits::eR | 
        vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | 
        vk::ColorComponentFlagBits::eA
      ), { 1.0f, 1.0f, 1.0f, 1.0f }
    );
    PipelineDynamicStateCreateInfo dynamic( { 
      vk::DynamicState::eViewport, vk::DynamicState::eScissor
    } );

    graphics.pipeline = device->createGraphicsPipeline( _window->pipelineCache, { },
      { vertexStage, fragmentStage }, vertexInput, assembly, nullptr,
      viewport, rasterization, multisample, depthStencil, colorBlend, dynamic,
      graphics.pipelineLayout, _window->defaultRenderPass( )
    );

    std::array<vk::DescriptorPoolSize, 3> poolSize =
    {
      // Compute UBO and MVP
      vk::DescriptorPoolSize( vk::DescriptorType::eUniformBuffer, 2 ),
      // Graphics image samplers
      vk::DescriptorPoolSize( vk::DescriptorType::eCombinedImageSampler, 1 ),
      // Storage image for ray traced image output
      vk::DescriptorPoolSize( vk::DescriptorType::eStorageImage, 1 )
    };
    descriptorPool = device->createDescriptorPool( 2, poolSize );

    // Init descriptor set
    graphics.descriptorSet = device->allocateDescriptorSet( descriptorPool, graphics.descriptorSetLayout );

    std::vector<WriteDescriptorSet> wdss =
    {
      WriteDescriptorSet( 
        graphics.descriptorSet, 0, 0, vk::DescriptorType::eUniformBuffer,
        1, nullptr, DescriptorBufferInfo( graphics.mvpBuffer, 0, sizeof( graphics.uboVS ) )
      ),
      WriteDescriptorSet(
        graphics.descriptorSet, 1, 0, vk::DescriptorType::eCombinedImageSampler, 1,
        textureComputeTarget->descriptor, nullptr
      )
    };
    device->updateDescriptorSets( wdss, { } );

    prepareCompute( );
  }

  float cheight = 2.0f;
  glm::vec2 center = glm::vec2( 0.001643721971153f, 0.822467633298876f );

  float lastTime = 0.0f;

  void updateBuffers( void )
  {
    auto size = _window->getExtent( );

    uint32_t width = size.width, height = size.height;

    static auto startTime = std::chrono::high_resolution_clock::now( );

    auto currentTime = std::chrono::high_resolution_clock::now( );
    float time = std::chrono::duration_cast<std::chrono::milliseconds>( currentTime - startTime ).count( ) / 1000.0f;

    graphics.uboVS.model = glm::rotate( glm::mat4( 1.0f ), time * glm::radians( 90.0f ), glm::vec3( 0.0f, 0.0f, 1.0f ) );
    graphics.uboVS.model = glm::rotate( graphics.uboVS.model, time * glm::radians( 45.0f ), glm::vec3( 0.0f, 1.0f, 0.0f ) );

    graphics.uboVS.view = glm::lookAt( glm::vec3( 2.0f, 2.0f, 2.0f ), glm::vec3( 0.0f, 0.0f, 0.0f ), glm::vec3( 0.0f, 0.0f, 1.0f ) );
    graphics.uboVS.proj = glm::perspective( glm::radians( 45.0f ), width / ( float ) height, 0.1f, 10.0f );
    graphics.uboVS.proj[ 1 ][ 1 ] *= -1;

    graphics.mvpBuffer->writeData( 0, sizeof( graphics.uboVS ), &graphics.uboVS );

    float dy = cheight / height;

    float speed = 5.0f;

    float deltaT = time - lastTime;

    lastTime = time;

    cheight -= deltaT * speed * dy;

    float ar = 1.0f;
    float cwidth = cheight * ar;

    glm::vec2 newCenter = center;
    newCenter.x += sin( time * 0.5f );
    newCenter.y += cos( time * 0.5f );

    glm::vec4 bbox( newCenter.x - cwidth / 2.0f, newCenter.y - cheight / 2.0f,
      newCenter.x + cwidth / 2.0f, newCenter.y + cheight / 2.0f );

    compute.ubo.winSize = bbox;

    compute.uniformBuffer->writeData( 0, sizeof( compute.ubo ), &compute.ubo );
  }

  void nextFrame( void ) override
  {
    if ( Input::isKeyPressed( lava::Keyboard::Key::Esc ) )
    {
      _window->_window->close( );
    }

    updateBuffers( );

    // Submit compute commands
    uint32_t timeout = std::numeric_limits<uint64_t>::max( );
    lava::Fence::waitForFences( { compute.fence }, true, timeout );
    lava::Fence::resetFences( { compute.fence } );

    compute.queue->submit( compute.commandBuffer, compute.fence );
    
    std::array<vk::ClearValue, 2 > clearValues;
    std::array<float, 4> ccv = { 0.2f, 0.3f, 0.3f, 1.0f };
    clearValues[ 0 ].color = vk::ClearColorValue( ccv );
    clearValues[ 1 ].depthStencil = vk::ClearDepthStencilValue( 1.0f, 0 );

    const glm::ivec2 size = _window->swapChainImageSize( );
    auto cmd = _window->currentCommandBuffer( );


    ImageMemoryBarrier imb( vk::AccessFlagBits::eShaderWrite,
      vk::AccessFlagBits::eShaderRead, vk::ImageLayout::eGeneral,
      vk::ImageLayout::eGeneral, 0, 0,
      std::make_shared< lava::Image >( _window->device( ), textureComputeTarget->image ),
      vk::ImageSubresourceRange( vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 )
    );

    cmd->pipelineBarrier( vk::PipelineStageFlagBits::eComputeShader,
      vk::PipelineStageFlagBits::eFragmentShader, {}, {}, {}, imb
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
      graphics.pipelineLayout, 0, { graphics.descriptorSet }, {} );

    cmd->bindVertexBuffer( 0, graphics.vertexBuffer, 0 );
    cmd->bindIndexBuffer( graphics.indexBuffer, 0, vk::IndexType::eUint16 );

    cmd->setViewportScissors( _window->getExtent( ) );
    cmd->drawIndexed( indices.size( ), 1, 0, 0, 1 );

    cmd->endRenderPass( );

    _window->frameReady( );
  }

private:
  VulkanWindow *_window;
  std::shared_ptr< Texture > textureComputeTarget;

  struct
  {
    std::shared_ptr< UniformBuffer > mvpBuffer;
    std::shared_ptr< Pipeline > pipeline;
    std::shared_ptr< PipelineLayout > pipelineLayout;
    std::shared_ptr< DescriptorSet > descriptorSet;
    std::shared_ptr< DescriptorSetLayout > descriptorSetLayout;

    std::shared_ptr< Buffer > vertexBuffer;
    std::shared_ptr< Buffer > indexBuffer;
    struct
    {
      glm::mat4 model;
      glm::mat4 view;
      glm::mat4 proj;
    } uboVS;
  } graphics;

  std::shared_ptr< DescriptorPool > descriptorPool;

  struct
  {
    std::shared_ptr<UniformBuffer> uniformBuffer;
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
      glm::vec4 winSize;
      int width = 256;
      int height = 256;
    } ubo;
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

  return 0;
}