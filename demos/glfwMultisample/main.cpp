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

#include <routes.h>

vk::SampleCountFlagBits mySampleCount = vk::SampleCountFlagBits::e1;

class MainWindowRenderer : public glfw::VulkanWindowRenderer
{
private:
  glfw::VulkanWindow* _window;
public:
  MainWindowRenderer( glfw::VulkanWindow* window )
    : _window( window )
  {
  }

  virtual void initResources( void ) override
  {
    auto device = _window->device( );

    tex = device->createTexture2D( POMPEII_EXAMPLES_IMAGES_ROUTE +
      std::string( "aliasing.png" ), _window->gfxCommandPool( ),
      _window->gfxQueue( ), vk::Format::eR8G8B8A8Unorm );

    std::vector<DescriptorSetLayoutBinding> dslbs =
    {
      DescriptorSetLayoutBinding( 0,
      vk::DescriptorType::eCombinedImageSampler,
      vk::ShaderStageFlagBits::eFragment
      )
    };

    descriptorSetLayout = device->createDescriptorSetLayout( dslbs );
    pipelineLayout = device->createPipelineLayout( descriptorSetLayout );

    std::shared_ptr<DescriptorPool> descriptorPool =
      device->createDescriptorPool( 1, {
        { vk::DescriptorType::eCombinedImageSampler, 1 }
      } );

    // Init descriptor set
    descriptorSet = device->allocateDescriptorSet( descriptorPool, descriptorSetLayout );

    // We can't updateDescriptorSet while the device is using it
    _window->device( )->waitIdle( );
    std::vector<WriteDescriptorSet> wdss =
    {
      WriteDescriptorSet( descriptorSet, 0, 0,
        vk::DescriptorType::eCombinedImageSampler, 1,
        tex->descriptor, nullptr
      )
    };
    _window->device( )->updateDescriptorSets( wdss, { } );

    auto vertexStage = device->createShaderPipelineShaderStage(
      POMPEII_EXAMPLES_SPV_ROUTE + std::string( "fullquad_vert.spv" ),
      vk::ShaderStageFlagBits::eVertex
    );
    auto fragmentStage = device->createShaderPipelineShaderStage(
      POMPEII_EXAMPLES_SPV_ROUTE + std::string( "fullquad_frag.spv" ),
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
      mySampleCount, false, 0.0f, nullptr, false, false
    );
    // MSAA with sample shading pipeline
    // Sample shading enables per-sample shading to avoid shader aliasing and smooth out e.g. high frequency texture maps
    // Note: This will trade performance for are more stable image
    multisample.sampleShadingEnable = VK_TRUE;				// Enable per-sample shading (instead of per-fragment)
    multisample.minSampleShading = 0.25f;					// Minimum fraction for sample shading


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

    pipeline = device->createGraphicsPipeline( _window->pipelineCache( ), { },
    { vertexStage, fragmentStage }, vertexInput, assembly, nullptr,
      viewport, rasterization, multisample, depthStencil, colorBlend, dynamic,
      pipelineLayout, _window->renderPass( ) );
  }

  virtual void nextFrame( void )
  {
    auto cmd = _window->currentCommandBuffer( );

    static auto startTime = std::chrono::high_resolution_clock::now( );

    auto currentTime = std::chrono::high_resolution_clock::now( );
    float time = std::chrono::duration_cast< std::chrono::milliseconds >(
      currentTime - startTime ).count( ) / 1000.0f;

    float _red = sin( time ) * 0.5f + 0.5f;
    float _blue = cos( time ) * 0.5f + 0.5f;

    std::array<vk::ClearValue, 3 > clearValues;
    clearValues[ 0 ] = pompeii::utils::getClearValueColor( _red, 0.0f, _blue );
    clearValues[ 1 ] = pompeii::utils::getClearValueColor( _red, 0.0f, _blue );
    clearValues[ 2 ] = pompeii::utils::getClearValueDepth( );

    vk::Extent2D extent = _window->swapchainImageSize( );

    cmd->beginRenderPass( _window->renderPass( ),
      _window->framebuffer( ),
      vk::Rect2D( { 0, 0 }, extent ), clearValues,
      vk::SubpassContents::eInline );

    cmd->bindGraphicsPipeline( pipeline );
    cmd->bindDescriptorSets( vk::PipelineBindPoint::eGraphics,
      pipelineLayout, 0, { descriptorSet }, nullptr );

    cmd->setViewportScissors( extent );
    cmd->draw( 4, 1, 0, 0 );

    cmd->endRenderPass( );

    _window->frameReady( );
  }

