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

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <routes.h>

struct FramebufferAttachment
{
  std::shared_ptr<Image> image;
  std::shared_ptr<ImageView> imageView;
  vk::Format format;
};

struct Attachments
{
  FramebufferAttachment color, depth;
};

std::vector<Attachments> renderpassAttachments;

class MainWindowRenderer : public glfw::VulkanWindowRenderer
{
private:
  glfw::VulkanWindow* _window;
public:
  MainWindowRenderer( glfw::VulkanWindow* window )
    : _window( window )
  {
  }

  std::shared_ptr<utility::Geometry> geometry;
  std::shared_ptr<Buffer> uniformBuffer1, uniformBuffer2;

  struct
  {
    glm::mat4 projection;
    glm::mat4 view;
    glm::mat4 model;
    glm::vec3 lightPos;
  } ubo1;

  struct
  {
    glm::vec2 brightnessContrast;
    glm::vec2 range;
    int attachmentIndex;
  } ubo2;

  struct
  {
    std::shared_ptr<Pipeline> read, write;
  } pipelines;

  struct
  {
    std::shared_ptr<PipelineLayout> read, write;
  } pipelineLayouts;

  struct
  {
    std::vector<std::shared_ptr<DescriptorSet>> read;
    std::shared_ptr<DescriptorSet> write;
  } descriptorSets;
  struct
  {
    std::shared_ptr<DescriptorSetLayout> read, write;
  } descriptorSetLayouts;

  std::shared_ptr<DescriptorPool> descriptorPool;

