#include <lava/lava.h>
using namespace lava;

#include <routes.h>

#include "utils/Camera.h"

// settings
const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;

// camera
Camera camera( glm::vec3( 0.0f, 2.0f, 3.0f ) );
float lastX = ( float ) SCR_WIDTH / 2.0;
float lastY = ( float ) SCR_HEIGHT / 2.0;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

class MyApp : public VulkanApp
{
public:
  struct ShadowPass
  {
    struct
    {
      glm::mat4 matrix;
    } uboLightSpaceMatrix;

    struct PCLightModel
    {
      glm::mat4 model;
    } pcLightModel;

    std::shared_ptr<PipelineLayout> pipelineLayout;
    std::shared_ptr<Pipeline> pipeline;
    std::shared_ptr<DescriptorSet> descriptorSet;

    std::shared_ptr<lava::extras::CustomFBO> fbo;
    std::shared_ptr<UniformBuffer> lightSpaceMatrixBuffer;

  } shadowPass;

  struct BasicPass
  {
    struct
    {
      glm::mat4 view;
      glm::mat4 projection;
      glm::mat4 lightSpaceMatrix;
    } uboVS;

    struct PCVS
    {
      glm::mat4 model;
    } pcVS;

    struct
    {
      glm::vec3 lightPos;
      glm::vec3 viewPos;
    } uboFS;

    std::shared_ptr<PipelineLayout> pipelineLayout;
    std::shared_ptr<Pipeline> pipeline;
    std::shared_ptr<DescriptorSet> descriptorSet;

    std::shared_ptr<UniformBuffer> vsBuffer;
    std::shared_ptr<UniformBuffer> fsBuffer;

    std::shared_ptr<lava::Texture2D> tex;
  } basicPass;

  std::shared_ptr<CommandPool> commandPool;
  std::shared_ptr<lava::extras::Geometry> geometryCube;
  std::shared_ptr<lava::extras::Geometry> geometryPlane;

  void prepareOffscreenRenderPass( uint32_t width, uint32_t height )  // 2048, 2048
  {
    shadowPass.fbo = std::make_shared<lava::extras::CustomFBO>( _device, width, height );
    shadowPass.fbo->addColorDepthAttachment( vk::Format::eD16Unorm );
    shadowPass.fbo->build( );

    // Create sampler to sample from the color attachments
    vk::SamplerCreateInfo sampler;
    sampler.magFilter = vk::Filter::eLinear;
    sampler.minFilter = vk::Filter::eLinear;
    sampler.mipmapMode = vk::SamplerMipmapMode::eLinear;
    sampler.addressModeU = vk::SamplerAddressMode::eClampToEdge;
    sampler.addressModeV = sampler.addressModeU;
    sampler.addressModeW = sampler.addressModeU;
    sampler.mipLodBias = 0.0f;
    sampler.maxAnisotropy = 1.0f;
    sampler.minLod = 0.0f;
    sampler.maxLod = 1.0f;
    sampler.borderColor = vk::BorderColor::eFloatOpaqueWhite;

    vk::Device device = static_cast< vk::Device >( *_device );
    device.destroySampler( shadowPass.fbo->colorSampler );

    shadowPass.fbo->colorSampler = device.createSampler( sampler );
  }
  
  std::shared_ptr<PipelineCache> pipelineCache;

