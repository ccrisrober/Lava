#include <iostream>

#include <glfwLava/glfwLava.h>

using namespace lava;

#include <glm/glm.hpp>

#include <random>

#include "../utils/Camera.h"

const size_t NUM_LIGHTS = 64;

class MainWindowRenderer : public lava::GLFWVulkanWindowRenderer
{
private:
  lava::GLFWVulkanWindow* _window;

  Camera camera;

  struct Light
  {
    glm::vec4 position;
    glm::vec3 color;
    float radius;
  };

  struct
  {
    glm::vec4 viewPos;
    Light lights[ NUM_LIGHTS ];
  } uboLights;

  struct
  {
    glm::mat4 projection;
    glm::mat4 model;
    glm::mat4 view;
  } uboGBuffer;

  struct
  {
    std::shared_ptr< lava::UniformBuffer > GBuffer;
    std::shared_ptr< lava::UniformBuffer > lights;
  } uniformBuffers;

  struct
  {
    std::shared_ptr< Pipeline > scene;
    std::shared_ptr< Pipeline > composition;
  } pipelineLayouts;

  struct
  {
    std::shared_ptr< DescriptorSet > scene;
    std::shared_ptr< DescriptorSet > composition;
  } descriptorSets;

  struct
  {
    std::shared_ptr< DescriptorSetLayout > scene;
    std::shared_ptr< DescriptorSetLayout > composition;
  } descriptorSetLayouts;

  // G-Buffer framebuffer attachments
  struct FramebufferAttachment
  {
    std::shared_ptr< Image > image;
    std::shared_ptr< ImageView > view;
    vk::Format format;
  };