  virtual void initResources( void )
  {
    auto device = _window->device( );

    geometry = std::make_shared<pompeii::utility::Geometry>( device,
      POMPEII_EXAMPLES_MESHES_ROUTE + std::string( "monkey.obj_" ) );



    ubo2.brightnessContrast = glm::vec2( 0.5f, 1.8f );
    ubo2.range = glm::vec2( 0.6f, 1.0f );
    ubo2.attachmentIndex = 0;


    
    uniformBuffer1 = device->createUniformBuffer( sizeof( ubo1 ) );
    uniformBuffer2 = device->createUniformBuffer( sizeof( ubo2 ) );

    uint32_t swapCount = _window->framebuffers( ).size( );

    descriptorPool = device->createDescriptorPool( 1 + swapCount, {
      vk::DescriptorPoolSize( vk::DescriptorType::eUniformBuffer, 1 + ( 2 * swapCount ) ),
      vk::DescriptorPoolSize( vk::DescriptorType::eInputAttachment, 2 * swapCount ),
    } );


    descriptorSetLayouts.write = device->createDescriptorSetLayout( {
      DescriptorSetLayoutBinding( 0,
        vk::DescriptorType::eUniformBuffer,
        vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment
      )
    } );
    descriptorSets.write = device->allocateDescriptorSet(
      descriptorPool, descriptorSetLayouts.write );

    descriptorSetLayouts.read = device->createDescriptorSetLayout( {
      DescriptorSetLayoutBinding( 0,
        vk::DescriptorType::eInputAttachment,
        vk::ShaderStageFlagBits::eFragment
      ),
      DescriptorSetLayoutBinding( 1,
        vk::DescriptorType::eInputAttachment,
        vk::ShaderStageFlagBits::eFragment
      ),
      DescriptorSetLayoutBinding( 2,
        vk::DescriptorType::eUniformBuffer,
        vk::ShaderStageFlagBits::eFragment
      )
    } );

    descriptorSets.read.resize( swapCount );
    for ( uint32_t i = 0; i < swapCount; ++i )
    {
      descriptorSets.read[ i ] = device->allocateDescriptorSet(
        descriptorPool, descriptorSetLayouts.read );
    }

    pipelineLayouts.read = device->createPipelineLayout( descriptorSetLayouts.read );
    pipelineLayouts.write = device->createPipelineLayout( descriptorSetLayouts.write );

    {
      auto vertexStage = device->createShaderPipelineShaderStage(
        POMPEII_EXAMPLES_SPV_ROUTE + std::string( "inputAttachment_vert.spv" ),
        vk::ShaderStageFlagBits::eVertex
      );
      auto fragmentStage = device->createShaderPipelineShaderStage(
        POMPEII_EXAMPLES_SPV_ROUTE + std::string( "inputAttachment_frag.spv" ),
        vk::ShaderStageFlagBits::eFragment
      );

      PipelineVertexInputStateCreateInfo vertexInput(
        vk::VertexInputBindingDescription( 0, sizeof( pompeii::utility::Vertex ),
          vk::VertexInputRate::eVertex ),
          {
            vk::VertexInputAttributeDescription(
              0, 0, vk::Format::eR32G32B32Sfloat,
              offsetof( pompeii::utility::Vertex, position )
            ),
            vk::VertexInputAttributeDescription(
              1, 0, vk::Format::eR32G32B32Sfloat,
              offsetof( pompeii::utility::Vertex, normal )
            )
          }
      );

      vk::PipelineInputAssemblyStateCreateInfo assembly( { },
        vk::PrimitiveTopology::eTriangleList, VK_FALSE );
      PipelineViewportStateCreateInfo viewport( 1, 1 );
      vk::PipelineRasterizationStateCreateInfo rasterization( { }, true,
        false, vk::PolygonMode::eFill, vk::CullModeFlagBits::eNone,
        vk::FrontFace::eCounterClockwise, false, 0.0f, 0.0f, 0.0f, 1.0f );
      PipelineMultisampleStateCreateInfo multisample( vk::SampleCountFlagBits::e1,
        false, 0.0f, nullptr, false, false );
      vk::StencilOpState stencilOpState( vk::StencilOp::eKeep,
        vk::StencilOp::eKeep, vk::StencilOp::eKeep, vk::CompareOp::eAlways,
        0, 0, 0 );
      vk::PipelineDepthStencilStateCreateInfo depthStencil( { }, true, true,
        vk::CompareOp::eLessOrEqual, false, false, stencilOpState, stencilOpState,
        0.0f, 0.0f );
      vk::PipelineColorBlendAttachmentState colorBlendAttachment( false,
        vk::BlendFactor::eZero, vk::BlendFactor::eZero, vk::BlendOp::eAdd,
        vk::BlendFactor::eZero, vk::BlendFactor::eZero, vk::BlendOp::eAdd,
        vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
        vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA );
      PipelineColorBlendStateCreateInfo colorBlend( false, vk::LogicOp::eNoOp,
        colorBlendAttachment, { 1.0f, 1.0f, 1.0f, 1.0f } );
      PipelineDynamicStateCreateInfo dynamic( {
        vk::DynamicState::eViewport, vk::DynamicState::eScissor
      } );

      pipelines.write = device->createGraphicsPipeline(
        _window->pipelineCache( ), { }, { vertexStage, fragmentStage },
        vertexInput, assembly, nullptr, viewport, rasterization, multisample,
        depthStencil, colorBlend, dynamic, pipelineLayouts.write,
        _window->renderPass( ) );

      std::vector<WriteDescriptorSet> wdss =
      {
        WriteDescriptorSet(
          descriptorSets.write, 0, 0, vk::DescriptorType::eUniformBuffer,
          1, nullptr, DescriptorBufferInfo( uniformBuffer1, 0, sizeof( ubo1 ) )
        )
      };
      device->updateDescriptorSets( wdss, { } );
    }
  
    {
      auto vertexStage = device->createShaderPipelineShaderStage(
        POMPEII_EXAMPLES_SPV_ROUTE + std::string( "inputAttachmentResolve_vert.spv" ),
        vk::ShaderStageFlagBits::eVertex
      );
      auto fragmentStage = device->createShaderPipelineShaderStage(
        POMPEII_EXAMPLES_SPV_ROUTE + std::string( "inputAttachmentResolve_frag.spv" ),
        vk::ShaderStageFlagBits::eFragment
      );

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

      pipelines.read = device->createGraphicsPipeline( _window->pipelineCache( ), { },
      { vertexStage, fragmentStage }, vertexInput, assembly, nullptr,
        viewport, rasterization, multisample, depthStencil, colorBlend, dynamic,
        pipelineLayouts.read, _window->renderPass( ), 1 );

      updateDescriptorsReadStage( );
    }
  }

  virtual void initSwapChainResources( void ) override
  {
    updateDescriptorsReadStage( );
  }

  void updateDescriptorsReadStage( void)
  {
    auto device = _window->device( );

    uint32_t swapCount = _window->framebuffers( ).size( );
    for ( uint32_t i = 0; i < swapCount; ++i )
    {
      std::vector<WriteDescriptorSet> wdss =
      {
        WriteDescriptorSet(
          descriptorSets.read[ i ], 0, 0, vk::DescriptorType::eInputAttachment, 1,
          DescriptorImageInfo( vk::ImageLayout::eShaderReadOnlyOptimal,
            renderpassAttachments[ i ].color.imageView, nullptr
          ), nullptr
        ),
        WriteDescriptorSet(
          descriptorSets.read[ i ], 1, 0, vk::DescriptorType::eInputAttachment, 1,
          DescriptorImageInfo( vk::ImageLayout::eShaderReadOnlyOptimal,
            renderpassAttachments[ i ].depth.imageView, nullptr
          ), nullptr
        ),
        WriteDescriptorSet(
          descriptorSets.read[ i ], 2, 0, vk::DescriptorType::eUniformBuffer,
          1, nullptr, DescriptorBufferInfo( uniformBuffer2, 0, sizeof( ubo2 ) )
        )
      };
      device->updateDescriptorSets( wdss, { } );
    }
  }