  std::shared_ptr< DescriptorSet > descriptorSet;
  std::shared_ptr< DescriptorSetLayout > descriptorSetLayout;
  std::shared_ptr< PipelineLayout > pipelineLayout;
  std::shared_ptr< Pipeline > pipeline;
  std::shared_ptr< Texture > tex;
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

  vk::SampleCountFlagBits getMaxUsableSampleCount( void )
  {
    vk::SampleCountFlags counts = vk::SampleCountFlagBits::e4;/* std::min(
      physicalDevice()->getDeviceProperties( ).limits.framebufferColorSampleCounts, 
      physicalDevice( )->getDeviceProperties( ).limits.framebufferDepthSampleCounts );*/
    if ( counts & vk::SampleCountFlagBits::e64 ) { return vk::SampleCountFlagBits::e64; }
    if ( counts & vk::SampleCountFlagBits::e32 ) { return vk::SampleCountFlagBits::e32; }
    if ( counts & vk::SampleCountFlagBits::e16 ) { return vk::SampleCountFlagBits::e16; }
    if ( counts & vk::SampleCountFlagBits::e8 ) { return vk::SampleCountFlagBits::e8; }
    if ( counts & vk::SampleCountFlagBits::e4 ) { return vk::SampleCountFlagBits::e4; }
    if ( counts & vk::SampleCountFlagBits::e32 ) { return vk::SampleCountFlagBits::e2; }
    return vk::SampleCountFlagBits::e1;
  }

  virtual void setupRenderPass( void ) override
  {
    mySampleCount = getMaxUsableSampleCount( );

    std::array<vk::AttachmentDescription, 4> attachments;
    // Multisampled attachment that we render to
    attachments[ 0 ].format = colorFormat( );
    attachments[ 0 ].samples = mySampleCount;
    attachments[ 0 ].loadOp = vk::AttachmentLoadOp::eClear;
    attachments[ 0 ].storeOp = vk::AttachmentStoreOp::eStore;
    attachments[ 0 ].stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
    attachments[ 0 ].stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
    attachments[ 0 ].initialLayout = vk::ImageLayout::eUndefined;
    attachments[ 0 ].finalLayout = vk::ImageLayout::eColorAttachmentOptimal;

    // This is the frame buffer attachment to where the multisampled image
    // will be resolved to and which will be presented to the swapchain
    attachments[ 1 ].format = colorFormat( );
    attachments[ 1 ].samples = vk::SampleCountFlagBits::e1;
    attachments[ 1 ].loadOp = vk::AttachmentLoadOp::eDontCare;
    attachments[ 1 ].storeOp = vk::AttachmentStoreOp::eStore;
    attachments[ 1 ].stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
    attachments[ 1 ].stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
    attachments[ 1 ].initialLayout = vk::ImageLayout::eUndefined;
    attachments[ 1 ].finalLayout = vk::ImageLayout::ePresentSrcKHR;

    // Multisampled depth attachment we render to
    attachments[ 2 ].format = depthStencilFormat( );
    attachments[ 2 ].samples = mySampleCount;
    attachments[ 2 ].loadOp = vk::AttachmentLoadOp::eClear;
    attachments[ 2 ].storeOp = vk::AttachmentStoreOp::eDontCare;
    attachments[ 2 ].stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
    attachments[ 2 ].stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
    attachments[ 2 ].initialLayout = vk::ImageLayout::eUndefined;
    attachments[ 2 ].finalLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

    // Depth resolve attachment
    attachments[ 3 ].format = depthStencilFormat( );
    attachments[ 3 ].samples = vk::SampleCountFlagBits::e1;
    attachments[ 3 ].loadOp = vk::AttachmentLoadOp::eDontCare;
    attachments[ 3 ].storeOp = vk::AttachmentStoreOp::eStore;
    attachments[ 3 ].stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
    attachments[ 3 ].stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
    attachments[ 3 ].initialLayout = vk::ImageLayout::eUndefined;
    attachments[ 3 ].finalLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

    vk::AttachmentReference colorReference;
    colorReference.attachment = 0;
    colorReference.layout = vk::ImageLayout::eColorAttachmentOptimal;

    vk::AttachmentReference depthReference;
    depthReference.attachment = 2;
    depthReference.layout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

    // Resolve attachment reference for the color attachment
    vk::AttachmentReference resolveReference = { };
    resolveReference.attachment = 1;
    resolveReference.layout = vk::ImageLayout::eColorAttachmentOptimal;

    vk::SubpassDescription subpass = { };
    subpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorReference;
    // Pass our resolve attachments to the sub pass
    subpass.pResolveAttachments = &resolveReference;
    subpass.pDepthStencilAttachment = &depthReference;

    std::array<vk::SubpassDependency, 2> dependencies;

    dependencies[ 0 ].srcSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[ 0 ].dstSubpass = 0;
    dependencies[ 0 ].srcStageMask = vk::PipelineStageFlagBits::eBottomOfPipe;
    dependencies[ 0 ].dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
    dependencies[ 0 ].srcAccessMask = vk::AccessFlagBits::eMemoryRead;
    dependencies[ 0 ].dstAccessMask = vk::AccessFlagBits::eColorAttachmentRead
      | vk::AccessFlagBits::eColorAttachmentWrite;
    dependencies[ 0 ].dependencyFlags = vk::DependencyFlagBits::eByRegion;

    dependencies[ 1 ].srcSubpass = 0;
    dependencies[ 1 ].dstSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[ 1 ].srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
    dependencies[ 1 ].dstStageMask = vk::PipelineStageFlagBits::eBottomOfPipe;
    dependencies[ 1 ].srcAccessMask = vk::AccessFlagBits::eColorAttachmentRead
      | vk::AccessFlagBits::eColorAttachmentWrite;
    dependencies[ 1 ].dstAccessMask = vk::AccessFlagBits::eMemoryRead;
    dependencies[ 1 ].dependencyFlags = vk::DependencyFlagBits::eByRegion;

    _renderPass = device( )->createRenderPass( attachments, subpass, dependencies );
  }
  