  struct
  {
    FramebufferAttachment position;
    FramebufferAttachment normal;
    FramebufferAttachment albedo;
  } attachments;
public:
  MainWindowRenderer( lava::GLFWVulkanWindow* window )
    : _window( window )
  {
  }
protected:
  std::vector<std::shared_ptr<Framebuffer> > frameBuffers;
  virtual void setupFramebuffer( void )// override
  {
    std::vector<std::shared_ptr<lava::ImageView>> attachments;

    /*VkFramebufferCreateInfo frameBufferCreateInfo = { };
    frameBufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    frameBufferCreateInfo.pNext = NULL;
    frameBufferCreateInfo.renderPass = renderPass;
    frameBufferCreateInfo.attachmentCount = 5;
    frameBufferCreateInfo.pAttachments = attachments;
    frameBufferCreateInfo.width = width;
    frameBufferCreateInfo.height = height;
    frameBufferCreateInfo.layers = 1;

    // Create frame buffers for every swap chain image
    frameBuffers.resize( swapChain.imageCount );
    for ( uint32_t i = 0; i < frameBuffers.size( ); i++ )
    {
      //attachments[ 0 ] = swapChain.buffers[ i ].view;
      attachments[ 1 ] = this->attachments.position.view;
      attachments[ 2 ] = this->attachments.normal.view;
      attachments[ 3 ] = this->attachments.albedo.view;
      attachments[ 4 ] = depthStencil.view;
      VK_CHECK_RESULT( vkCreateFramebuffer( device, &frameBufferCreateInfo, nullptr, &frameBuffers[ i ] ) );
    }*/
  }
  virtual void setupRenderPass( void )// override
  {
    createGBufferAttachments( );
    std::array<vk::AttachmentDescription, 5> attachments = {
      vk::AttachmentDescription( vk::AttachmentDescriptionFlags( ),
        _window->colorFormat( ), vk::SampleCountFlagBits::e1,
        vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eStore,
        vk::AttachmentLoadOp::eDontCare, vk::AttachmentStoreOp::eDontCare,
        vk::ImageLayout::eUndefined, vk::ImageLayout::ePresentSrcKHR
      ),
      vk::AttachmentDescription( vk::AttachmentDescriptionFlags( ),
        this->attachments.position.format, vk::SampleCountFlagBits::e1,
        vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eStore,
        vk::AttachmentLoadOp::eDontCare, vk::AttachmentStoreOp::eDontCare,
        vk::ImageLayout::eUndefined, vk::ImageLayout::eColorAttachmentOptimal
      ),
      vk::AttachmentDescription( vk::AttachmentDescriptionFlags( ),
        this->attachments.normal.format, vk::SampleCountFlagBits::e1,
        vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eStore,
        vk::AttachmentLoadOp::eDontCare, vk::AttachmentStoreOp::eDontCare,
        vk::ImageLayout::eUndefined, vk::ImageLayout::eColorAttachmentOptimal
      ),
      vk::AttachmentDescription( vk::AttachmentDescriptionFlags( ),
        this->attachments.albedo.format, vk::SampleCountFlagBits::e1,
        vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eStore,
        vk::AttachmentLoadOp::eDontCare, vk::AttachmentStoreOp::eDontCare,
        vk::ImageLayout::eUndefined, vk::ImageLayout::eColorAttachmentOptimal
      ),
      vk::AttachmentDescription( vk::AttachmentDescriptionFlags( ),
        _window->depthStencilFormat( ), vk::SampleCountFlagBits::e1,
        vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eStore,
        vk::AttachmentLoadOp::eDontCare, vk::AttachmentStoreOp::eDontCare,
        vk::ImageLayout::eUndefined, vk::ImageLayout::eDepthStencilAttachmentOptimal
      )
    };

    // Two subpasses
    std::array < vk::SubpassDescription, 2 > subpassDescription;
    
    // First subpass: Fill G-Buffer components
    vk::AttachmentReference colorRefs[ 4 ] =
    {
      { 0, vk::ImageLayout::eColorAttachmentOptimal },
      { 1, vk::ImageLayout::eColorAttachmentOptimal },
      { 2, vk::ImageLayout::eColorAttachmentOptimal },
      { 3, vk::ImageLayout::eColorAttachmentOptimal }
    };
    vk::AttachmentReference depthRef = { 4, vk::ImageLayout::eDepthStencilAttachmentOptimal };

    subpassDescription[ 0 ].pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
    subpassDescription[ 0 ].colorAttachmentCount = 4;
    subpassDescription[ 0 ].pColorAttachments = colorRefs;
    subpassDescription[ 0 ].pDepthStencilAttachment = &depthRef;

    // Second subpass: Final composition (using G-Buffer componentes)
    vk::AttachmentReference colorRef = { 0, vk::ImageLayout::eColorAttachmentOptimal };

    vk::AttachmentReference inputReferences[ 3 ] = {
      { 1, vk::ImageLayout::eShaderReadOnlyOptimal },
      { 2, vk::ImageLayout::eShaderReadOnlyOptimal },
      { 3, vk::ImageLayout::eShaderReadOnlyOptimal }
    };

    //uint32_t preserveAttachmentIndex = 1;

    subpassDescription[ 1 ].pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
    subpassDescription[ 1 ].colorAttachmentCount = 1;
    subpassDescription[ 1 ].pColorAttachments = &colorRef;
    subpassDescription[ 1 ].pDepthStencilAttachment = &depthRef;
    // Use the color attachments filled in the first pass as input attachments
    subpassDescription[ 1 ].inputAttachmentCount = 3;
    subpassDescription[ 1 ].pInputAttachments = inputReferences;

    // Subpass dependencies for layout transitions
    std::array<vk::SubpassDependency, 3> dependencies;

    dependencies[ 0 ].srcSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[ 0 ].dstSubpass = 0;
    dependencies[ 0 ].srcStageMask = vk::PipelineStageFlagBits::eBottomOfPipe;
    dependencies[ 0 ].dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
    dependencies[ 0 ].srcAccessMask = vk::AccessFlagBits::eMemoryRead;
    dependencies[ 0 ].dstAccessMask = vk::AccessFlagBits::eColorAttachmentRead | 
      vk::AccessFlagBits::eColorAttachmentWrite;
    dependencies[ 0 ].dependencyFlags = vk::DependencyFlagBits::eByRegion;

    // This dependency transitions the input attachment from color attachment to shader read
    dependencies[ 1 ].srcSubpass = 0;
    dependencies[ 1 ].dstSubpass = 1;
    dependencies[ 1 ].srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
    dependencies[ 1 ].dstStageMask = vk::PipelineStageFlagBits::eFragmentShader;
    dependencies[ 1 ].srcAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
    dependencies[ 1 ].dstAccessMask = vk::AccessFlagBits::eShaderRead;
    dependencies[ 1 ].dependencyFlags = vk::DependencyFlagBits::eByRegion;

    dependencies[ 2 ].srcSubpass = 0;
    dependencies[ 2 ].dstSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[ 2 ].srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
    dependencies[ 2 ].dstStageMask = vk::PipelineStageFlagBits::eBottomOfPipe;
    dependencies[ 2 ].srcAccessMask = vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite;
    dependencies[ 2 ].dstAccessMask = vk::AccessFlagBits::eMemoryRead;
    dependencies[ 2 ].dependencyFlags = vk::DependencyFlagBits::eByRegion;

    auto renderPass = _window->device( )->createRenderPass( attachments, subpassDescription, dependencies );
  }
public:
  // Create color attachments for the G-Buffer components
  void createGBufferAttachments( void )
  {
    auto size = _window->swapchainImageSize( );
    uint32_t width = size.width,
             height = size.height;
    // (World space) Positions
    createAttachment( vk::Format::eR16G16B16A16Sfloat,
      vk::ImageUsageFlagBits::eColorAttachment, &attachments.position, 
      width, height );
    // (World space) Normals
    createAttachment( vk::Format::eR16G16B16A16Sfloat, 
      vk::ImageUsageFlagBits::eColorAttachment, &attachments.normal, 
      width, height );
    // Albedo (color)
    createAttachment( vk::Format::eR8G8B8A8Unorm, 
      vk::ImageUsageFlagBits::eColorAttachment, &attachments.albedo, 
      width, height );
  }

