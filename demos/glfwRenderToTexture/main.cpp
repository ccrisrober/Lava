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
    std::shared_ptr<pompeii::Image> color;
    std::shared_ptr<pompeii::ImageView> colorView;
    std::shared_ptr<pompeii::Image> msaaColor;
    std::shared_ptr<pompeii::ImageView> msaaColorView;
    std::shared_ptr<pompeii::RenderPass> renderPass;
    std::shared_ptr<pompeii::Framebuffer> framebuffer;

    std::shared_ptr<pompeii::Sampler> sampler;
  } fb;

  std::shared_ptr<pompeii::CommandBuffer> rtCmdBuffer;
  std::shared_ptr<pompeii::Semaphore> rtSemaphore;
  std::shared_ptr<pompeii::/*Graphics*/Pipeline> rtPipeline;

  std::shared_ptr<pompeii::UniformBuffer> uniformBuffer;
  std::shared_ptr<pompeii::Sampler> nearestSampler;
  std::shared_ptr<pompeii::DescriptorSetLayout> descriptorSetLayout;
  std::shared_ptr<pompeii::DescriptorSet> descriptorSet;
  std::shared_ptr<pompeii::DescriptorPool> descriptorPool;
  std::shared_ptr<pompeii::PipelineLayout> pipelineLayout;
  std::shared_ptr<pompeii::/*Graphics*/Pipeline> graphicsPipeline;

  glm::uvec2 fbSize = glm::uvec2(48, 32);
  uint32_t msaaSamples = 4;

  virtual void initResources( void )
  {
    createFramebuffer( { fbSize.x, fbSize.y } );
    createUniformBuffer( );
    setupDescriptorSet( );
    setupPipelines( );
    recordRTCommandBuffer( );
  }

  uint32_t maxSampleCount( vk::SampleCountFlags sampleCounts ) const
  {
    const std::vector<vk::SampleCountFlags> sampleCountBits = {
      vk::SampleCountFlagBits::e1,
      vk::SampleCountFlagBits::e2,
      vk::SampleCountFlagBits::e4,
      vk::SampleCountFlagBits::e8,
      vk::SampleCountFlagBits::e16,
      vk::SampleCountFlagBits::e32,
      vk::SampleCountFlagBits::e64
    };
    for ( auto it = sampleCountBits.rbegin( ); it < sampleCountBits.rend( ); ++it )
    {
      const vk::SampleCountFlags countBit = *it;
      if ( sampleCounts & countBit )
      {
        if ( countBit == vk::SampleCountFlagBits::e1 )
        {
          return 1;
        }
        if ( countBit == vk::SampleCountFlagBits::e2 )
        {
          return 2;
        }
        if ( countBit == vk::SampleCountFlagBits::e4 )
        {
          return 4;
        }
        if ( countBit == vk::SampleCountFlagBits::e8 )
        {
          return 8;
        }
        if ( countBit == vk::SampleCountFlagBits::e16 )
        {
          return 16;
        }
        if ( countBit == vk::SampleCountFlagBits::e32 )
        {
          return 32;
        }
        if ( countBit == vk::SampleCountFlagBits::e64 )
        {
          return 64;
        }
      }
    }
    return 1;
  }

  void createFramebuffer( const vk::Extent2D& extent )
  {
    auto device = _window->device( );
    auto colorFormat = vk::Format::eR8G8B8A8Unorm;
    fb.color = device->createImage( { }, vk::ImageType::e2D,
      colorFormat, vk::Extent3D( extent.width, extent.height, 1 ), 1, 1,
      vk::SampleCountFlagBits::e1, vk::ImageTiling::eOptimal,
      // We will sample directly from the color attachment
      vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled,
      vk::SharingMode::eExclusive, { }, vk::ImageLayout::eUndefined,
      vk::MemoryPropertyFlagBits::eDeviceLocal );
    fb.colorView = fb.color->createImageView( vk::ImageViewType::e2D, colorFormat );

    // Make sure that we are fit to hardware limits
    const vk::ImageFormatProperties formatProperties =
      _window->physicalDevice( )->getImageFormatProperties( colorFormat, vk::ImageType::e2D, true,
        vk::ImageUsageFlagBits::eColorAttachment );
    /*const */uint32_t maxFormatSamples = maxSampleCount( formatProperties.sampleCounts );
    maxFormatSamples = 1;
    if ( msaaSamples > maxFormatSamples )
    {
      msaaSamples = maxFormatSamples;
    }
    //msaaSamples = 1;
    if ( msaaSamples > 1 )
    {
      fb.msaaColor = device->createImage( { }, vk::ImageType::e2D,
        colorFormat, vk::Extent3D( extent.width, extent.height, 1 ), 1, 1,
        vk::SampleCountFlagBits::e4, vk::ImageTiling::eOptimal,
        // This image will only be used as a transient render target.
        // Its purpose is only to hold the multisampled data before resolving the render pass.
        vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransientAttachment,
        vk::SharingMode::eExclusive, { }, vk::ImageLayout::eUndefined,
        vk::MemoryPropertyFlagBits::eDeviceLocal/*eLazilyAllocated*/ );
      fb.msaaColorView = fb.color->createImageView( 
        vk::ImageViewType::e2D, colorFormat );

      vk::AttachmentDescription colorAttachment;
      colorAttachment.format = colorFormat;
      colorAttachment.samples = vk::SampleCountFlagBits::e4;
      colorAttachment.loadOp = vk::AttachmentLoadOp::eClear;

      // DONT_CARE is critical here, since it allows tile based renderers to completely avoid
      // writing out the multisampled framebuffer to memory. This is a huge performance and bandwidth
      // improvement.
      colorAttachment.storeOp = vk::AttachmentStoreOp::eDontCare;
      colorAttachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
      colorAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
      colorAttachment.initialLayout = vk::ImageLayout::eUndefined;
      colorAttachment.finalLayout = vk::ImageLayout::eColorAttachmentOptimal;

      vk::AttachmentDescription colorAttachmentResolve;
      colorAttachmentResolve.format = colorFormat;
      colorAttachmentResolve.samples = vk::SampleCountFlagBits::e1;
      colorAttachmentResolve.loadOp = vk::AttachmentLoadOp::eDontCare;
      colorAttachmentResolve.storeOp = vk::AttachmentStoreOp::eStore;
      colorAttachmentResolve.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
      colorAttachmentResolve.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
      colorAttachmentResolve.initialLayout = vk::ImageLayout::eUndefined;
      colorAttachmentResolve.finalLayout = vk::ImageLayout::eShaderReadOnlyOptimal;

      vk::AttachmentReference colorAttachmentRef;
      colorAttachmentRef.attachment = 0;
      colorAttachmentRef.layout = vk::ImageLayout::eColorAttachmentOptimal;

      vk::AttachmentReference colorAttachmentResolveRef;
      colorAttachmentResolveRef.attachment = 1;
      colorAttachmentResolveRef.layout = vk::ImageLayout::eColorAttachmentOptimal;

      vk::SubpassDescription subpass;
      subpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
      subpass.colorAttachmentCount = 1;
      subpass.pColorAttachments = &colorAttachmentRef;
      // Pass our resolve attachments to the sub pass
      subpass.pResolveAttachments = &colorAttachmentResolveRef;

      // Use subpass dependencies for layout transitions
      std::array<vk::SubpassDependency, 2> dependencies;

      dependencies[ 0 ].srcSubpass = VK_SUBPASS_EXTERNAL;
      dependencies[ 0 ].dstSubpass = 0;
      dependencies[ 0 ].srcStageMask = vk::PipelineStageFlagBits::eBottomOfPipe;;
      dependencies[ 0 ].dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
      dependencies[ 0 ].srcAccessMask = vk::AccessFlagBits::eMemoryRead;
      dependencies[ 0 ].dstAccessMask = vk::AccessFlagBits::eColorAttachmentRead |
        vk::AccessFlagBits::eColorAttachmentWrite;
      dependencies[ 0 ].dependencyFlags = vk::DependencyFlagBits::eByRegion;

      dependencies[ 1 ].srcSubpass = 0;
      dependencies[ 1 ].dstSubpass = VK_SUBPASS_EXTERNAL;
      dependencies[ 1 ].srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
      dependencies[ 1 ].dstStageMask = vk::PipelineStageFlagBits::eBottomOfPipe;
      dependencies[ 1 ].srcAccessMask =
        vk::AccessFlagBits::eColorAttachmentRead |
        vk::AccessFlagBits::eColorAttachmentWrite;
      dependencies[ 1 ].dstAccessMask = vk::AccessFlagBits::eMemoryRead;
      dependencies[ 1 ].dependencyFlags = vk::DependencyFlagBits::eByRegion;

      std::array<vk::AttachmentDescription, 2> attachments = { 
        colorAttachment, colorAttachmentResolve
      };

      fb.renderPass = _window->device( )->createRenderPass(
        attachments, subpass, dependencies );

      fb.framebuffer = device->createFramebuffer( fb.renderPass, 
        { fb.msaaColorView, fb.colorView }, extent, 1 );
    }
    else
    {
      vk::AttachmentDescription attachmentDescription;
      attachmentDescription.format = colorFormat;
      attachmentDescription.samples = vk::SampleCountFlagBits::e1;
      attachmentDescription.loadOp = vk::AttachmentLoadOp::eClear;
      attachmentDescription.storeOp = vk::AttachmentStoreOp::eStore;
      attachmentDescription.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
      attachmentDescription.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
      attachmentDescription.initialLayout = vk::ImageLayout::eUndefined;
      attachmentDescription.finalLayout = vk::ImageLayout::eShaderReadOnlyOptimal;

      vk::AttachmentReference colorRef;
      colorRef.attachment = 0;
      colorRef.layout = vk::ImageLayout::eColorAttachmentOptimal;

      vk::SubpassDescription subpass;
      subpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
      subpass.colorAttachmentCount = 1;
      subpass.pColorAttachments = &colorRef;

      // Use subpass dependencies for layout transitions
      std::array<vk::SubpassDependency, 2> dependencies;

      dependencies[ 0 ].srcSubpass = VK_SUBPASS_EXTERNAL;
      dependencies[ 0 ].dstSubpass = 0;
      dependencies[ 0 ].srcStageMask = vk::PipelineStageFlagBits::eBottomOfPipe;;
      dependencies[ 0 ].dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
      dependencies[ 0 ].srcAccessMask = vk::AccessFlagBits::eMemoryRead;
      dependencies[ 0 ].dstAccessMask = vk::AccessFlagBits::eColorAttachmentRead |
        vk::AccessFlagBits::eColorAttachmentWrite;
      dependencies[ 0 ].dependencyFlags = vk::DependencyFlagBits::eByRegion;

      dependencies[ 1 ].srcSubpass = 0;
      dependencies[ 1 ].dstSubpass = VK_SUBPASS_EXTERNAL;
      dependencies[ 1 ].srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
      dependencies[ 1 ].dstStageMask = vk::PipelineStageFlagBits::eBottomOfPipe;
      dependencies[ 1 ].srcAccessMask =
        vk::AccessFlagBits::eColorAttachmentRead |
        vk::AccessFlagBits::eColorAttachmentWrite;
      dependencies[ 1 ].dstAccessMask = vk::AccessFlagBits::eMemoryRead;
      dependencies[ 1 ].dependencyFlags = vk::DependencyFlagBits::eByRegion;

      fb.renderPass = _window->device( )->createRenderPass(
        attachmentDescription, subpass, dependencies );

      fb.framebuffer = device->createFramebuffer( fb.renderPass, { fb.colorView }, extent, 1 );
    }
  }

  void createUniformBuffer( void )
  {
    uniformBuffer = _window->device( )->createUniformBuffer( sizeof( glm::mat4 ) ); // Only transform matrix
  }

  void setupDescriptorSet( void )
  {
    auto device = _window->device( );
    descriptorPool = device->createDescriptorPool( 1, {
      vk::DescriptorPoolSize( vk::DescriptorType::eUniformBuffer, 1 ),
      vk::DescriptorPoolSize( vk::DescriptorType::eCombinedImageSampler, 1 ),
    } );
    descriptorSetLayout = device->createDescriptorSetLayout( {
      DescriptorSetLayoutBinding( 0,
        vk::DescriptorType::eCombinedImageSampler,
        vk::ShaderStageFlagBits::eFragment
      ),
      DescriptorSetLayoutBinding( 1,
        vk::DescriptorType::eUniformBuffer,
        vk::ShaderStageFlagBits::eVertex
      )
    } );
    descriptorSet = device->allocateDescriptorSet(
      descriptorPool, descriptorSetLayout );

    fb.sampler = device->createSampler(
      vk::Filter::eNearest, vk::Filter::eNearest,
      vk::SamplerMipmapMode::eNearest,
      vk::SamplerAddressMode::eClampToEdge,
      vk::SamplerAddressMode::eClampToEdge,
      vk::SamplerAddressMode::eClampToEdge,
      0.0f, false, 0.0f, false,
      vk::CompareOp::eNever, 0.0f, 1.0f,
      vk::BorderColor::eFloatOpaqueWhite, false );

    std::vector<WriteDescriptorSet> wdss =
    {
      WriteDescriptorSet(
        descriptorSet, 0, 0, vk::DescriptorType::eCombinedImageSampler, 1,
        DescriptorImageInfo( vk::ImageLayout::eShaderReadOnlyOptimal,
          fb.colorView, fb.sampler
        ), nullptr
      ),
      WriteDescriptorSet(
        descriptorSet, 1, 0, vk::DescriptorType::eUniformBuffer,
        1, nullptr, DescriptorBufferInfo( uniformBuffer, 0, sizeof( glm::mat4 ) )
      )
    };
    device->updateDescriptorSets( wdss, { } );
  }

  void setupPipelines( void )
  {
    auto device = _window->device( );
    pipelineLayout = device->createPipelineLayout( descriptorSetLayout );

    {
      auto vertexStage = device->createShaderPipelineShaderStage(
        POMPEII_EXAMPLES_SPV_ROUTE + std::string( "renderTextureTriangle_vert.spv" ),
        vk::ShaderStageFlagBits::eVertex
      );
      auto fragmentStage = device->createShaderPipelineShaderStage(
        POMPEII_EXAMPLES_SPV_ROUTE + std::string( "renderTextureTriangle_frag.spv" ),
        vk::ShaderStageFlagBits::eFragment
      );

      PipelineVertexInputStateCreateInfo vertexInput;

      vk::PipelineInputAssemblyStateCreateInfo assembly( { },
        vk::PrimitiveTopology::eTriangleList, VK_FALSE );
      PipelineViewportStateCreateInfo viewport( 1, 1 );
      vk::PipelineRasterizationStateCreateInfo rasterization( { }, true,
        false, vk::PolygonMode::eFill, vk::CullModeFlagBits::eNone,
        vk::FrontFace::eCounterClockwise, false, 0.0f, 0.0f, 0.0f, 1.0f );

      vk::StencilOpState stencilOpState( vk::StencilOp::eKeep,
        vk::StencilOp::eKeep, vk::StencilOp::eKeep, vk::CompareOp::eAlways,
        0, 0, 0 );
      vk::PipelineDepthStencilStateCreateInfo depthStencil( { }, true, true,
        vk::CompareOp::eLessOrEqual, false, false, stencilOpState,
        stencilOpState, 0.0f, 0.0f );
      ;
      PipelineColorBlendStateCreateInfo colorBlend( false, vk::LogicOp::eNoOp,
        vk::PipelineColorBlendAttachmentState( false,
          vk::BlendFactor::eZero, vk::BlendFactor::eZero, vk::BlendOp::eAdd,
          vk::BlendFactor::eZero, vk::BlendFactor::eZero, vk::BlendOp::eAdd,
          vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
          vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA
        ), { 1.0f, 1.0f, 1.0f, 1.0f }
      );
      PipelineDynamicStateCreateInfo dynamic( {
        vk::DynamicState::eViewport, vk::DynamicState::eScissor
      } );


      if ( msaaSamples > 1 )
      {
        PipelineMultisampleStateCreateInfo multisample( vk::SampleCountFlagBits::e2,
          false, 0.0f, nullptr, false, false );
        rtPipeline = device->createGraphicsPipeline( _window->pipelineCache( ), { },
        { vertexStage, fragmentStage }, vertexInput, assembly, nullptr,
          viewport, rasterization, multisample, depthStencil, colorBlend, dynamic,
          pipelineLayout, fb.renderPass
        );
      }
      else
      {
        PipelineMultisampleStateCreateInfo multisample( vk::SampleCountFlagBits::e1,
          false, 0.0f, nullptr, false, false );
        rtPipeline = device->createGraphicsPipeline( _window->pipelineCache( ), { },
        { vertexStage, fragmentStage }, vertexInput, assembly, nullptr,
          viewport, rasterization, multisample, depthStencil, colorBlend, dynamic,
          pipelineLayout, fb.renderPass
        );
      }
    }
    {
      auto vertexStage = device->createShaderPipelineShaderStage(
        POMPEII_EXAMPLES_SPV_ROUTE + std::string( "renderTexturePlane_vert.spv" ),
        vk::ShaderStageFlagBits::eVertex
      );
      auto fragmentStage = device->createShaderPipelineShaderStage(
        POMPEII_EXAMPLES_SPV_ROUTE + std::string( "renderTexturePlane_frag.spv" ),
        vk::ShaderStageFlagBits::eFragment
      );

      PipelineVertexInputStateCreateInfo vertexInput( { }, { } );
      vk::PipelineInputAssemblyStateCreateInfo assembly( { },
        vk::PrimitiveTopology::eTriangleStrip, VK_FALSE );
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

      graphicsPipeline = device->createGraphicsPipeline( _window->pipelineCache( ), { },
      { vertexStage, fragmentStage }, vertexInput, assembly, nullptr,
        viewport, rasterization, multisample, depthStencil, colorBlend, dynamic,
        pipelineLayout, _window->renderPass( ) );
    }
  }

  void recordRTCommandBuffer( void )
  {
    rtCmdBuffer = _window->gfxCommandPool( )->allocateCommandBuffer( );
    rtSemaphore = _window->device( )->createSemaphore( );

    rtCmdBuffer->begin( );
    {
      std::array<vk::ClearValue, 2 > clearValues;
      std::array<float, 4> ccv = { 1.0f, 1.0f, 1.0f, 1.0f };
      clearValues[ 0 ].color = vk::ClearColorValue( ccv );
      clearValues[ 1 ].depthStencil = vk::ClearDepthStencilValue( 1.0f, 0 );

      rtCmdBuffer->beginRenderPass( fb.renderPass, fb.framebuffer,
        vk::Rect2D( { 0, 0 }, { fbSize.x, fbSize.y } ), clearValues, vk::SubpassContents::eInline );
      {
        rtCmdBuffer->setViewportScissors( { fbSize.x, fbSize.y } );
        rtCmdBuffer->bindDescriptorSets( vk::PipelineBindPoint::eGraphics,
          pipelineLayout, 0, descriptorSet, { } );
        rtCmdBuffer->bindGraphicsPipeline( rtPipeline );
        rtCmdBuffer->draw( 3 );
      }
      rtCmdBuffer->endRenderPass( );
    }
    rtCmdBuffer->end( );
  }

  virtual void nextFrame( void )
  {
    _window->gfxQueue( )->submit( SubmitInfo{
        _window->currentSemaphore( ),
        { vk::PipelineStageFlagBits::eColorAttachmentOutput },
        rtCmdBuffer,
        rtSemaphore
    } );
    auto cmd = _window->currentCommandBuffer( );


    static auto startTime = std::chrono::high_resolution_clock::now( );

    auto currentTime = std::chrono::high_resolution_clock::now( );
    float time = std::chrono::duration_cast<std::chrono::milliseconds>( currentTime - startTime ).count( ) / 1000.0f;

    const float speed = 0.5f;
    static float angle = 0.f;
    angle = time * speed;

    glm::mat4 transform( 1.0f );
    transform = glm::rotate( transform, angle, glm::vec3( 0.0f, 0.3f, 1.0f ) );
    uniformBuffer->set( &transform );


    std::array<vk::ClearValue, 2 > clearValues;
    clearValues[ 0 ] = pompeii::utils::getClearValueColor( 0.0f, 0.0f, 0.0f );
    clearValues[ 1 ] = pompeii::utils::getClearValueDepth( );

    vk::Extent2D extent = _window->swapchainImageSize( );

    cmd->beginRenderPass( _window->renderPass( ),
      _window->framebuffer( ),
      vk::Rect2D( { 0, 0 }, extent ), clearValues,
      vk::SubpassContents::eInline );

    cmd->setViewportScissors( extent );
    cmd->bindDescriptorSets(
      vk::PipelineBindPoint::eGraphics, pipelineLayout, 0, descriptorSet );
    cmd->bindGraphicsPipeline( graphicsPipeline );
    cmd->draw( 4 );

    cmd->endRenderPass( );

    _window->frameReady( rtSemaphore );
  }
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
  VulkanWindow app( 1024, 768, "GLFWRenderer", true );
  app.show( );
  return EXIT_SUCCESS;
}