  void createShadowPipelineDescriptor( void )
  {
    // ShadowMapping buffer
    shadowPass.lightSpaceMatrixBuffer = std::make_shared<UniformBuffer>( _device,
      sizeof( shadowPass.uboLightSpaceMatrix ) );

    std::vector<DescriptorSetLayoutBinding> dslbs =
    {
      DescriptorSetLayoutBinding(
        0, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eVertex
      )
    };
    std::shared_ptr<DescriptorSetLayout> descriptorSetLayout =
      _device->createDescriptorSetLayout( dslbs );

    vk::PushConstantRange pushConstantRange( vk::ShaderStageFlagBits::eVertex,
      0, sizeof( shadowPass.pcLightModel ) );

    shadowPass.pipelineLayout = _device->createPipelineLayout(
      descriptorSetLayout, pushConstantRange
    );

    PipelineShaderStageCreateInfo vertexStage = _device->createShaderPipelineShaderStage(
      LAVA_EXAMPLES_SPV_ROUTE + std::string( "shadowMappingDepth_vert.spv" ),
      vk::ShaderStageFlagBits::eVertex
    );
    PipelineShaderStageCreateInfo fragmentStage = _device->createShaderPipelineShaderStage(
      LAVA_EXAMPLES_SPV_ROUTE + std::string( "shadowMappingDepth_frag.spv" ),
      vk::ShaderStageFlagBits::eFragment
    );

    PipelineVertexInputStateCreateInfo vertexInput(
      vk::VertexInputBindingDescription( 0, sizeof( lava::extras::Vertex ),
        vk::VertexInputRate::eVertex ),
        {
          vk::VertexInputAttributeDescription(
            0, 0, vk::Format::eR32G32B32Sfloat,
            offsetof( lava::extras::Vertex, position )
          )
        }
    );
    vk::PipelineInputAssemblyStateCreateInfo assembly( {},
      vk::PrimitiveTopology::eTriangleList, VK_FALSE
    );
    PipelineViewportStateCreateInfo viewport( 1, 1 );
    vk::PipelineRasterizationStateCreateInfo rasterization( {}, true,
      false, vk::PolygonMode::eFill, vk::CullModeFlagBits::eBack,
      vk::FrontFace::eCounterClockwise, false, 0.0f, 0.0f, 0.0f, 1.0f );
    PipelineMultisampleStateCreateInfo multisample(
      vk::SampleCountFlagBits::e1, false, 0.0f, nullptr, false, false
    );
    vk::StencilOpState stencilOpState( vk::StencilOp::eKeep,
      vk::StencilOp::eKeep, vk::StencilOp::eKeep, vk::CompareOp::eAlways,
      0, 0, 0
    );
    vk::PipelineDepthStencilStateCreateInfo depthStencil( {}, true, true,
      vk::CompareOp::eLessOrEqual, false, false, stencilOpState,
      stencilOpState, 0.0f, 0.0f
    );
    vk::PipelineColorBlendAttachmentState colorBlendAttachment(
      false, vk::BlendFactor::eZero, vk::BlendFactor::eZero,
      vk::BlendOp::eAdd, vk::BlendFactor::eZero,
      vk::BlendFactor::eZero, vk::BlendOp::eAdd,
      vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
      vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA
    );
    PipelineColorBlendStateCreateInfo colorBlend( false, vk::LogicOp::eNoOp,
      colorBlendAttachment, { 1.0f, 1.0f, 1.0f, 1.0f }
    );
    PipelineDynamicStateCreateInfo dynamic( {
      vk::DynamicState::eViewport, vk::DynamicState::eScissor
    } );

    rasterization.polygonMode = vk::PolygonMode::eLine;

    shadowPass.pipeline = _device->createGraphicsPipeline( pipelineCache, {},
    { vertexStage, fragmentStage }, vertexInput, assembly, nullptr,
      viewport, rasterization, multisample, depthStencil, colorBlend, dynamic,
      shadowPass.pipelineLayout, _renderPass );

    std::array<vk::DescriptorPoolSize, 1> poolSize;
    poolSize[ 0 ] = vk::DescriptorPoolSize( vk::DescriptorType::eUniformBuffer, 1 );
    std::shared_ptr<DescriptorPool> descriptorPool =
      _device->createDescriptorPool( {}, 1, poolSize );

    // Init descriptor set
    shadowPass.descriptorSet = _device->allocateDescriptorSet(
      descriptorPool, descriptorSetLayout
    );
    std::vector<WriteDescriptorSet> wdss =
    {
      WriteDescriptorSet( shadowPass.descriptorSet, 0, 0,
        vk::DescriptorType::eUniformBuffer, 1, nullptr,
        DescriptorBufferInfo( shadowPass.lightSpaceMatrixBuffer, 0,
          sizeof( shadowPass.uboLightSpaceMatrix )
        )
      )
    };

    _device->updateDescriptorSets( wdss, {} );
  }

