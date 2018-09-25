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
#include <pompeiiUtils/pompeiiUtils.h>
using namespace pompeii;

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
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

  struct
  {
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
  } uboVS;

  struct Vertex
  {
    glm::vec3 pos;
    glm::vec2 texCoord;
  };

  const float side = 1.0f;
  const float side2 = side * 0.5f;

  const std::vector<Vertex> vertices =
  {
    { { -side2, -side2,  side2 }, { 0.0f, 0.0f } },
    { { side2, -side2,  side2 }, { 1.0f, 0.0f } },
    { { -side2,  side2,  side2 }, { 0.0f, 1.0f } },
    { { side2,  side2,  side2 }, { 1.0f, 1.0f } },

    { { -side2, -side2, -side2 }, { 0.0f, 0.0f } },
    { { side2, -side2, -side2 }, { 1.0f, 0.0f } },
    { { -side2,  side2, -side2 }, { 0.0f, 1.0f } },
    { { side2,  side2, -side2 }, { 1.0f, 1.0f } },

    { { side2, -side2, -side2 }, { 0.0f, 0.0f } },
    { { side2, -side2,  side2 }, { 1.0f, 0.0f } },
    { { side2,  side2, -side2 }, { 0.0f, 1.0f } },
    { { side2,  side2,  side2 }, { 1.0f, 1.0f } },

    { { -side2, -side2, -side2 }, { 0.0f, 0.0f } },
    { { -side2, -side2,  side2 }, { 1.0f, 0.0f } },
    { { -side2,  side2, -side2 }, { 0.0f, 1.0f } },
    { { -side2,  side2,  side2 }, { 1.0f, 1.0f } },

    { { -side2,  side2, -side2 }, { 0.0f, 0.0f } },
    { { -side2,  side2,  side2 }, { 1.0f, 0.0f } },
    { { side2,  side2, -side2 }, { 0.0f, 1.0f } },
    { { side2,  side2,  side2 }, { 1.0f, 1.0f } },

    { { -side2, -side2, -side2 }, { 0.0f, 0.0f } },
    { { -side2, -side2,  side2 } ,{ 1.0f, 0.0f } },
    { { side2, -side2, -side2 }, { 0.0f, 1.0f } },
    { { side2, -side2,  side2 }, { 1.0f, 1.0f } }
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

  void recreatePPCommandBuffer( void )
  {
    std::array<vk::ClearValue, 2 > clearValues;
    std::array<float, 4> ccv = { 0.2f, 0.3f, 0.3f, 1.0f };
    clearValues[ 0 ].color = vk::ClearColorValue( ccv );
    clearValues[ 1 ].depthStencil = vk::ClearDepthStencilValue( 1.0f, 0 );

    const auto size = _window->swapchainImageSize( );
    vk::Rect2D rect;
    rect.extent = size;

    {
      cmdSolidBuffer = _window->gfxCommandPool( )->allocateCommandBuffer( );
      cmdSolidBuffer->begin( vk::CommandBufferUsageFlagBits::eSimultaneousUse );

      cmdSolidBuffer->beginRenderPass(
        postProcessFBO->_renderPass,
        postProcessFBO->_framebuffer,
        rect, clearValues, vk::SubpassContents::eInline );

      cmdSolidBuffer->setViewportScissors( _window->swapchainImageSize( ) );
      cmdSolidBuffer->bindDescriptorSets( vk::PipelineBindPoint::eGraphics,
        pipelineLayouts.solid, 0, { descriptorSets.solid }, nullptr );
      cmdSolidBuffer->bindGraphicsPipeline( pipelines.solid );

      cmdSolidBuffer->bindVertexBuffer( 0, vertexBuffer, 0 );
      cmdSolidBuffer->bindIndexBuffer( indexBuffer, 0, vk::IndexType::eUint16 );

      cmdSolidBuffer->drawIndexed( indices.size( ), 1, 0, 0, 1 );
      cmdSolidBuffer->endRenderPass( );

      cmdSolidBuffer->end( );
    }
    {
      ppSolidBuffer2 = _window->gfxCommandPool( )->allocateCommandBuffer( );
      ppSolidBuffer2->begin( vk::CommandBufferUsageFlagBits::eSimultaneousUse );

      ppSolidBuffer2->beginRenderPass(
        postProcessFBO2->_renderPass,
        postProcessFBO2->_framebuffer,
        rect, clearValues, vk::SubpassContents::eInline
      );

      ppSolidBuffer2->setViewportScissors( _window->swapchainImageSize( ) );
      ppSolidBuffer2->bindDescriptorSets( vk::PipelineBindPoint::eGraphics,
        pipelineLayouts.postprocess, 0, { descriptorSets.postprocess }, nullptr );
      ppSolidBuffer2->bindGraphicsPipeline( pipelines.postprocess );
      ppSolidBuffer2->draw( 4, 1, 0, 0 );
      ppSolidBuffer2->endRenderPass( );

      ppSolidBuffer2->end( );
    }
  }

  virtual void initSwapChainResources( void ) override
  {
    recreatePostProcessFBO2( );
    recreatePostProcessFBO( );

    recreatePPCommandBuffer( );
  }

  virtual void releaseSwapChainResources( void ) override
  {
    postProcessFBO.reset( );
    postProcessFBO2.reset( );
  }

  void recreatePostProcessFBO( void )
  {
    uint32_t w, h;

    auto device = _window->device( );

    auto sc = _window->swapchainImageSize( );
    w = sc.width;
    h = sc.height;

    postProcessFBO = std::make_shared<pompeii::utility::CustomFramebuffer>( device, w, h );

    // Color attachments
    // Attachment 0: Color
    postProcessFBO->addAttachment( w, h, vk::Format::eR16G16B16A16Sfloat,
      vk::ImageUsageFlagBits::eColorAttachment |
      vk::ImageUsageFlagBits::eSampled
    );

    // Depth attachment
    vk::Format attDepthFormat;
    VkBool32 validDepthFormat = pompeii::utils::getSupportedDepthFormat(
      _window->physicalDevice( ), attDepthFormat );
    assert( validDepthFormat );

    postProcessFBO->addAttachment( w, h, attDepthFormat,
      vk::ImageUsageFlagBits::eDepthStencilAttachment );
    postProcessFBO->createSampler( vk::Filter::eLinear, vk::Filter::eLinear,
      vk::SamplerAddressMode::eClampToEdge );
    postProcessFBO->createRenderPass( );

    std::vector<WriteDescriptorSet> wdss =
    {
      WriteDescriptorSet(
        descriptorSets.postprocess, 0, 0,
        vk::DescriptorType::eCombinedImageSampler, 1,
        DescriptorImageInfo(
          vk::ImageLayout::eShaderReadOnlyOptimal,
          postProcessFBO->attachments[ 0 ].imageView,
          postProcessFBO->_sampler
        ), nullptr
      )
    };
    device->updateDescriptorSets( wdss, { } );
  }

  void createPostProcess( void )
  {
    auto device = _window->device( );

    std::vector<DescriptorSetLayoutBinding> dslbs =
    {
      // Binding 0 : Color texture target
      DescriptorSetLayoutBinding(
        0,
        vk::DescriptorType::eCombinedImageSampler,
        vk::ShaderStageFlagBits::eFragment
      )
    };
    descriptorSetLayouts.postprocess =
      device->createDescriptorSetLayout( dslbs );
    pipelineLayouts.postprocess =
      device->createPipelineLayout( descriptorSetLayouts.postprocess );

    descriptorSets.postprocess =
      device->allocateDescriptorSet( dspPool, descriptorSetLayouts.postprocess );


    recreatePostProcessFBO( );

    PipelineVertexInputStateCreateInfo vertexInput( { }, { } );
    vk::PipelineInputAssemblyStateCreateInfo assembly( { },
      vk::PrimitiveTopology::eTriangleList, VK_FALSE );
    PipelineViewportStateCreateInfo viewport( 1, 1 );
    vk::PipelineRasterizationStateCreateInfo rasterization( { }, true,
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
    vk::PipelineDepthStencilStateCreateInfo depthStencil( { },
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

    auto vertexStage = device->createShaderPipelineShaderStage(
      POMPEII_EXAMPLES_SPV_ROUTE + std::string( "fullquad_vert.spv" ),
      vk::ShaderStageFlagBits::eVertex
    );
    auto fragmentStage = device->createShaderPipelineShaderStage(
      POMPEII_EXAMPLES_SPV_ROUTE + std::string( "vignette_frag.spv" ),
      vk::ShaderStageFlagBits::eFragment
    );

    pipelines.postprocess = device->createGraphicsPipeline( 
      _window->pipelineCache( ), { }, { vertexStage, fragmentStage },
      PipelineVertexInputStateCreateInfo( { }, { } ), assembly, nullptr,
      viewport, rasterization, multisample, depthStencil, colorBlend, dynamic,
      pipelineLayouts.postprocess, postProcessFBO2->_renderPass
    );
  }

  void recreatePostProcessFBO2( void )
  {
    uint32_t w, h;

    auto device = _window->device( );

    auto sc = _window->swapchainImageSize( );
    w = sc.width;
    h = sc.height;

    postProcessFBO2 = std::make_shared<pompeii::utility::CustomFramebuffer>( device, w, h );

    // Color attachments
    // Attachment 0: Color
    postProcessFBO2->addAttachment( w, h, vk::Format::eR16G16B16A16Sfloat,
      vk::ImageUsageFlagBits::eColorAttachment |
      vk::ImageUsageFlagBits::eSampled
    );

    postProcessFBO2->createSampler( vk::Filter::eLinear, vk::Filter::eLinear,
      vk::SamplerAddressMode::eClampToEdge );
    postProcessFBO2->createRenderPass( );

    std::vector<WriteDescriptorSet> wdss =
    {
      WriteDescriptorSet(
        descriptorSets.postprocess2, 0, 0,
        vk::DescriptorType::eCombinedImageSampler, 1,
        DescriptorImageInfo(
          vk::ImageLayout::eShaderReadOnlyOptimal,
          postProcessFBO2->attachments[ 0 ].imageView,
          postProcessFBO2->_sampler
        ), nullptr
      )
    };
    device->updateDescriptorSets( wdss, { } );
  }

  void createPostProcess2( void )
  {
    auto device = _window->device( );

    std::vector<DescriptorSetLayoutBinding> dslbs =
    {
      // Binding 0 : Color texture target
      DescriptorSetLayoutBinding(
        0,
        vk::DescriptorType::eCombinedImageSampler,
        vk::ShaderStageFlagBits::eFragment
      )
    };
    descriptorSetLayouts.postprocess2 =
      device->createDescriptorSetLayout( dslbs );
    pipelineLayouts.postprocess2 =
      device->createPipelineLayout( descriptorSetLayouts.postprocess2 );

    descriptorSets.postprocess2 =
      device->allocateDescriptorSet( dspPool, descriptorSetLayouts.postprocess2 );


    recreatePostProcessFBO2( );

    PipelineVertexInputStateCreateInfo vertexInput( { }, { } );
    vk::PipelineInputAssemblyStateCreateInfo assembly( { },
      vk::PrimitiveTopology::eTriangleList, VK_FALSE );
    PipelineViewportStateCreateInfo viewport( 1, 1 );
    vk::PipelineRasterizationStateCreateInfo rasterization( { }, true,
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
    vk::PipelineDepthStencilStateCreateInfo depthStencil( { },
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

    auto vertexStage = device->createShaderPipelineShaderStage(
      POMPEII_EXAMPLES_SPV_ROUTE + std::string( "fullquad_vert.spv" ),
      vk::ShaderStageFlagBits::eVertex
    );
    auto fragmentStage = device->createShaderPipelineShaderStage(
      POMPEII_EXAMPLES_SPV_ROUTE + std::string( "tintPostProcess_frag.spv" ),
      vk::ShaderStageFlagBits::eFragment
    );

    pipelines.postprocess2 = device->createGraphicsPipeline( _window->pipelineCache( ), { },
    { vertexStage, fragmentStage },
      PipelineVertexInputStateCreateInfo( { }, { } ), assembly, nullptr,
      viewport, rasterization, multisample, depthStencil, colorBlend, dynamic,
      pipelineLayouts.postprocess2, _window->renderPass( )
    );
  }

  void initResources( void ) override
  {
    auto device = _window->device( );

    std::array<vk::DescriptorPoolSize, 2> poolSize =
    {
      vk::DescriptorPoolSize( vk::DescriptorType::eUniformBuffer, 1 ),
      vk::DescriptorPoolSize( vk::DescriptorType::eCombinedImageSampler, 3 )
    };
    dspPool = device->createDescriptorPool( 3, poolSize );

    createPostProcess2( );
    createPostProcess( );

    auto cmd = _window->gfxCommandPool( )->allocateCommandBuffer( );
    cmd->begin( );
    // Vertex buffer
    {
      uint32_t vertexBufferSize = vertices.size( ) * sizeof( Vertex );

      vertexBuffer = device->createBuffer( vertexBufferSize,
        vk::BufferUsageFlagBits::eVertexBuffer |
        vk::BufferUsageFlagBits::eTransferDst,
        vk::MemoryPropertyFlagBits::eDeviceLocal );
      vertexBuffer->update<Vertex>( cmd, 0, { uint32_t( vertices.size( ) ),
        vertices.data( ) } );
    }

    // Index buffer
    {
      uint32_t indexBufferSize = indices.size( ) * sizeof( uint32_t );

      indexBuffer = device->createBuffer( indexBufferSize,
        vk::BufferUsageFlagBits::eIndexBuffer |
        vk::BufferUsageFlagBits::eTransferDst,
        vk::MemoryPropertyFlagBits::eDeviceLocal );
      indexBuffer->update<uint16_t>( cmd, 0, { uint32_t( indices.size( ) ),
        indices.data( ) } );
    }
    cmd->end( );
    _window->gfxQueue( )->submitAndWait( cmd );

    // MVP buffer
    {
      mvpBuffer = device->createUniformBuffer( sizeof( uboVS ) );
    }

    tex = device->createTexture2D( POMPEII_EXAMPLES_IMAGES_ROUTE +
      std::string( "uv_checker.png" ), _window->gfxCommandPool( ),
      _window->gfxQueue( ), vk::Format::eR8G8B8A8Unorm );

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

    descriptorSetLayouts.solid = device->createDescriptorSetLayout( dslbs );

    pipelineLayouts.solid = device->createPipelineLayout( descriptorSetLayouts.solid, nullptr );

    vk::VertexInputBindingDescription binding( 0, sizeof( Vertex ),
      vk::VertexInputRate::eVertex );

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
    vk::PipelineRasterizationStateCreateInfo rasterization( { }, true,
      false, vk::PolygonMode::eFill, vk::CullModeFlagBits::eBack,
      vk::FrontFace::eCounterClockwise, false, 0.0f, 0.0f, 0.0f, 1.0f );
    PipelineMultisampleStateCreateInfo multisample( vk::SampleCountFlagBits::e1,
      false, 0.0f, nullptr, false, false );
    vk::StencilOpState stencilOpState( vk::StencilOp::eKeep,
      vk::StencilOp::eKeep, vk::StencilOp::eKeep, vk::CompareOp::eAlways,
      0, 0, 0 );
    vk::PipelineDepthStencilStateCreateInfo depthStencil( { }, true, true,
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

    pipelines.solid = device->createGraphicsPipeline( _window->pipelineCache( ), { },
    { vertexStage, fragmentStage }, vertexInput, assembly, nullptr,
      viewport, rasterization, multisample, depthStencil, colorBlend, dynamic,
      pipelineLayouts.solid, postProcessFBO->_renderPass
    );

    // Init descriptor set
    descriptorSets.solid = device->allocateDescriptorSet( dspPool, descriptorSetLayouts.solid );

    std::vector<WriteDescriptorSet> wdss =
    {
      WriteDescriptorSet(
        descriptorSets.solid, 0, 0, vk::DescriptorType::eUniformBuffer,
        1, nullptr, DescriptorBufferInfo( mvpBuffer, 0, sizeof( uboVS ) )
      ),
      WriteDescriptorSet(
        descriptorSets.solid, 1, 0, vk::DescriptorType::eCombinedImageSampler, 1,
        tex->descriptor, nullptr
      )
    };
    device->updateDescriptorSets( wdss, { } );

    recreatePPCommandBuffer( );
  }

  void updateMVP( void )
  {
    auto size = _window->swapchainImageSize( );

    uint32_t width = size.width, height = size.height;

    static auto startTime = std::chrono::high_resolution_clock::now( );

    auto currentTime = std::chrono::high_resolution_clock::now( );
    float time = std::chrono::duration_cast<std::chrono::milliseconds>( currentTime - startTime ).count( ) / 1000.0f;

    uboVS.model = glm::rotate( glm::mat4( 1.0f ), time * glm::radians( 90.0f ), glm::vec3( 0.0f, 0.0f, 1.0f ) );
    uboVS.model = glm::rotate( uboVS.model, time * glm::radians( 45.0f ), glm::vec3( 0.0f, 1.0f, 0.0f ) );

    uboVS.view = glm::lookAt( glm::vec3( 2.0f, 2.0f, 2.0f ), glm::vec3( 0.0f, 0.0f, 0.0f ), glm::vec3( 0.0f, 0.0f, 1.0f ) );
    uboVS.proj = glm::perspective( glm::radians( 45.0f ), width / ( float ) height, 0.1f, 10.0f );

    // Vulkan clip space has inverted Y and half Z.
    glm::mat4 clip = glm::mat4(
      1.0f, 0.0f, 0.0f, 0.0f,
      0.0f, -1.0f, 0.0f, 0.0f,
      0.0f, 0.0f, 0.5f, 0.0f,
      0.0f, 0.0f, 0.5f, 1.0f
    );
    uboVS.proj = clip * uboVS.proj;
    //uboVS.proj[ 1 ][ 1 ] *= -1;

    mvpBuffer->writeData( 0, sizeof( uboVS ), &uboVS );
  }

  virtual void nextFrame( void )
  {
    updateMVP( );

    std::array<vk::ClearValue, 2 > clearValues;
    std::array<float, 4> ccv = { 0.2f, 0.3f, 0.3f, 1.0f };
    clearValues[ 0 ].color = vk::ClearColorValue( ccv );
    clearValues[ 1 ].depthStencil = vk::ClearDepthStencilValue( 1.0f, 0 );

    vk::Extent2D extent = _window->swapchainImageSize( );
    auto cmd = _window->currentCommandBuffer( );
    vk::Rect2D rect;
    rect.extent = extent;
    {
      cmd->beginRenderPass(
        _window->renderPass( ),
        _window->framebuffer( ),
        rect, clearValues, vk::SubpassContents::eInline
      );

      cmd->setViewportScissors( extent );
      cmd->bindDescriptorSets( vk::PipelineBindPoint::eGraphics,
        pipelineLayouts.postprocess2, 0, { descriptorSets.postprocess2 }, nullptr );
      cmd->bindGraphicsPipeline( pipelines.postprocess2 );
      cmd->draw( 4, 1, 0, 0 );
      cmd->endRenderPass( );
    }

    _window->gfxQueue( )->submit( SubmitInfo{
      { _window->currentSemaphore( ) },
      { vk::PipelineStageFlagBits::eColorAttachmentOutput },
      cmdSolidBuffer,
      postProcessFBO->semaphore
    } );
    _window->gfxQueue( )->submit( SubmitInfo{
      { postProcessFBO->semaphore },
      { vk::PipelineStageFlagBits::eColorAttachmentOutput },
      ppSolidBuffer2,
      postProcessFBO2->semaphore
    } );

    _window->frameReady( postProcessFBO2->semaphore );
  }
private:
  std::shared_ptr< Texture2D > tex;

  struct
  {
    std::shared_ptr<Pipeline> solid;
    std::shared_ptr<Pipeline> postprocess;
    std::shared_ptr<Pipeline> postprocess2;
  } pipelines;

  struct
  {
    std::shared_ptr<PipelineLayout> solid;
    std::shared_ptr<PipelineLayout> postprocess;
    std::shared_ptr<PipelineLayout> postprocess2;
  } pipelineLayouts;

  struct
  {
    std::shared_ptr<DescriptorSet> solid;
    std::shared_ptr<DescriptorSet> postprocess;
    std::shared_ptr<DescriptorSet> postprocess2;
  } descriptorSets;

  struct
  {
    std::shared_ptr<DescriptorSetLayout> solid;
    std::shared_ptr<DescriptorSetLayout> postprocess;
    std::shared_ptr<DescriptorSetLayout> postprocess2;
  } descriptorSetLayouts;

  std::shared_ptr< Buffer > vertexBuffer;
  std::shared_ptr< Buffer > indexBuffer;
  std::shared_ptr< UniformBuffer > mvpBuffer;

  std::shared_ptr<DescriptorPool> dspPool;

  std::shared_ptr<CommandBuffer> cmdSolidBuffer;
  std::shared_ptr<CommandBuffer> ppSolidBuffer2;

public:
  std::shared_ptr<::utility::CustomFramebuffer> postProcessFBO;
  std::shared_ptr<::utility::CustomFramebuffer> postProcessFBO2;
};

class VulkanWindow : public glfw::VulkanWindow
{
public:
  explicit VulkanWindow( int width, int height,
    const std::string& title, bool enableLayers )
    : glfw::VulkanWindow( width, height, title, enableLayers )
  {
  }
  virtual glfw::VulkanWindowRenderer* createRenderer( void ) override
  {
    return new MainWindowRenderer( this );
  }
};


int main( int, char** )
{
  VulkanWindow app( 500, 500, "GLFWPostProcessing", true );
  app.show( );
  return EXIT_SUCCESS;
}