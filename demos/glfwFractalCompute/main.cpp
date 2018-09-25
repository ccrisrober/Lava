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

#include <iostream>

#include <glfwPompeii/glfwPompeii.h>
using namespace pompeii;

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_SWIZZLE

#include <glm/glm.hpp>

#include <glm/gtc/matrix_transform.hpp>

#include <routes.h>

class MainWindowRenderer : public glfw::VulkanWindowRenderer
{
private:
  glfw::VulkanWindow* _window;
public:
  MainWindowRenderer( glfw::VulkanWindow* window )
    : _window( window )
  {
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
    auto usageFlags = vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eStorage;

    //vk::Device dev = static_cast<vk::Device>( *device );

    tex->image = device->createImage( { }, vk::ImageType::e2D, format,
      vk::Extent3D( tex->width, tex->height, 1 ), 1, 1,
      vk::SampleCountFlagBits::e1, vk::ImageTiling::eOptimal, usageFlags,
      vk::SharingMode::eExclusive, { }, vk::ImageLayout::eUndefined,
      vk::MemoryPropertyFlagBits::eDeviceLocal );

    std::shared_ptr<CommandBuffer> layoutCmd = cmdPool->allocateCommandBuffer( );
    layoutCmd->begin( vk::CommandBufferUsageFlagBits::eOneTimeSubmit );

    tex->imageLayout = vk::ImageLayout::eGeneral;
    pompeii::utils::transitionImageLayout(
      layoutCmd, tex->image, vk::ImageAspectFlagBits::eColor,
      vk::ImageLayout::eUndefined, tex->imageLayout
    );

    // Send command buffer
    layoutCmd->end( );

    _window->gfxQueue( )->submitAndWait( layoutCmd ); // TODO: Use another kind of quee

    // Create sampler
    tex->sampler = device->createSampler( vk::Filter::eLinear, vk::Filter::eLinear,
      vk::SamplerMipmapMode::eLinear, vk::SamplerAddressMode::eClampToBorder,
      vk::SamplerAddressMode::eClampToBorder, vk::SamplerAddressMode::eClampToBorder,
      0.0f, true, 1.0f, false, vk::CompareOp::eNever, 0.0f, 0.0f,
      vk::BorderColor::eFloatOpaqueWhite, false );


    // Create image view
    tex->view = tex->image->createImageView( vk::ImageViewType::e2D, format );

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
      POMPEII_EXAMPLES_SPV_ROUTE + std::string( "fractalCompute_comp.spv" ),
      vk::ShaderStageFlagBits::eCompute
    );

    std::cout << "CREATE PIPELINE" << std::endl;

    compute.pipeline = device->createComputePipeline(
      _window->pipelineCache( ), {}, computeStage, compute.pipelineLayout );

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

    auto cmd = _window->gfxCommandPool( )->allocateCommandBuffer( );
    cmd->begin( );
    // Vertex buffer
    {
      uint32_t vertexBufferSize = vertices.size( ) * sizeof( Vertex );

      graphics.vertexBuffer = device->createBuffer( vertexBufferSize,
        vk::BufferUsageFlagBits::eVertexBuffer |
        vk::BufferUsageFlagBits::eTransferDst,
        vk::MemoryPropertyFlagBits::eDeviceLocal );
      graphics.vertexBuffer->update<Vertex>( cmd, 0, { uint32_t( vertices.size( ) ),
        vertices.data( ) } );
    }

    // Index buffer
    {
      uint32_t indexBufferSize = indices.size( ) * sizeof( uint32_t );

      graphics.indexBuffer = device->createBuffer( indexBufferSize,
        vk::BufferUsageFlagBits::eIndexBuffer |
        vk::BufferUsageFlagBits::eTransferDst,
        vk::MemoryPropertyFlagBits::eDeviceLocal );
      graphics.indexBuffer->update<uint16_t>( cmd, 0, { uint32_t( indices.size( ) ),
        indices.data( ) } );
    }
    cmd->end( );
    _window->gfxQueue( )->submitAndWait( cmd );
  
    // Uniform buffers
    {
      graphics.mvpBuffer = device->createUniformBuffer( sizeof( graphics.uboVS ) );

      compute.uniformBuffer = device->createUniformBuffer( sizeof( compute.ubo ) );
    }