  void createPipelineDescriptor( void )
  {
    basicPass.vsBuffer = std::make_shared<UniformBuffer>( _device,
      sizeof( basicPass.uboVS ) );

    basicPass.fsBuffer = std::make_shared<UniformBuffer>( _device,
      sizeof( basicPass.uboFS ) );

    std::vector<DescriptorSetLayoutBinding> dslbs =
    {
      DescriptorSetLayoutBinding(
        0, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eVertex
      ),
      DescriptorSetLayoutBinding(
        1, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eFragment
      ),
      DescriptorSetLayoutBinding(
        2, vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment
      ),
      DescriptorSetLayoutBinding(
        3, vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment
      )
    };
    std::shared_ptr<DescriptorSetLayout> descriptorSetLayout =
      _device->createDescriptorSetLayout( dslbs );

    vk::PushConstantRange pushConstantRange( vk::ShaderStageFlagBits::eVertex,
      0, sizeof( basicPass.pcVS ) );

    basicPass.pipelineLayout = _device->createPipelineLayout(
      descriptorSetLayout, pushConstantRange
    );

    PipelineShaderStageCreateInfo vertexStage = _device->createShaderPipelineShaderStage(
      LAVA_EXAMPLES_SPV_ROUTE + std::string( "shadowMapping_vert.spv" ),
      vk::ShaderStageFlagBits::eVertex
    );
    PipelineShaderStageCreateInfo fragmentStage = _device->createShaderPipelineShaderStage(
      LAVA_EXAMPLES_SPV_ROUTE + std::string( "shadowMapping_frag.spv" ),
      vk::ShaderStageFlagBits::eFragment
    );

    PipelineVertexInputStateCreateInfo vertexInput(
      vk::VertexInputBindingDescription( 0, sizeof( lava::extras::Vertex ),
        vk::VertexInputRate::eVertex ),
        {
          vk::VertexInputAttributeDescription(
            0, 0, vk::Format::eR32G32B32Sfloat,
            offsetof( lava::extras::Vertex, position )
          ),
          vk::VertexInputAttributeDescription(
            1, 0, vk::Format::eR32G32B32Sfloat,
            offsetof( lava::extras::Vertex, normal )
          ),
          vk::VertexInputAttributeDescription(
            2, 0, vk::Format::eR32G32Sfloat,
            offsetof( lava::extras::Vertex, texCoord )
          )
        }
    );
    vk::PipelineInputAssemblyStateCreateInfo assembly( {},
      vk::PrimitiveTopology::eTriangleList, VK_FALSE
    );
    PipelineViewportStateCreateInfo viewport( 1, 1 );
    vk::PipelineRasterizationStateCreateInfo rasterization( {}, true,
      false, vk::PolygonMode::eFill, vk::CullModeFlagBits::eBack,
      vk::FrontFace::eCounterClockwise, false, 0.0f, 0.0f, 0.0f, 1.0f );
    PipelineMultisampleStateCreateInfo multisample(
      vk::SampleCountFlagBits::e1, false, 0.0f, nullptr, false, false
    );
    vk::StencilOpState stencilOpState( vk::StencilOp::eKeep,
      vk::StencilOp::eKeep, vk::StencilOp::eKeep, vk::CompareOp::eAlways,
      0, 0, 0
    );
    vk::PipelineDepthStencilStateCreateInfo depthStencil( {}, true, true,
      vk::CompareOp::eLessOrEqual, false, false, stencilOpState,
      stencilOpState, 0.0f, 0.0f
    );
    vk::PipelineColorBlendAttachmentState colorBlendAttachment(
      false, vk::BlendFactor::eZero, vk::BlendFactor::eZero,
      vk::BlendOp::eAdd, vk::BlendFactor::eZero,
      vk::BlendFactor::eZero, vk::BlendOp::eAdd,
      vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
      vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA
    );
    PipelineColorBlendStateCreateInfo colorBlend( false, vk::LogicOp::eNoOp,
      colorBlendAttachment, { 1.0f, 1.0f, 1.0f, 1.0f }
    );
    PipelineDynamicStateCreateInfo dynamic( {
      vk::DynamicState::eViewport, vk::DynamicState::eScissor
    } );

    basicPass.pipeline = _device->createGraphicsPipeline( pipelineCache, {},
    { vertexStage, fragmentStage }, vertexInput, assembly, nullptr,
      viewport, rasterization, multisample, depthStencil, colorBlend, dynamic,
      basicPass.pipelineLayout, _renderPass );

    std::array<vk::DescriptorPoolSize, 2> poolSize;
    poolSize[ 0 ] = vk::DescriptorPoolSize( vk::DescriptorType::eUniformBuffer, 2 );
    poolSize[ 1 ] = vk::DescriptorPoolSize( vk::DescriptorType::eCombinedImageSampler, 2 );
    std::shared_ptr<DescriptorPool> descriptorPool =
      _device->createDescriptorPool( {}, 1, poolSize );

    // Init descriptor set
    basicPass.descriptorSet = _device->allocateDescriptorSet(
      descriptorPool, descriptorSetLayout
    );
    std::vector<WriteDescriptorSet> wdss =
    {
      WriteDescriptorSet( basicPass.descriptorSet, 0, 0,
        vk::DescriptorType::eUniformBuffer, 1, nullptr,
        DescriptorBufferInfo( basicPass.vsBuffer, 0,
          sizeof( basicPass.uboVS )
        )
      ),
      WriteDescriptorSet( basicPass.descriptorSet, 1, 0,
        vk::DescriptorType::eUniformBuffer, 1, nullptr,
        DescriptorBufferInfo( basicPass.fsBuffer, 0,
          sizeof( basicPass.uboFS )
        )
      ),
      WriteDescriptorSet( basicPass.descriptorSet, 2, 0,
        vk::DescriptorType::eCombinedImageSampler, 1,
        basicPass.tex->descriptor, nullptr
      ),
      WriteDescriptorSet( basicPass.descriptorSet, 2, 0,
        vk::DescriptorType::eCombinedImageSampler, 1,
        DescriptorImageInfo(
          vk::ImageLayout::eShaderReadOnlyOptimal,
          std::make_shared<vk::ImageView>( *shadowPass.fbo->_depthAttachment.view ),
          std::make_shared<vk::Sampler>( shadowPass.fbo->colorSampler )
        ), nullptr
      )
    };

    _device->updateDescriptorSets( wdss, {} );
  }