  void updateMVP( void )
  {
    auto size = _window->swapchainImageSize( );

    uint32_t width = size.width, height = size.height;

    static auto startTime = std::chrono::high_resolution_clock::now( );

    auto currentTime = std::chrono::high_resolution_clock::now( );
    float time = std::chrono::duration_cast<std::chrono::milliseconds>( currentTime - startTime ).count( ) / 1000.0f;

    ubo1.model = glm::rotate( glm::mat4( 1.0f ),
      time * 0.5f * glm::radians( 90.0f ), glm::vec3( 0.0f, 1.0f, 0.0f )
    );
    glm::vec3 cameraPos = glm::vec3( 0.0f, 0.0f, 0.5f );
    glm::vec3 cameraFront = glm::vec3( 0.0f, 0.0f, -1.0f );
    glm::vec3 cameraUp = glm::vec3( 0.0f, 1.0f, 0.0f );

    cameraPos.z += std::abs( 2.5f * sin( time * 0.25f ) );

    ubo1.view = glm::lookAt( cameraPos, cameraPos + cameraFront, cameraUp );
    ubo1.projection = glm::perspective( glm::radians( 45.0f ), width / ( float ) height, 0.1f, 10.0f );
    ubo1.projection[ 1 ][ 1 ] *= -1;

    ubo1.lightPos = glm::vec3( 1.0f, 1.0f, 1.0f );

    uniformBuffer1->set( &ubo1 );

    uniformBuffer2->set( &ubo2 );
  }

  virtual void nextFrame( void )
  {
    auto cmd = _window->currentCommandBuffer( );

    updateMVP( );

    std::array<vk::ClearValue, 3 > clearValues;
    std::array<float, 4> ccv = { 1.0f, 1.0f, 1.0f, 1.0f };

    clearValues[ 0 ].color = vk::ClearColorValue( ccv );
    clearValues[ 1 ].color = vk::ClearColorValue( ccv );
    clearValues[ 2 ].depthStencil = vk::ClearDepthStencilValue( 1.0f, 0 );

    vk::Extent2D extent = _window->swapchainImageSize( );

    cmd->beginRenderPass( _window->renderPass( ),
      _window->framebuffer( ),
      vk::Rect2D( { 0, 0 }, extent ), clearValues,
      vk::SubpassContents::eInline );

    cmd->setViewportScissors( extent );
    /*
      First sub pass
      Fills the attachments
    */
    {
      cmd->bindGraphicsPipeline( pipelines.write );
      cmd->bindDescriptorSets( vk::PipelineBindPoint::eGraphics, pipelineLayouts.write, 0, descriptorSets.write );
      geometry->render( cmd );
    }

    /*
      Second sub pass
      Render a full screen quad, reading from the previously written attachments via input attachments
    */
    {
      cmd->nextSubpass( vk::SubpassContents::eInline );

      uint32_t currentIndex = _window->currentIndex( );
      cmd->bindGraphicsPipeline( pipelines.read );
      cmd->bindDescriptorSets( vk::PipelineBindPoint::eGraphics,
        pipelineLayouts.read, 0, { descriptorSets.read[ currentIndex ] }, nullptr );
      cmd->draw( 4, 1, 0, 0 );
    }

    cmd->endRenderPass( );

    _window->frameReady( );
  }
};

class VulkanWindow : public glfw::VulkanWindow
{
protected:
  MainWindowRenderer* _renderer;
public:
  explicit VulkanWindow( int width, int height,
    const std::string& title, bool enableLayers )
    : glfw::VulkanWindow( width, height, title, enableLayers )
  {
  }

  virtual ~VulkanWindow( void )
  {
    renderpassAttachments.clear( );
  }