  struct
  {
    struct
    {
      std::shared_ptr<pompeii::Image> image;
      std::shared_ptr<pompeii::ImageView> view;
    } color;
    struct
    {
      std::shared_ptr<pompeii::Image> image;
      std::shared_ptr<pompeii::ImageView> view;
    } depth;
  } multisampleTarget;

  // Creates a multi sample render target (image and view) that is used to resolve 
  // into the visible frame buffer target in the render pass
  void setupMultisampleTarget( )
  {
    auto size = swapchainImageSize( );
    uint32_t width = size.width, 
             height = size.height;

    //auto deviceProperties = physicalDevice( )->getDeviceProperties( );
    // Check if device supports requested sample count for color and depth frame buffer
    //assert( ( deviceProperties.limits.framebufferColorSampleCounts >= sampleCount ) && ( deviceProperties.limits.framebufferDepthSampleCounts >= sampleCount ) );

    // Color target
    multisampleTarget.color.image = device( )->createImage( { }, vk::ImageType::e2D,
      colorFormat( ), vk::Extent3D( width, height, 1 ), 1, 1, mySampleCount, vk::ImageTiling::eOptimal,
      // Image will only be used as a transient target
      vk::ImageUsageFlagBits::eTransientAttachment | vk::ImageUsageFlagBits::eColorAttachment,
      vk::SharingMode::eExclusive, { }, vk::ImageLayout::eUndefined, 
      /*VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT*/vk::MemoryPropertyFlagBits::eDeviceLocal );

    // Create image view for the MSAA target
    multisampleTarget.color.view = multisampleTarget.color.image->createImageView( 
      vk::ImageViewType::e2D, colorFormat( ), vk::ImageAspectFlagBits::eColor );

    // Depth target
    multisampleTarget.depth.image = device( )->createImage( { }, vk::ImageType::e2D,
      depthStencilFormat( ), vk::Extent3D( width, height, 1 ), 1, 1, mySampleCount, vk::ImageTiling::eOptimal,
      // Image will only be used as a transient target
      vk::ImageUsageFlagBits::eTransientAttachment | vk::ImageUsageFlagBits::eDepthStencilAttachment,
      vk::SharingMode::eExclusive, { }, vk::ImageLayout::eUndefined,
      /*VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT*/vk::MemoryPropertyFlagBits::eDeviceLocal );

    // Create image view for the MSAA target
    multisampleTarget.depth.view = multisampleTarget.depth.image->createImageView(
      vk::ImageViewType::e2D, depthStencilFormat( ),
      vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil );
  }

  virtual void setupFramebuffer( void ) override
  {
    // Overrides the virtual function of the base class
    setupMultisampleTarget( );

    vk::Extent2D extent = _swapchain->extent( );

    _framebuffers.reserve( _swapchain->imageViews( ).size( ) );
    for ( size_t i = 0, l = _swapchain->imageViews( ).size( ); i < l; ++i )
    {
      _framebuffers.push_back( device( )->createFramebuffer( _renderPass,
      { 
        multisampleTarget.color.view, 
        _swapchain->imageViews( )[ i ], 
        multisampleTarget.depth.view,
        _depthView
      }, extent, 1 ) );
    }

    std::cout << "Framebuffer Swapchain OK" << std::endl;
  }
};


int main( int, char** )
{
  VulkanWindow app( 422, 627, "GLFWRenderer", true );
  app.show( );
  return EXIT_SUCCESS;
}