  MyApp(char const* title, uint32_t width, uint32_t height)
    : VulkanApp( title, width, height )
  {
    pipelineCache = _device->createPipelineCache( 0, nullptr );

    prepareOffscreenRenderPass( width * 2, height * 2 );
    commandPool = _device->createCommandPool(
      vk::CommandPoolCreateFlagBits::eResetCommandBuffer, _queueFamilyIndex );
    
    basicPass.tex = std::make_shared<Texture2D>( _device, 
      LAVA_EXAMPLES_IMAGES_ROUTE +
       std::string( "uv_checker.png" ), commandPool, _graphicsQueue,
      vk::Format::eR8G8B8A8Unorm );

    geometryCube = std::make_shared<lava::extras::Geometry>( _device,
      LAVA_EXAMPLES_MESHES_ROUTE + std::string( "cube.obj_" ) );

    geometryPlane = std::make_shared<lava::extras::Geometry>( _device,
      LAVA_EXAMPLES_MESHES_ROUTE + std::string( "plane.obj_" ) );

    createShadowPipelineDescriptor( );
    createPipelineDescriptor( );
  }
  glm::vec3 lightPos = glm::vec3( -2.0f, 4.0f, -1.0f );

  void renderShadowScene( std::shared_ptr<CommandBuffer>& commandBuffer )
  {
    // plane
    glm::mat4 model;
    shadowPass.pcLightModel.model = model;
    commandBuffer->pushConstants<ShadowPass::PCLightModel>(
      *shadowPass.pipelineLayout, vk::ShaderStageFlagBits::eVertex,
      0, shadowPass.pcLightModel
      );
    geometryPlane->render( commandBuffer );

    // cubes
    model = glm::mat4( );
    model = glm::translate( model, glm::vec3( 0.0f, 1.5f, 0.0 ) );
    model = glm::scale( model, glm::vec3( 0.5f ) );
    shadowPass.pcLightModel.model = model;
    commandBuffer->pushConstants<ShadowPass::PCLightModel>(
      *shadowPass.pipelineLayout, vk::ShaderStageFlagBits::eVertex,
      0, shadowPass.pcLightModel
      );
    geometryCube->render( commandBuffer );

    model = glm::mat4( );
    model = glm::translate( model, glm::vec3( 2.0f, 0.0f, 1.0 ) );
    model = glm::scale( model, glm::vec3( 0.5f ) );
    shadowPass.pcLightModel.model = model;
    commandBuffer->pushConstants<ShadowPass::PCLightModel>(
      *shadowPass.pipelineLayout, vk::ShaderStageFlagBits::eVertex,
      0, shadowPass.pcLightModel
      );
    geometryCube->render( commandBuffer );

    model = glm::mat4( );
    model = glm::translate( model, glm::vec3( -1.0f, 0.0f, 2.0 ) );
    model = glm::rotate( model, glm::radians( 60.0f ), glm::normalize( glm::vec3( 1.0, 0.0, 1.0 ) ) );
    model = glm::scale( model, glm::vec3( 0.25 ) );
    shadowPass.pcLightModel.model = model;
    commandBuffer->pushConstants<ShadowPass::PCLightModel>(
      *shadowPass.pipelineLayout, vk::ShaderStageFlagBits::eVertex,
      0, shadowPass.pcLightModel
      );
    geometryCube->render( commandBuffer );
  }
  void renderScene( std::shared_ptr<CommandBuffer>& commandBuffer )
  {
    // plane
    glm::mat4 model;
    basicPass.pcVS.model = model;
    commandBuffer->pushConstants<BasicPass::PCVS>(
      *basicPass.pipelineLayout, vk::ShaderStageFlagBits::eVertex,
      0, basicPass.pcVS
      );
    geometryPlane->render( commandBuffer );

    // cubes
    model = glm::mat4( );
    model = glm::translate( model, glm::vec3( 0.0f, 1.5f, 0.0 ) );
    model = glm::scale( model, glm::vec3( 0.5f ) );
    basicPass.pcVS.model = model;
    commandBuffer->pushConstants<BasicPass::PCVS>(
      *basicPass.pipelineLayout, vk::ShaderStageFlagBits::eVertex,
      0, basicPass.pcVS
      );
    geometryCube->render( commandBuffer );

    model = glm::mat4( );
    model = glm::translate( model, glm::vec3( 2.0f, 0.0f, 1.0 ) );
    model = glm::scale( model, glm::vec3( 0.5f ) );
    basicPass.pcVS.model = model;
    commandBuffer->pushConstants<BasicPass::PCVS>(
      *basicPass.pipelineLayout, vk::ShaderStageFlagBits::eVertex,
      0, basicPass.pcVS
      );
    geometryCube->render( commandBuffer );

    model = glm::mat4( );
    model = glm::translate( model, glm::vec3( -1.0f, 0.0f, 2.0 ) );
    model = glm::rotate( model, glm::radians( 60.0f ), glm::normalize( glm::vec3( 1.0, 0.0, 1.0 ) ) );
    model = glm::scale( model, glm::vec3( 0.25 ) );
    basicPass.pcVS.model = model;
    commandBuffer->pushConstants<BasicPass::PCVS>(
      *basicPass.pipelineLayout, vk::ShaderStageFlagBits::eVertex,
      0, basicPass.pcVS
      );
    geometryCube->render( commandBuffer );
  }

