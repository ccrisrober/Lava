#include <iostream>

#include <glfwPompeii/glfwPompeii.h>
#include <pompeiiUtils/pompeiiUtils.h>
using namespace pompeii;

#include <glm/glm.hpp>

#include <random>

#include <routes.h>

#include "../utils/Camera.h"

const size_t NUM_LIGHTS = 16;

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

class MainWindowRenderer : public glfw::VulkanWindowRenderer
{
private:
  glfw::VulkanWindow* _window;

  Camera camera;

  std::shared_ptr< pompeii::utility::Geometry > geometry;

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
    glm::mat4 view;
  } uboGBuffer;

  struct
  {
    std::shared_ptr< pompeii::UniformBuffer > GBuffer;
    std::shared_ptr< pompeii::UniformBuffer > lights;
  } uniformBuffers;

  struct
  {
    std::shared_ptr< Pipeline > offscreen;
    std::shared_ptr< Pipeline > composition;
  } pipelines;

  struct
  {
    std::shared_ptr< PipelineLayout > offscreen;
    std::shared_ptr< PipelineLayout > composition;
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
public:
  MainWindowRenderer( glfw::VulkanWindow* window )
    : _window( window )
  {
  }
public:
  void prepareUniformBuffers( )
  {
    auto device = _window->device( );
    // Deferred vertex shader
    uniformBuffers.GBuffer = device->createUniformBuffer( sizeof( uboGBuffer ) );
    // Composite fragment shader
    uniformBuffers.lights = device->createUniformBuffer( sizeof( uboLights ) );

    // update
    updateUniformBufferDeferredMatrices( );
    updateUniformBufferDeferredLights( );
  }

  void updateUniformBufferDeferredMatrices( void )
  {
    auto size = _window->swapchainImageSize( );

    uint32_t width = size.width, height = size.height;

    uboGBuffer.view = camera.GetViewMatrix( );
    uboGBuffer.projection = glm::perspective( glm::radians( camera.Zoom ),
      ( float ) width / ( float ) height, 0.1f, 100.0f );
    uboGBuffer.projection[ 1 ][ 1 ] *= -1;

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

  void prepareCompositionPass( void )
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

    descriptorSetLayouts.composition = device->createDescriptorSetLayout( dslb );

    pipelineLayouts.composition = device->createPipelineLayout( 
      descriptorSetLayouts.composition );

    descriptorSets.composition = device->allocateDescriptorSet( descriptorPool, 
      descriptorSetLayouts.composition );

    // Image descriptor for the offscreen color attachments
    DescriptorImageInfo texDescriptorPosition(
      vk::ImageLayout::eShaderReadOnlyOptimal,
      attachments.position.view, nullptr );
    DescriptorImageInfo texDescriptorNormal(
      vk::ImageLayout::eShaderReadOnlyOptimal,
      attachments.normal.view, nullptr );
    DescriptorImageInfo texDescriptorAlbedo(
      vk::ImageLayout::eShaderReadOnlyOptimal,
      attachments.albedo.view, nullptr );

    std::vector<WriteDescriptorSet> wdss =
    {
      // Binding 0: Position texture target
      WriteDescriptorSet(
        descriptorSets.composition, 0, 0,
        vk::DescriptorType::eInputAttachment, 1, texDescriptorPosition, nullptr
      ),
      // Binding 1: Normals texture target
      WriteDescriptorSet(
        descriptorSets.composition, 1, 0,
        vk::DescriptorType::eInputAttachment, 1, texDescriptorNormal, nullptr
      ),
			// Binding 2: Albedo texture target
      WriteDescriptorSet(
        descriptorSets.composition, 2, 0,
        vk::DescriptorType::eInputAttachment, 1, texDescriptorAlbedo, nullptr
      ),
			// Binding 4: Fragment shader lights
      WriteDescriptorSet(
        descriptorSets.composition, 3, 0, vk::DescriptorType::eUniformBuffer,
        1, nullptr, DescriptorBufferInfo( uniformBuffers.lights, 0, sizeof( uboLights ) )
      )
    };
    device->updateDescriptorSets( wdss, { } );

    PipelineVertexInputStateCreateInfo vertexInput( { }, { } );

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
    depthStencil.depthWriteEnable = false;
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

    // Specialization constants for number of lights
    std::vector<vk::SpecializationMapEntry> specMapEntries =
    {
      vk::SpecializationMapEntry( 0, 0, sizeof( uint32_t ) )
    };

    uint32_t specData = NUM_LIGHTS;

    SpecializationInfo specInfo( specMapEntries, &specData );

    auto vertexStage = device->createShaderPipelineShaderStage(
      POMPEII_EXAMPLES_SPV_ROUTE + std::string( "composition_vert.spv" ),
      vk::ShaderStageFlagBits::eVertex
    );
    auto fragmentStage = device->createShaderPipelineShaderStage(
      POMPEII_EXAMPLES_SPV_ROUTE + std::string( "composition_frag.spv" ),
      vk::ShaderStageFlagBits::eFragment, specInfo
    );

    pipelines.composition = device->createGraphicsPipeline( 
      _window->pipelineCache( ), { },
      { vertexStage, fragmentStage }, vertexInput, assembly, nullptr,
      viewport, rasterization, multisample, depthStencil, colorBlend, dynamic,
      pipelineLayouts.composition, _window->renderPass( ),
      // Index of the subpass that this pipeline will be used in
      1
    );
  }

  void setupDescriptorPool( void )
  {
    std::vector<vk::DescriptorPoolSize> poolSizes = 
    {
      vk::DescriptorPoolSize( vk::DescriptorType::eUniformBuffer, 2 ),
      vk::DescriptorPoolSize( vk::DescriptorType::eCombinedImageSampler, 1 ),
      vk::DescriptorPoolSize( vk::DescriptorType::eInputAttachment, 3),
    };

    descriptorPool = _window->device( )->createDescriptorPool( 2, poolSizes );
  }

  void preparePipelines( void )
  {
    vk::VertexInputBindingDescription binding( 0, sizeof( pompeii::utility::Vertex ),
      vk::VertexInputRate::eVertex );

    PipelineVertexInputStateCreateInfo vertexInput( binding, {
      vk::VertexInputAttributeDescription(
        0, 0, vk::Format::eR32G32B32Sfloat,
        offsetof( pompeii::utility::Vertex, position )
      ),
      vk::VertexInputAttributeDescription(
        1, 0, vk::Format::eR32G32B32Sfloat,
        offsetof( pompeii::utility::Vertex, normal )
      ),
      vk::VertexInputAttributeDescription(
        2, 0, vk::Format::eR32G32Sfloat,
        offsetof( pompeii::utility::Vertex, texCoord )
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
    vk::PipelineColorBlendAttachmentState blendAttachmentState( false,
      vk::BlendFactor::eZero, vk::BlendFactor::eZero, vk::BlendOp::eAdd,
      vk::BlendFactor::eZero, vk::BlendFactor::eZero, vk::BlendOp::eAdd,
      vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
      vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA
    );
    PipelineColorBlendStateCreateInfo colorBlend( false, vk::LogicOp::eNoOp,
      { blendAttachmentState, blendAttachmentState, 
      blendAttachmentState, blendAttachmentState }, { 1.0f, 1.0f, 1.0f, 1.0f }
    );
    PipelineDynamicStateCreateInfo dynamic( {
      vk::DynamicState::eViewport, vk::DynamicState::eScissor
    } );

    auto vertexStage = _window->device( )->createShaderPipelineShaderStage(
      POMPEII_EXAMPLES_SPV_ROUTE + std::string( "gbuffer_vert.spv" ),
      vk::ShaderStageFlagBits::eVertex
    );
    auto fragmentStage = _window->device( )->createShaderPipelineShaderStage(
      POMPEII_EXAMPLES_SPV_ROUTE + std::string( "gbuffer_frag.spv" ),
      vk::ShaderStageFlagBits::eFragment
    );
    pipelines.offscreen = _window->device( )->createGraphicsPipeline( 
      _window->pipelineCache( ), { },
      { vertexStage, fragmentStage }, vertexInput, assembly, nullptr,
      viewport, rasterization, multisample, depthStencil, colorBlend, dynamic,
      pipelineLayouts.offscreen, _window->renderPass( ),
      // Index of the subpass that this pipeline will be used in
      0
    );
  }

  void setupDescriptorSet( void )
  {
    auto device = _window->device( );
    descriptorSets.scene = device->allocateDescriptorSet( descriptorPool, 
      descriptorSetLayouts.scene );

    std::vector<WriteDescriptorSet> wdss =
    {
      // Binding 0: Vertex shader uniform buffer
      WriteDescriptorSet(
        descriptorSets.scene, 0, 0, vk::DescriptorType::eUniformBuffer,
        1, nullptr, DescriptorBufferInfo( uniformBuffers.GBuffer, 0, 
          sizeof( uboGBuffer ) )
      ),
      // Binding 1: Fragment shader mesh texture
      WriteDescriptorSet(
        descriptorSets.scene, 1, 0, vk::DescriptorType::eCombinedImageSampler, 1,
        tex->descriptor, nullptr
      ),
    };

    device->updateDescriptorSets( wdss, { } );
  }

  void setupDescriptorSetLayout( void )
  {
    auto device = _window->device( );
    // Deferred shading layout
    descriptorSetLayouts.scene = device->createDescriptorSetLayout( {
      // Binding 0: Vertex shader uniform buffer
      DescriptorSetLayoutBinding( 0, vk::DescriptorType::eUniformBuffer, 
        vk::ShaderStageFlagBits::eVertex
      ),
      // Binding 1: Fragment shader mesh texture
      DescriptorSetLayoutBinding( 1, vk::DescriptorType::eCombinedImageSampler, 
        vk::ShaderStageFlagBits::eFragment
      ),
    } );

    vk::PushConstantRange pushConstantRange(
      vk::ShaderStageFlagBits::eVertex,
      0, sizeof( glm::mat4 )
    );

    // Offscreen (scene) rendering pipeline layout
    pipelineLayouts.offscreen = device->createPipelineLayout( 
      descriptorSetLayouts.scene, pushConstantRange );
  }

  virtual void initResources( void ) override
  {
    geometry = std::make_shared<pompeii::utility::Geometry>( _window->device( ),
      POMPEII_EXAMPLES_MESHES_ROUTE + std::string( "wolf.obj_" ) );

    tex = _window->device( )->createTexture2D( POMPEII_EXAMPLES_IMAGES_ROUTE +
      std::string( "earth/earth_diffuse.jpg" ), _window->gfxCommandPool( ),
      _window->gfxQueue( ), vk::Format::eR8G8B8A8Unorm );

    initLights( );
    prepareUniformBuffers( );
    setupDescriptorSetLayout( );
    preparePipelines( );

    setupDescriptorPool( );
    setupDescriptorSet( );
    prepareCompositionPass( );
  }

  virtual void nextFrame( void )
  {
    auto cmd = _window->currentCommandBuffer( );

    std::array<vk::ClearValue, 5 > clearValues;
    std::array<float, 4> ccv = { 0.0f, 0.0f, 0.0, 1.0f };

    for ( int i = 0; i < 4; ++i )
    {
      clearValues[ i ].color = vk::ClearColorValue( ccv );
    }
    clearValues[ 4 ].depthStencil = vk::ClearDepthStencilValue( 1.0f, 0 );

    vk::Extent2D extent = _window->swapchainImageSize( );

    cmd->beginRenderPass( _window->renderPass( ),
      _window->framebuffer( ),
      vk::Rect2D( { 0, 0 }, extent ), clearValues,
      vk::SubpassContents::eInline );
    cmd->setViewportScissors( extent );
    
    // First subpass
    // Render the components of the scene to the G-Buffer attachments
    //    DRAW
      cmd->bindGraphicsPipeline( pipelines.offscreen );
      cmd->bindDescriptorSets( vk::PipelineBindPoint::eGraphics, 
        pipelineLayouts.offscreen, 0, descriptorSets.scene, nullptr );
      cmd->pushConstants<glm::mat4>( *pipelineLayouts.offscreen, 
        vk::ShaderStageFlagBits::eVertex, 0, glm::mat4( 1.0f ) );
      geometry->render( cmd, 1 );

    // Second subpass
			// This subpass will use the G-Buffer components that have been filled in the first subpass as input attachment for the final compositing
    cmd->nextSubpass( vk::SubpassContents::eInline );
    //    DRAW
      cmd->bindGraphicsPipeline( pipelines.composition );
      cmd->bindDescriptorSets( vk::PipelineBindPoint::eGraphics,
        pipelineLayouts.composition, 0, descriptorSets.composition, nullptr );
      cmd->draw( 4, 1, 0, 0 );  // Empty plane
    
    cmd->endRenderPass( );

    _window->frameReady( );
  }
protected:
  std::shared_ptr< pompeii::DescriptorPool > descriptorPool;
  std::shared_ptr< pompeii::Texture > tex;
};

class VulkanWindow : public glfw::VulkanWindow
{
public:
  explicit VulkanWindow( int width, int height,
    const std::string& title, bool enableLayers )
    : glfw::VulkanWindow( width, height, title, enableLayers )
  {
  }

  virtual void setupFramebuffer( void ) override
  {
    std::vector<std::shared_ptr<pompeii::ImageView>> attachmentsIV( 5 );

    _framebuffers.resize( _swapchain->count( ) );
    // Create frame buffers for every swap chain image
    for ( uint32_t i = 0; i < _framebuffers.size( ); ++i )
    {
      attachmentsIV[ 0 ] = _swapchain->imageViews( )[ i ];
      attachmentsIV[ 1 ] = attachments.position.view;
      attachmentsIV[ 2 ] = attachments.normal.view;
      attachmentsIV[ 3 ] = attachments.albedo.view;
      attachmentsIV[ 4 ] = _depthView;

      _framebuffers[ i ] = device( )->createFramebuffer( renderPass( ), 
        attachmentsIV, swapchainImageSize( ), 1 );
    }
  }

  virtual void setupRenderPass( void ) override
  {
    createGBufferAttachments( );
    std::array<vk::AttachmentDescription, 5> attachmentsDescription = {
      vk::AttachmentDescription( vk::AttachmentDescriptionFlags( ),
      colorFormat( ), vk::SampleCountFlagBits::e1,
      vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eStore,
      vk::AttachmentLoadOp::eDontCare, vk::AttachmentStoreOp::eDontCare,
      vk::ImageLayout::eUndefined, vk::ImageLayout::ePresentSrcKHR
      ),
      vk::AttachmentDescription( vk::AttachmentDescriptionFlags( ),
        attachments.position.format, vk::SampleCountFlagBits::e1,
        vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eStore,
        vk::AttachmentLoadOp::eDontCare, vk::AttachmentStoreOp::eDontCare,
        vk::ImageLayout::eUndefined, vk::ImageLayout::eColorAttachmentOptimal
      ),
      vk::AttachmentDescription( vk::AttachmentDescriptionFlags( ),
        attachments.normal.format, vk::SampleCountFlagBits::e1,
        vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eStore,
        vk::AttachmentLoadOp::eDontCare, vk::AttachmentStoreOp::eDontCare,
        vk::ImageLayout::eUndefined, vk::ImageLayout::eColorAttachmentOptimal
      ),
      vk::AttachmentDescription( vk::AttachmentDescriptionFlags( ),
        attachments.albedo.format, vk::SampleCountFlagBits::e1,
        vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eStore,
        vk::AttachmentLoadOp::eDontCare, vk::AttachmentStoreOp::eDontCare,
        vk::ImageLayout::eUndefined, vk::ImageLayout::eColorAttachmentOptimal
      ),
      vk::AttachmentDescription( vk::AttachmentDescriptionFlags( ),
        depthStencilFormat( ), vk::SampleCountFlagBits::e1,
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

    _renderPass = device( )->createRenderPass( 
      attachmentsDescription, subpassDescription, dependencies );
  }
 
  // Create color attachments for the G-Buffer components
  void createGBufferAttachments( void )
  {
    auto size = swapchainImageSize( );
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
    //vk::ImageLayout imageLayout;

    attachment->format = format;

    if ( usage & vk::ImageUsageFlagBits::eColorAttachment )
    {
      aspectMask = vk::ImageAspectFlagBits::eColor;
      //imageLayout = vk::ImageLayout::eColorAttachmentOptimal;
    }
    if ( usage & vk::ImageUsageFlagBits::eDepthStencilAttachment )
    {
      aspectMask = vk::ImageAspectFlagBits::eDepth |
        vk::ImageAspectFlagBits::eStencil;
      //imageLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;
    }

    //assert( aspectMask > 0 );

    attachment->image = device( )->createImage( vk::ImageCreateFlagBits( ),
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

  virtual glfw::VulkanWindowRenderer* createRenderer( void ) override
  {
    return new MainWindowRenderer( this );
  }
};


int main( int, char** )
{
  VulkanWindow app( 500, 500, "GLFWRenderer", true );
  app.show( );
  return EXIT_SUCCESS;
}