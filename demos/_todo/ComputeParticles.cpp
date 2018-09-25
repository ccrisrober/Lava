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

#include <pompeii/pompeii.h>
using namespace pompeii;

#include <routes.h>

class CustomRenderer : public VulkanWindowRenderer
{
public:
  CustomRenderer( glfw::VulkanWindow *w )
    : VulkanWindowRenderer( )
    , _window( w )
    , nParticles( 100, 100, 100 )
    , bh1( 5.0f, 0.0f, 0.0f, 1.0f )
    , bh2( -5.0f, 0.0f, 0.0f, 1.0f )
  {
    totalParticles = nParticles.x * nParticles.y * nParticles.z;
  }
  glm::ivec3 nParticles;
  uint32_t totalParticles;
  glm::vec4 bh1, bh2;
  void initResources( void ) override
  {
    auto device = _window->device( );



    // Initial positions of the particles
    std::vector<float> initPos;
    std::vector<float> initVel( totalParticles * 4, 0.0f );
    glm::vec4 p( 0.0f, 0.0f, 0.0f, 1.0f );
    float dx = 2.0f / ( nParticles.x - 1 ),
      dy = 2.0f / ( nParticles.y - 1 ),
      dz = 2.0f / ( nParticles.z - 1 );
    // We want to center the particles at (0,0,0)
    glm::mat4 transf = glm::translate( glm::mat4( 1.0f ), 
      glm::vec3( -1.0f, -1.0f, -1.0f ) );
    for ( int i = 0; i < nParticles.x; ++i )
    {
      for ( int j = 0; j < nParticles.y; ++j )
      {
        for ( int k = 0; k < nParticles.z; ++k )
        {
          p.x = dx * i;
          p.y = dy * j;
          p.z = dz * k;
          p.w = 1.0f;
          p = transf * p;
          initPos.push_back( p.x );
          initPos.push_back( p.y );
          initPos.push_back( p.z );
          initPos.push_back( p.w );
        }
      }
    }

    uint32_t posSize = initPos.size( ) * sizeof( float );
    compute.positionSSBO = device->createStorageBuffer( posSize );
    compute.positionSSBO->writeData( 0, posSize, initPos.data( ) );

    uint32_t velSize = initVel.size( ) * sizeof( float );
    compute.velocitySSBO = device->createStorageBuffer( velSize );
    compute.velocitySSBO->writeData( 0, velSize, initVel.data( ) );

    // Compute stage
    {
      auto queueFamilyIndices = _window->physicalDevice( )->getComputeQueueFamilyIndices( );
      assert( !queueFamilyIndices.empty( ) );
      const uint32_t queueComputeFamilyIndex = queueFamilyIndices[ 0 ];

      compute.queue = device->getQueue( queueComputeFamilyIndex, 0 );

      std::vector<DescriptorSetLayoutBinding> dslbs = 
      {
        // Binding 0: Position SSBO
        DescriptorSetLayoutBinding(
          0, vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eCompute
        ),
        // Binding 1: Velocity SSBO
        DescriptorSetLayoutBinding(
          0, vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eCompute
        ),
      };
      compute.descriptorSetLayout = device->createDescriptorSetLayout( dslbs );
      compute.pipelineLayout = device->createPipelineLayout( compute.descriptorSetLayout );

      compute.descriptorSet = device->allocateDescriptorSet( descriptorPool, compute.descriptorSetLayout );

      std::vector<WriteDescriptorSet> wdss =
      {
        // Binding 0: Position SSBO
        WriteDescriptorSet(
          compute.descriptorSet, 1, 0, vk::DescriptorType::eStorageBuffer,
          1, nullptr, DescriptorBufferInfo( compute.positionSSBO, 0, posSize )
        ),
        // Binding 1: Velocity SSBO
        WriteDescriptorSet(
          compute.descriptorSet, 1, 0, vk::DescriptorType::eStorageBuffer,
          1, nullptr, DescriptorBufferInfo( compute.velocitySSBO, 0, velSize )
        )
      };

      device->updateDescriptorSets( wdss, {} );
      // Create compute shader pipelines
      std::cout << "Loading shader... ";
      auto computeStage = device->createShaderPipelineShaderStage(
        POMPEII_EXAMPLES_SPV_ROUTE + std::string( "particles_comp.spv" ),
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
        compute.commandBuffer->begin( );
          compute.commandBuffer->dispatch( totalParticles / 1000, 1, 1 );
        compute.commandBuffer->end( );
      }
    }

    auto vertexStage = device->createShaderPipelineShaderStage(
      POMPEII_EXAMPLES_SPV_ROUTE + std::string( "particles_vert.spv" ),
      vk::ShaderStageFlagBits::eVertex
    );
    auto fragmentStage = device->createShaderPipelineShaderStage(
      POMPEII_EXAMPLES_SPV_ROUTE + std::string( "particles_frag.spv" ),
      vk::ShaderStageFlagBits::eFragment
    );

    PipelineVertexInputStateCreateInfo vertexInput( {}, {} );

    vk::PipelineInputAssemblyStateCreateInfo assembly( {},
      vk::PrimitiveTopology::ePointList, VK_FALSE );
    PipelineViewportStateCreateInfo viewport( 1, 1 );
    vk::PipelineRasterizationStateCreateInfo rasterization( {}, true,
      false, vk::PolygonMode::eFill, vk::CullModeFlagBits::eNone,
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
    vk::PipelineColorBlendAttachmentState colorBlendAttachment( true,
      vk::BlendFactor::eSrcAlpha, vk::BlendFactor::eOneMinusSrcAlpha, vk::BlendOp::eAdd,
      vk::BlendFactor::eOne, vk::BlendFactor::eOneMinusSrcAlpha, vk::BlendOp::eAdd,
      vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
      vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA
    );
    PipelineColorBlendStateCreateInfo colorBlend( false, vk::LogicOp::eNoOp,
      colorBlendAttachment, { 1.0f, 1.0f, 1.0f, 1.0f }
    );
    PipelineDynamicStateCreateInfo dynamic( {
      vk::DynamicState::eViewport, vk::DynamicState::eScissor
    } );

    graphics.pipeline = device->createGraphicsPipeline( _window->pipelineCache, {},
      { vertexStage, fragmentStage }, vertexInput, assembly, nullptr,
      viewport, rasterization, multisample, depthStencil, colorBlend, dynamic,
      graphics.pipelineLayout, _window->defaultRenderPass( )
    );
  }

  void nextFrame( void ) override
  {
    if ( Input::isKeyPressed( pompeii::Keyboard::Key::Esc ) )
    {
      _window->_window->close( );
    }

    uint32_t timeout = std::numeric_limits<uint64_t>::max( );
    pompeii::Fence::waitForFences( { compute.fence }, true, timeout );
    pompeii::Fence::resetFences( { compute.fence } );

    compute.queue->submit( compute.commandBuffer, compute.fence );

    _window->device( )->waitIdle( );

    std::array<vk::ClearValue, 2 > clearValues;
    std::array<float, 4> ccv = { 1.0f, 1.0f, 1.0f, 1.0f };
    clearValues[ 0 ].color = vk::ClearColorValue( ccv );
    clearValues[ 1 ].depthStencil  = vk::ClearDepthStencilValue(  1.0f, 0 );

    const auto size = _window->swapChainImageSize( );
    auto cmd = _window->currentCommandBuffer( );


    /*vk::BufferMemoryBarrier bmb;
    bmb.srcAccessMask = vk::AccessFlagBits::eVertexAttributeRead;
    bmb.dstAccessMask = vk::AccessFlagBits::eShaderWrite;

    ( vk::AccessFlagBits::eShaderWrite,
      vk::AccessFlagBits::eShaderRead, vk::ImageLayout::eGeneral,
      vk::ImageLayout::eGeneral, 0, 0,
      std::make_shared< pompeii::Image >( _window->device( ), textureComputeTarget->image ),
      vk::ImageSubresourceRange( vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 )
    );

    cmd->pipelineBarrier( vk::PipelineStageFlagBits::eComputeShader,
      vk::PipelineStageFlagBits::eFragmentShader, {}, {}, {}, imb
    );*/


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
    cmd->draw( totalParticles, 1, 0, 0 );

    cmd->endRenderPass( );

    _window->requestUpdate( );
  }
private:
  VulkanWindow *_window;

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
      glm::mat4 mvp;
    } uboVS;
  } graphics;

  std::shared_ptr< DescriptorPool > descriptorPool;

  struct
  {
    std::shared_ptr<Queue> queue;
    std::shared_ptr<CommandPool> commandPool;
    std::shared_ptr<CommandBuffer> commandBuffer;
    std::shared_ptr<Fence> fence;
    std::shared_ptr<Pipeline> pipeline;
    std::shared_ptr<PipelineLayout> pipelineLayout;
    std::shared_ptr<DescriptorSet> descriptorSet;
    std::shared_ptr<DescriptorSetLayout> descriptorSetLayout;

    std::shared_ptr< Buffer > positionSSBO;
    std::shared_ptr< Buffer > velocitySSBO;
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
    POMPEII_KHR_EXT, // OS specific surface extension
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
  w.resize( 800, 600 );

  w.show( );

  return 0;
}