  void createAttachment( vk::Format format, 
    vk::ImageUsageFlags usage, FramebufferAttachment* attachment )
  {
    auto size = swapchainImageSize( );
    uint32_t width = size.width,
             height = size.height;

    vk::ImageAspectFlags aspectMask;
    //vk::ImageLayout imageLayout;

    attachment->format = format;

    if ( usage & vk::ImageUsageFlagBits::eColorAttachment )
    {
      aspectMask = vk::ImageAspectFlagBits::eColor;
      //imageLayout = vk::ImageLayout::eColorAttachmentOptimal;
    }
    if ( usage & vk::ImageUsageFlagBits::eDepthStencilAttachment )
    {
      aspectMask = vk::ImageAspectFlagBits::eDepth;// |
      //  vk::ImageAspectFlagBits::eStencil;
      //imageLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;
    }

    //assert( aspectMask > 0 );

    attachment->image = device( )->createImage( vk::ImageCreateFlagBits( ),
      vk::ImageType::e2D, format, vk::Extent3D( width, height, 1 ), 1, 1,
      vk::SampleCountFlagBits::e1, vk::ImageTiling::eOptimal,
      // VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT flag is required for input attachments;
      usage | vk::ImageUsageFlagBits::eInputAttachment, vk::SharingMode::eExclusive,
      { }, vk::ImageLayout::eUndefined,
      vk::MemoryPropertyFlagBits::eDeviceLocal
    );

    attachment->imageView = attachment->image->createImageView( 
      vk::ImageViewType::e2D,
      format, {
        vk::ComponentSwizzle::eR, vk::ComponentSwizzle::eG,
        vk::ComponentSwizzle::eB, vk::ComponentSwizzle::eA
      },
      vk::ImageSubresourceRange( )
        .setAspectMask( aspectMask )
        .setBaseMipLevel( 0 )
        .setLevelCount( 1 )
        .setBaseArrayLayer( 0 )
        .setLayerCount( 1 )
    );
  }

  virtual glfw::VulkanWindowRenderer* createRenderer( void ) override
  {
    _renderer = new MainWindowRenderer( this );
    return _renderer;
  }

  void createBufferAttachments( vk::Format colorF )
  {
    uint32_t swapCount = _swapchain->imageViews( ).size( );
    renderpassAttachments.resize( swapCount );
    for ( uint32_t i = 0; i < swapCount; ++i )
    {
      createAttachment(colorF, vk::ImageUsageFlagBits::eColorAttachment, 
        &renderpassAttachments[i].color);
      createAttachment(depthStencilFormat( ), vk::ImageUsageFlagBits::eDepthStencilAttachment,
        &renderpassAttachments[i].depth);
    }
  }