  virtual void doPaint( void ) override
  {
    uint32_t width = _window->getWidth( );
    uint32_t height = _window->getHeight( );

    static auto startTime = std::chrono::high_resolution_clock::now( );

    auto currentTime = std::chrono::high_resolution_clock::now( );
    float time = std::chrono::duration_cast<std::chrono::milliseconds>( currentTime - startTime ).count( ) / 1000.0f;

    float currentFrame = time;
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    glm::mat4 view = camera.GetViewMatrix( );

    glm::mat4 proj = glm::perspective( glm::radians( 45.0f ), width / ( float ) height, 0.1f, 10.0f );
    proj[ 1 ][ 1 ] *= -1;


    lightPos.x = sin(time) * 3.0f;
    lightPos.z = cos(time) * 2.0f;
    lightPos.y = 5.0 + cos(time) * 1.0f;

    glm::mat4 lightProjection, lightView;
    glm::mat4 lightSpaceMatrix;
    float near_plane = 1.0f, far_plane = 7.5f;
    
    //lightProjection = glm::perspective(glm::radians(45.0f), (width * 2.0f) / (height * 2.0f), near_plane, far_plane); // note that if you use a perspective projection matrix you'll have to change the light position as the current light position isn't enough to reflect the whole scene
    lightProjection = glm::ortho( -10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane );
    
    lightView = glm::lookAt( lightPos, glm::vec3( 0.0f ), glm::vec3( 0.0, 1.0, 0.0 ) );
    lightSpaceMatrix = lightProjection * lightView;

    shadowPass.uboLightSpaceMatrix.matrix = lightSpaceMatrix;

    shadowPass.lightSpaceMatrixBuffer->writeData( 0, 
      sizeof( shadowPass.uboLightSpaceMatrix ), &shadowPass.uboLightSpaceMatrix
    );

    std::shared_ptr<CommandBuffer> fboCommandBuffer =
      commandPool->allocateCommandBuffer( );

    fboCommandBuffer->begin( );

    std::array<float, 4> ccv = { 0.2f, 0.3f, 0.3f, 1.0f };
    fboCommandBuffer->beginRenderPass( shadowPass.fbo->renderPass,
      shadowPass.fbo->_fbo,
      vk::Rect2D( { 0, 0 },
        _defaultFramebuffer->getExtent( ) ),
        { vk::ClearValue( ccv ), vk::ClearValue(
          vk::ClearDepthStencilValue( 1.0f, 0 ) )
        }, vk::SubpassContents::eInline );

    fboCommandBuffer->bindGraphicsPipeline( shadowPass.pipeline );
    fboCommandBuffer->bindDescriptorSets( vk::PipelineBindPoint::eGraphics,
      shadowPass.pipelineLayout, 0, { shadowPass.descriptorSet }, nullptr );

    fboCommandBuffer->setViewportScissors( _defaultFramebuffer->getExtent( ) );

    renderShadowScene( fboCommandBuffer );

    fboCommandBuffer->endRenderPass( );

    fboCommandBuffer->end( );

    std::shared_ptr<CommandBuffer> commandBuffer =
      commandPool->allocateCommandBuffer( );


    commandBuffer->begin( );

    commandBuffer->beginRenderPass( _renderPass,
      _defaultFramebuffer->getFramebuffer( ),
      vk::Rect2D( { 0, 0 },
        _defaultFramebuffer->getExtent( ) ),
        { vk::ClearValue( ccv ), vk::ClearValue(
          vk::ClearDepthStencilValue( 1.0f, 0 ) )
        }, vk::SubpassContents::eInline );

    commandBuffer->setViewportScissors( width, height );
    commandBuffer->bindDescriptorSets( vk::PipelineBindPoint::eGraphics,
      basicPass.pipelineLayout, 0, { basicPass.descriptorSet }, nullptr );
    commandBuffer->bindGraphicsPipeline( basicPass.pipeline );
    commandBuffer->draw( 4, 1, 0, 0 );
    commandBuffer->endRenderPass( );

    commandBuffer->end( );


    basicPass.uboVS.view = view;
    basicPass.uboVS.projection = proj;

    basicPass.uboFS.lightPos = lightPos;
    basicPass.uboFS.viewPos = camera.Position;

    basicPass.vsBuffer->writeData( 0, sizeof( basicPass.uboVS ), &basicPass.uboVS );
    basicPass.fsBuffer->writeData( 0, sizeof( basicPass.uboFS ), &basicPass.uboFS );



    _graphicsQueue->submit( SubmitInfo{
      { _defaultFramebuffer->getPresentSemaphore( ) }, // wait 
      { vk::PipelineStageFlagBits::eColorAttachmentOutput },
      fboCommandBuffer,
      { shadowPass.fbo->semaphore } // signal
    } );

    _graphicsQueue->submit( SubmitInfo{
      { shadowPass.fbo->semaphore },
      { vk::PipelineStageFlagBits::eColorAttachmentOutput },
      commandBuffer,
      _renderComplete
    } );
  }
  void keyEvent(int key, int scancode, int action, int mods)
  {
    switch (key)
    {
    case GLFW_KEY_W:
      switch ( action )
      {
      case GLFW_PRESS:
        camera.ProcessKeyboard( FORWARD, deltaTime );
        break;
      }
      break;
    case GLFW_KEY_S:
      switch ( action )
      {
      case GLFW_PRESS:
        camera.ProcessKeyboard( BACKWARD, deltaTime );
        break;
      }
      break;
    case GLFW_KEY_A:
      switch ( action )
      {
      case GLFW_PRESS:
        camera.ProcessKeyboard( LEFT, deltaTime );
        break;
      }
      break;
    case GLFW_KEY_D:
      switch ( action )
      {
      case GLFW_PRESS:
        camera.ProcessKeyboard( RIGHT, deltaTime );
        break;
      }
      break;
    case GLFW_KEY_ESCAPE:
      switch (action)
      {
      case GLFW_PRESS:
        getWindow( )->close( );
        break;
      default:
        break;
      }
      break;
    default:
      break;
    }
  }
  virtual void cursorPosEvent( double xPos, double yPos )
  {
    if ( firstMouse )
    {
      lastX = xPos;
      lastY = yPos;
      firstMouse = false;
    }

    float xoffset = xPos - lastX;
    float yoffset = lastY - yPos; // reversed since y-coordinates go from bottom to top

    lastX = xPos;
    lastY = yPos;

    camera.ProcessMouseMovement( xoffset, yoffset );
  }
};

void glfwErrorCallback(int error, const char* description)
{
  fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

int main( void )
{
  try
  {
    VulkanApp* app = new MyApp( "Shadow Mapping", 800, 600 );

    app->getWindow( )->setErrorCallback( glfwErrorCallback );

    while ( app->isRunning( ) )
    {
      app->waitEvents( );
      app->paint( );
    }

    delete app;
  }
  catch ( std::system_error err )
  {
    std::cout << "System Error: " << err.what( ) << std::endl;
  }
  return 0;
}