  void createAttachment( vk::Format format, vk::ImageUsageFlags usage,
    FramebufferAttachment* attachment, int width, int height )
  {
    vk::ImageAspectFlags aspectMask;
    vk::ImageLayout imageLayout;

    attachment->format = format;

    if ( usage & vk::ImageUsageFlagBits::eColorAttachment )
    {
      aspectMask = vk::ImageAspectFlagBits::eColor;
      imageLayout = vk::ImageLayout::eColorAttachmentOptimal;
    }
    if ( usage & vk::ImageUsageFlagBits::eDepthStencilAttachment )
    {
      aspectMask = vk::ImageAspectFlagBits::eDepth | 
        vk::ImageAspectFlagBits::eStencil;
      imageLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;
    }

    //assert( aspectMask > 0 );
    
    auto device = _window->device( );

    attachment->image = device->createImage( vk::ImageCreateFlagBits( ), 
      vk::ImageType::e2D, format, vk::Extent3D( width, height, 1 ), 1, 1, 
      vk::SampleCountFlagBits::e1, vk::ImageTiling::eOptimal, 
      usage | vk::ImageUsageFlagBits::eInputAttachment, vk::SharingMode( ), 
      { }, vk::ImageLayout::eUndefined,
      vk::MemoryPropertyFlagBits::eDeviceLocal
    );

    attachment->view = attachment->image->createImageView( vk::ImageViewType::e2D, 
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

  void prepareUniformBuffers( )
  {
    auto device = _window->device( );
    // Deferred vertex shader
    uniformBuffers.GBuffer = device->createUniformBuffer( sizeof( uboGBuffer ) );
    // Deferred fragment shader
    uniformBuffers.lights = device->createUniformBuffer( sizeof( uboLights ) );

    // update
    updateUniformBufferDeferredMatrices( );
    updateUniformBufferDeferredLights( );
  }

  void updateUniformBufferDeferredMatrices( void )
  {
    auto size = _window->swapchainImageSize( );

    uint32_t width = size.width, height = size.height;
    uboGBuffer.projection = glm::perspective( glm::radians( camera.Zoom ),
      ( float ) width / ( float ) height, 1.0f, 100.0f );;
    uboGBuffer.view = camera.GetViewMatrix( );
    uboGBuffer.model = glm::mat4( 1.0f );

    uniformBuffers.GBuffer->set( &uboGBuffer );
  }

  void updateUniformBufferDeferredLights( void )
  {
    // Current view position
    uboLights.viewPos = glm::vec4( camera.Position, 0.0f ) * glm::vec4( -1.0f, 1.0f, -1.0f, 1.0f );

    uniformBuffers.lights->set( &uboLights );
  }

  void initLights( void )
  {
    std::vector< glm::vec3 > colors = 
    {
      glm::vec3( 1.0f, 1.0f, 1.0f ),
      glm::vec3( 1.0f, 0.0f, 0.0f ),
      glm::vec3( 0.0f, 1.0f, 0.0f ),
      glm::vec3( 0.0f, 0.0f, 1.0f ),
      glm::vec3( 1.0f, 1.0f, 0.0f ),
    };

    std::default_random_engine rndGen( ( unsigned ) time( nullptr ) );
    std::uniform_real_distribution<float> rndDist( -1.0f, 1.0f );
    std::uniform_int_distribution<uint32_t> rndCol( 
      0, static_cast< uint32_t >( colors.size( ) - 1 ) );

    for ( auto& light : uboLights.lights )
    {
      light.position = glm::vec4( 
        rndDist( rndGen ) * 6.0f, 
        0.25f + std::abs( rndDist( rndGen ) ) * 4.0f, 
        rndDist( rndGen ) * 6.0f, 
        1.0f
      );
      light.color = colors[ rndCol( rndGen ) ];
      light.radius = 1.0f + std::abs( rndDist( rndGen ) );
    }
  }

  void prepareCompositionPass( )
  {
    auto device = _window->device( );
    std::vector<DescriptorSetLayoutBinding> dslb =
    {
      // Binding 0: Position input attachment
      DescriptorSetLayoutBinding( 0, 
        vk::DescriptorType::eInputAttachment,
        vk::ShaderStageFlagBits::eFragment
      ),
      // Binding 1: Normal input attachment 
      DescriptorSetLayoutBinding( 1,
        vk::DescriptorType::eInputAttachment,
        vk::ShaderStageFlagBits::eFragment
      ),
      // Binding 2: Albedo input attachment 
      DescriptorSetLayoutBinding( 2,
        vk::DescriptorType::eInputAttachment,
        vk::ShaderStageFlagBits::eFragment
      ),
      // Binding 3: Light positions
      DescriptorSetLayoutBinding( 3,
        vk::DescriptorType::eUniformBuffer,
        vk::ShaderStageFlagBits::eFragment
      )
    };

    auto descriptorLayout = device->createDescriptorSetLayout( dslb );

    auto pipLayout = device->createPipelineLayout( descriptorLayout );


    auto descriptorSet = device->allocateDescriptorSet( descriptorPool, descriptorLayout );

    std::vector<WriteDescriptorSet> wdss =
    {
      // Binding 0: Position texture target
      WriteDescriptorSet(
        descriptorSets.composition, 0, 0,
        vk::DescriptorType::eInputAttachment, 1, texPosition->descriptor, nullptr
      ),
      // Binding 1: Normals texture target
      WriteDescriptorSet(
        descriptorSets.composition, 1, 0,
        vk::DescriptorType::eInputAttachment, 1, texNormal->descriptor, nullptr
      ),
			// Binding 2: Albedo texture target
      WriteDescriptorSet(
        descriptorSets.composition, 2, 0,
        vk::DescriptorType::eInputAttachment, 1, texAlbedo->descriptor, nullptr
      ),
			// Binding 4: Fragment shader lights
      WriteDescriptorSet(
        descriptorSets.composition, 3, 0, vk::DescriptorType::eUniformBuffer,
        1, nullptr, DescriptorBufferInfo( uniformBuffers.lights, 0, sizeof( uboLights ) )
      )
    };
    device->updateDescriptorSets( wdss, { } );
  }

  void setupDescriptorPool( void )
  {
    std::vector<vk::DescriptorPoolSize> poolSizes = 
    {
      vk::DescriptorPoolSize( vk::DescriptorType::eUniformBuffer, 4 ),
      vk::DescriptorPoolSize( vk::DescriptorType::eCombinedImageSampler, 4 ),
      vk::DescriptorPoolSize( vk::DescriptorType::eInputAttachment, 4),
    };

    descriptorPool = _window->device( )->createDescriptorPool( 4, poolSizes );
  }

  virtual void initResources( void ) override
  {
    setupRenderPass( );

    initLights( );
    prepareUniformBuffers( );


    prepareCompositionPass( );

    setupDescriptorPool( );
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

    std::array<vk::ClearValue, 2 > clearValues;
    std::array<float, 4> ccv = { _red, 0.0f, _blue, 1.0f };

    clearValues[ 0 ].color = vk::ClearColorValue( ccv );
    clearValues[ 1 ].depthStencil = vk::ClearDepthStencilValue( 1.0f, 0 );

    vk::Extent2D extent = _window->swapchainImageSize( );

    cmd->beginRenderPass( _window->renderPass( ),
      _window->framebuffer( ),
      vk::Rect2D( { 0, 0 }, extent ), clearValues,
      vk::SubpassContents::eInline );

    cmd->endRenderPass( );

    _window->frameReady( );
  }
protected:
  std::shared_ptr< lava::DescriptorPool > descriptorPool;
  std::shared_ptr< lava::Texture > texPosition, texNormal, texAlbedo;
};

class VulkanWindow : public lava::GLFWVulkanWindow
{
public:
  explicit VulkanWindow( int width, int height,
    const std::string& title, bool enableLayers )
    : lava::GLFWVulkanWindow( width, height, title, enableLayers )
  {

  }
  virtual lava::GLFWVulkanWindowRenderer* createRenderer( void ) override
  {
    return new MainWindowRenderer( this );
  }
};


int main( int argc, char** argv )
{
  VulkanWindow app( 500, 500, "GLFWRenderer", true );
  app.show( );
  return EXIT_SUCCESS;
}