    textureComputeTarget = std::make_shared<Texture>( device );
    prepareTextureTarget( textureComputeTarget, compute.ubo.width, compute.ubo.height,
      vk::Format::eR8G8B8A8Unorm, _window->gfxCommandPool( ) );

    auto vertexStage = device->createShaderPipelineShaderStage(
      POMPEII_EXAMPLES_SPV_ROUTE + std::string( "cubeUV_vert.spv" ),
      vk::ShaderStageFlagBits::eVertex
    );
    auto fragmentStage = device->createShaderPipelineShaderStage(
      POMPEII_EXAMPLES_SPV_ROUTE + std::string( "cubeUV_frag.spv" ),
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

    graphics.pipeline = device->createGraphicsPipeline( _window->pipelineCache( ), { },
      { vertexStage, fragmentStage }, vertexInput, assembly, nullptr,
      viewport, rasterization, multisample, depthStencil, colorBlend, dynamic,
      graphics.pipelineLayout, _window->renderPass( )
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
    auto size = _window->swapchainImageSize( );

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

  virtual void nextFrame( void )
  {
    updateBuffers( );

    // Submit compute commands
    uint32_t timeout = std::numeric_limits<uint64_t>::max( );
    pompeii::Fence::waitForFences( { compute.fence }, true, timeout );
    pompeii::Fence::resetFences( { compute.fence } );

    compute.queue->submit( compute.commandBuffer, compute.fence );
    
    std::array<vk::ClearValue, 2 > clearValues;
    std::array<float, 4> ccv = { 0.2f, 0.3f, 0.3f, 1.0f };
    clearValues[ 0 ].color = vk::ClearColorValue( ccv );
    clearValues[ 1 ].depthStencil = vk::ClearDepthStencilValue( 1.0f, 0 );

    const auto size = _window->swapchainImageSize( );
    auto cmd = _window->currentCommandBuffer( );


    ImageMemoryBarrier imb( vk::AccessFlagBits::eShaderWrite,
      vk::AccessFlagBits::eShaderRead, vk::ImageLayout::eGeneral,
      vk::ImageLayout::eGeneral, 0, 0,
      textureComputeTarget->image,
      //std::make_shared< pompeii::Image >( _window->device( ), textureComputeTarget->image ),
      vk::ImageSubresourceRange( vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 )
    );

    cmd->pipelineBarrier( vk::PipelineStageFlagBits::eComputeShader,
      vk::PipelineStageFlagBits::eFragmentShader, {}, {}, {}, imb
    );

    vk::Rect2D rect;
    rect.extent = size;
    cmd->beginRenderPass(
      _window->renderPass( ),
      _window->framebuffer( ),
      rect, clearValues, vk::SubpassContents::eInline
    );

    cmd->bindGraphicsPipeline( graphics.pipeline );
    cmd->bindDescriptorSets( vk::PipelineBindPoint::eGraphics, 
      graphics.pipelineLayout, 0, { graphics.descriptorSet }, {} );

    cmd->bindVertexBuffer( 0, graphics.vertexBuffer, 0 );
    cmd->bindIndexBuffer( graphics.indexBuffer, 0, vk::IndexType::eUint16 );

    cmd->setViewportScissors( size );
    cmd->drawIndexed( indices.size( ), 1, 0, 0, 1 );

    cmd->endRenderPass( );

    _window->frameReady( );
  }
private:
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

class VulkanWindow : public glfw::VulkanWindow
{
public:
  explicit VulkanWindow( int width, int height,
    const std::string& title, bool enableLayers )
    : glfw::VulkanWindow( width, height, title, enableLayers )
  {

  }
  /*VulkanWindow( int w, int h, const char* title, bool enableLayers )
    : glfw::VulkanWindow( w, h, title, enableLayers )
  {

  }*/
  virtual glfw::VulkanWindowRenderer* createRenderer( void ) override
  {
    return new MainWindowRenderer( this );
  }
};


int main( int, char** )
{
  VulkanWindow app( 500, 500, "Cube with computed fractal", true );
  app.show( );
  return EXIT_SUCCESS;
}