  virtual void setupRenderPass( void ) override
  {
    vk::Format colorF = vk::Format::eR8G8B8A8Unorm;
    createBufferAttachments( colorF );

    std::array<vk::AttachmentDescription, 3> attachments;
    // Swap chain image color attachment
    // Will be transitioned to present layout
    attachments[ 0 ].format = colorFormat( );
    attachments[ 0 ].samples = vk::SampleCountFlagBits::e1;
    attachments[ 0 ].loadOp = vk::AttachmentLoadOp::eClear;
    attachments[ 0 ].storeOp = vk::AttachmentStoreOp::eStore;
    attachments[ 0 ].stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
    attachments[ 0 ].stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
    attachments[ 0 ].initialLayout = vk::ImageLayout::eUndefined;
    attachments[ 0 ].finalLayout = vk::ImageLayout::ePresentSrcKHR;

    // Input attachments
    // These will be written in the first subpass, transitioned to input attachments 
    // and then read in the secod subpass

    // Color
    attachments[ 1 ].format = colorF;
    attachments[ 1 ].samples = vk::SampleCountFlagBits::e1;
    attachments[ 1 ].loadOp = vk::AttachmentLoadOp::eClear;
    attachments[ 1 ].storeOp = vk::AttachmentStoreOp::eStore;
    attachments[ 1 ].stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
    attachments[ 1 ].stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
    attachments[ 1 ].initialLayout = vk::ImageLayout::eUndefined;
    attachments[ 1 ].finalLayout = vk::ImageLayout::eColorAttachmentOptimal;

    // Depth
    attachments[ 2 ].format = depthStencilFormat( );
    attachments[ 2 ].samples = vk::SampleCountFlagBits::e1;
    attachments[ 2 ].loadOp = vk::AttachmentLoadOp::eClear;
    attachments[ 2 ].storeOp = vk::AttachmentStoreOp::eDontCare;
    attachments[ 2 ].stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
    attachments[ 2 ].stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
    attachments[ 2 ].initialLayout = vk::ImageLayout::eUndefined;
    attachments[ 2 ].finalLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

    std::array<vk::SubpassDescription, 2> subpassDescriptions;

    /*
      First subpass
      Fill the color and depth attachments
    */
    vk::AttachmentReference colorReference = { 1, vk::ImageLayout::eColorAttachmentOptimal };
    vk::AttachmentReference depthReference = { 2, vk::ImageLayout::eDepthStencilAttachmentOptimal };

    subpassDescriptions[0].pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
    subpassDescriptions[0].colorAttachmentCount = 1;
    subpassDescriptions[0].pColorAttachments = &colorReference;
    subpassDescriptions[0].pDepthStencilAttachment = &depthReference;

    /*
      Second subpass
      Input attachment read and swap chain color attachment write
    */

    // Color reference (target) for this sub pass is the swap chain color attachment
    vk::AttachmentReference colorReferenceSwapchain = { 0, vk::ImageLayout::eColorAttachmentOptimal };

    subpassDescriptions[1].pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
    subpassDescriptions[1].colorAttachmentCount = 1;
    subpassDescriptions[1].pColorAttachments = &colorReferenceSwapchain;

    // Color and depth attachment written to in first sub pass will be used as input attachments to be read in the fragment shader
    vk::AttachmentReference inputReferences[2];
    inputReferences[0] = { 1, vk::ImageLayout::eShaderReadOnlyOptimal };
    inputReferences[1] = { 2, vk::ImageLayout::eShaderReadOnlyOptimal };
    
    // Use the attachments filled in the first pass as input attachments
    subpassDescriptions[1].inputAttachmentCount = 2;
    subpassDescriptions[1].pInputAttachments = inputReferences;

    /*
      Subpass dependencies for layout transitions
    */
    std::array<vk::SubpassDependency, 3> dependencies;

    dependencies[ 0 ].srcSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[ 0 ].dstSubpass = 0;
    dependencies[ 0 ].srcStageMask = vk::PipelineStageFlagBits::eBottomOfPipe;
    dependencies[ 0 ].dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
    dependencies[ 0 ].srcAccessMask = vk::AccessFlagBits::eMemoryRead;
    dependencies[ 0 ].dstAccessMask = vk::AccessFlagBits::eColorAttachmentRead
      | vk::AccessFlagBits::eColorAttachmentWrite;
    dependencies[ 0 ].dependencyFlags = vk::DependencyFlagBits::eByRegion;

    // This dependency transitions the input attachment from color attachment to shader read
    dependencies[ 1 ].srcSubpass = 0;
    dependencies[ 1 ].dstSubpass = 1;
    dependencies[ 1 ].srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
    dependencies[ 1 ].dstStageMask = vk::PipelineStageFlagBits::eFragmentShader;
    dependencies[ 1 ].srcAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
    dependencies[ 1 ].dstAccessMask = vk::AccessFlagBits::eShaderRead;;
    dependencies[ 1 ].dependencyFlags = vk::DependencyFlagBits::eByRegion;

    dependencies[ 2 ].srcSubpass = 0;
    dependencies[ 2 ].dstSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[ 2 ].srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
    dependencies[ 2 ].dstStageMask = vk::PipelineStageFlagBits::eBottomOfPipe;
    dependencies[ 2 ].srcAccessMask = vk::AccessFlagBits::eColorAttachmentRead
      | vk::AccessFlagBits::eColorAttachmentWrite;
    dependencies[ 2 ].dstAccessMask = vk::AccessFlagBits::eMemoryRead;
    dependencies[ 2 ].dependencyFlags = vk::DependencyFlagBits::eByRegion;

    _renderPass = device( )->createRenderPass( attachments, subpassDescriptions, dependencies );
  }
  
  // Override framebuffer setup from base class
  virtual void setupFramebuffer( void ) override
  {
    vk::Extent2D extent = _swapchain->extent( );

    uint32_t swapCount = _swapchain->imageViews( ).size( );

    _framebuffers.reserve( swapCount );
    for ( size_t i = 0; i < swapCount; ++i )
    {
      _framebuffers.push_back( device( )->createFramebuffer( _renderPass,
      { 
        _swapchain->imageViews( )[ i ], 
        renderpassAttachments[ i ].color.imageView, 
        renderpassAttachments[ i ].depth.imageView
      }, extent, 1 ) );
    }

    std::cout << "Framebuffer Swapchain OK" << std::endl;
  }

  virtual void keyEvent( int key, int act ) override
  {
    if ( act == GLFW_PRESS )
    {
      if ( key == GLFW_KEY_Z )
      {
        _renderer->ubo2.attachmentIndex = 0;
      }
      else if ( key == GLFW_KEY_X )
      {
        _renderer->ubo2.attachmentIndex = 1;
      }
    }
  }
};


int main( int, char** )
{
  VulkanWindow app( 500, 500, "GLFWRenderer", true );
  app.show( );
  return EXIT_SUCCESS;
}