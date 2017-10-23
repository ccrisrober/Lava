#include <lava/lava.h>
using namespace lava;

#include <routes.h>

#include "utils/Camera.h"

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 5.0f));

// timing
float deltaTime = 0.0f; // time between current frame and last frame
float lastFrame = 0.0f;

float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

const int NR_LIGHTS = 6;

class MyApp : public VulkanApp
{
public:
  struct
  {
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
    glm::vec3 instancePos[ 9 ];
  } uboMRT;

  struct Light
  {
    glm::vec3 position;
    glm::vec3 color;
  };
  struct
  {
    Light lights[ NR_LIGHTS ];
    glm::vec3 viewPos;
  } uboPP;

  struct
  {
    std::shared_ptr<Buffer> mrt;        // Vertex shader
    std::shared_ptr<Buffer> postprocess;  // Fragment shader
  } uniforms;

  struct
  {
    std::shared_ptr<Pipeline> mrt;
    std::shared_ptr<Pipeline> postprocess;
  } pipelines;

  struct
  {
    std::shared_ptr<PipelineLayout> mrt;
    std::shared_ptr<PipelineLayout> postprocess;
  } pipelineLayouts;

  struct
  {
    std::shared_ptr<DescriptorSet> mrt;
    std::shared_ptr<DescriptorSet> postprocess;
  } descriptorSets;

  struct
  {
    std::shared_ptr<DescriptorSetLayout> mrt;
    std::shared_ptr<DescriptorSetLayout> postprocess;
  } descriptorSetLayouts;

  std::shared_ptr<lava::extras::CustomFBO> fbo;

  std::shared_ptr<lava::extras::Geometry> geometry;

  std::shared_ptr<Texture2D> texDiffuse, texSpecular;

  MyApp( char const* title, uint32_t width, uint32_t height )
    : VulkanApp( title, width, height )
  {
    std::shared_ptr<CommandPool> commandPool = _device->createCommandPool(
      vk::CommandPoolCreateFlagBits::eResetCommandBuffer, _queueFamilyIndex );

    texDiffuse = std::make_shared<Texture2D>( _device, LAVA_EXAMPLES_IMAGES_ROUTE +
      std::string( "earth_diffuse.jpg" ), commandPool, _graphicsQueue,
      vk::Format::eR8G8B8A8Unorm );

    texSpecular = std::make_shared<Texture2D>( _device, LAVA_EXAMPLES_IMAGES_ROUTE +
      std::string( "earth_specular.jpg" ), commandPool, _graphicsQueue,
      vk::Format::eR8G8B8A8Unorm );

    uboMRT.instancePos[ 0 ] = glm::vec3( -3.0f, -3.0f, -3.0f );
    uboMRT.instancePos[ 1 ] = glm::vec3( 0.0f, -3.0f, -3.0f );
    uboMRT.instancePos[ 2 ] = glm::vec3( 3.0f, -3.0f, -3.0f );
    uboMRT.instancePos[ 3 ] = glm::vec3( -3.0f, -3.0f, 0.0f );
    uboMRT.instancePos[ 4 ] = glm::vec3( 0.0f, -3.0f, 0.0f );
    uboMRT.instancePos[ 5 ] = glm::vec3( 3.0f, -3.0f, 0.0f );
    uboMRT.instancePos[ 6 ] = glm::vec3( -3.0f, -3.0f, 3.0f );
    uboMRT.instancePos[ 7 ] = glm::vec3( 0.0f, -3.0f, 3.0f );
    uboMRT.instancePos[ 8 ] = glm::vec3( 3.0f, -3.0f, 3.0f );

    geometry = std::make_shared<lava::extras::Geometry>( _device,
      LAVA_EXAMPLES_MESHES_ROUTE + std::string( "sphere.obj_" ) );

    fbo = std::make_shared< lava::extras::CustomFBO>( _device, width, height );

    fbo->addColorAttachmentt( vk::Format::eR16G16B16A16Sfloat );     // Position
    fbo->addColorAttachmentt( vk::Format::eR16G16B16A16Sfloat );     // Normal
    fbo->addColorAttachmentt( vk::Format::eR8G8B8A8Unorm );         // Albedo (color + spec)
    fbo->addDepthAttachment( this->_depthFormat );

    fbo->build( );

    // Uniform buffers
    {
      {
        uint32_t bufferSize = sizeof( uboMRT );
        uniforms.mrt = _device->createBuffer( bufferSize,
          vk::BufferUsageFlagBits::eUniformBuffer,
          vk::SharingMode::eExclusive, nullptr,
          vk::MemoryPropertyFlagBits::eHostVisible |
          vk::MemoryPropertyFlagBits::eHostCoherent );
      }
      {
        uint32_t bufferSize = sizeof( uboPP );
        uniforms.postprocess = _device->createBuffer( bufferSize,
          vk::BufferUsageFlagBits::eUniformBuffer,
          vk::SharingMode::eExclusive, nullptr,
          vk::MemoryPropertyFlagBits::eHostVisible |
          vk::MemoryPropertyFlagBits::eHostCoherent );
      }
    }

    std::vector<DescriptorSetLayoutBinding> dslbs =
    {
      // Binding 0: Vertex shader uniform buffer
      DescriptorSetLayoutBinding(
        0,
        vk::DescriptorType::eUniformBuffer,
        vk::ShaderStageFlagBits::eVertex
      ),
      // Binding 1: Color texture target
      DescriptorSetLayoutBinding(
        1,
        vk::DescriptorType::eCombinedImageSampler,
        vk::ShaderStageFlagBits::eFragment
      ),
      // Binding 2: Specular texture target
      DescriptorSetLayoutBinding(
        2,
        vk::DescriptorType::eCombinedImageSampler,
        vk::ShaderStageFlagBits::eFragment
      )
    };

    descriptorSetLayouts.mrt = _device->createDescriptorSetLayout( dslbs );
    pipelineLayouts.mrt = _device->createPipelineLayout( descriptorSetLayouts.mrt );

    dslbs =
    {
      // Binding 0: Fragment shader uniform buffer (lights + viewPos)
      DescriptorSetLayoutBinding(
        0,
        vk::DescriptorType::eUniformBuffer,
        vk::ShaderStageFlagBits::eFragment
      ),
      // Binding 1: Position texture target
      DescriptorSetLayoutBinding(
        1,
        vk::DescriptorType::eCombinedImageSampler,
        vk::ShaderStageFlagBits::eFragment
      ),
      // Binding 2: Normal texture target
      DescriptorSetLayoutBinding(
        2,
        vk::DescriptorType::eCombinedImageSampler,
        vk::ShaderStageFlagBits::eFragment
      ),
      // Binding 3: Albedo texture target
      DescriptorSetLayoutBinding(
        3,
        vk::DescriptorType::eCombinedImageSampler,
        vk::ShaderStageFlagBits::eFragment
      )
    };

    descriptorSetLayouts.postprocess = _device->createDescriptorSetLayout( dslbs );
    pipelineLayouts.postprocess = _device->createPipelineLayout( descriptorSetLayouts.postprocess );

    // Init pipelines
    std::shared_ptr< PipelineCache> pipelineCache = _device->createPipelineCache( 0 );

    // Create pipelines
    {
      PipelineShaderStageCreateInfo vertexStage = _device->createShaderPipelineShaderStage(
        LAVA_EXAMPLES_SPV_ROUTE + std::string( "mrt_vert.spv" ), 
        vk::ShaderStageFlagBits::eVertex
      );
      PipelineShaderStageCreateInfo fragmentStage = _device->createShaderPipelineShaderStage(
        LAVA_EXAMPLES_SPV_ROUTE + std::string( "mrt_frag.spv" ), 
        vk::ShaderStageFlagBits::eFragment
      );

      PipelineVertexInputStateCreateInfo vertexInput(
        vk::VertexInputBindingDescription( 0, sizeof( lava::extras::Vertex ),
          vk::VertexInputRate::eVertex ),
          {
            vk::VertexInputAttributeDescription( 0, 0,
            vk::Format::eR32G32B32Sfloat, offsetof( lava::extras::Vertex, position ) ),
            vk::VertexInputAttributeDescription( 1, 0,
            vk::Format::eR32G32B32Sfloat, offsetof( lava::extras::Vertex, normal ) ),
            vk::VertexInputAttributeDescription( 2, 0,
            vk::Format::eR32G32Sfloat, offsetof( lava::extras::Vertex, texCoord ) )
          }
      );

      vk::PipelineInputAssemblyStateCreateInfo assembly( {},
        vk::PrimitiveTopology::eTriangleList, VK_FALSE );
      PipelineViewportStateCreateInfo viewport( { {} }, { {} } );
      vk::PipelineRasterizationStateCreateInfo rasterization( {}, true,
        false, vk::PolygonMode::eFill, vk::CullModeFlagBits::eNone,
        vk::FrontFace::eCounterClockwise, false, 0.0f, 0.0f, 0.0f, 1.0f );
      PipelineMultisampleStateCreateInfo multisample( vk::SampleCountFlagBits::e1,
        false, 0.0f, nullptr, false, false );
      vk::StencilOpState stencilOpState( vk::StencilOp::eKeep,
        vk::StencilOp::eKeep, vk::StencilOp::eKeep, vk::CompareOp::eAlways, 0, 0, 0 );
      vk::PipelineDepthStencilStateCreateInfo depthStencil( {}, true, true,
        vk::CompareOp::eLessOrEqual, false, false, stencilOpState,
        stencilOpState, 0.0f, 0.0f );

      std::array<vk::PipelineColorBlendAttachmentState, 3> blendAttachments =
      {
        vk::PipelineColorBlendAttachmentState( false,
          vk::BlendFactor::eZero, vk::BlendFactor::eZero, vk::BlendOp::eAdd,
          vk::BlendFactor::eZero, vk::BlendFactor::eZero, vk::BlendOp::eAdd,
          vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
          vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA
        ),
        vk::PipelineColorBlendAttachmentState( false,
          vk::BlendFactor::eZero, vk::BlendFactor::eZero, vk::BlendOp::eAdd,
          vk::BlendFactor::eZero, vk::BlendFactor::eZero, vk::BlendOp::eAdd,
          vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
          vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA
        ),
        vk::PipelineColorBlendAttachmentState( false,
          vk::BlendFactor::eZero, vk::BlendFactor::eZero, vk::BlendOp::eAdd,
          vk::BlendFactor::eZero, vk::BlendFactor::eZero, vk::BlendOp::eAdd,
          vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
          vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA
        ),
      };

      PipelineColorBlendStateCreateInfo colorBlend( false, vk::LogicOp::eNoOp,
        blendAttachments, { 1.0f, 1.0f, 1.0f, 1.0f } );
      PipelineDynamicStateCreateInfo dynamic( { vk::DynamicState::eViewport,
        vk::DynamicState::eScissor } );


      pipelines.mrt = _device->createGraphicsPipeline( pipelineCache, { },
      { vertexStage, fragmentStage }, vertexInput, assembly, nullptr,
        viewport, rasterization, multisample, depthStencil, colorBlend, dynamic,
        pipelineLayouts.mrt, fbo->renderPass );

      PipelineVertexInputStateCreateInfo emptyInputState( { }, { } );
      vk::PipelineInputAssemblyStateCreateInfo assemblyPP( { },
        vk::PrimitiveTopology::eTriangleStrip, VK_FALSE );

      PipelineShaderStageCreateInfo ppVertexStage = 
        _device->createShaderPipelineShaderStage(
          LAVA_EXAMPLES_SPV_ROUTE + std::string( "fullquad_vert.spv" ), 
          vk::ShaderStageFlagBits::eVertex
      );
      PipelineShaderStageCreateInfo ppFragmentStage = 
        _device->createShaderPipelineShaderStage(
          LAVA_EXAMPLES_SPV_ROUTE + std::string( "deferred_frag.spv" ), 
          vk::ShaderStageFlagBits::eFragment
      );

      colorBlend = PipelineColorBlendStateCreateInfo( false, vk::LogicOp::eNoOp,
        blendAttachments[ 0 ], { 1.0f, 1.0f, 1.0f, 1.0f } );

      pipelines.postprocess = _device->createGraphicsPipeline( pipelineCache, { },
      { ppVertexStage, ppFragmentStage }, emptyInputState, assemblyPP, nullptr,
        viewport, rasterization, multisample, depthStencil, colorBlend, dynamic,
        pipelineLayouts.postprocess, _renderPass );
    }

    std::array< vk::DescriptorPoolSize, 2 > poolSize =
    {
      vk::DescriptorPoolSize( vk::DescriptorType::eUniformBuffer, 2 ),
      vk::DescriptorPoolSize( vk::DescriptorType::eCombinedImageSampler, 5 )
    };

    std::shared_ptr<DescriptorPool> descriptorPool =
      _device->createDescriptorPool( {}, 2, poolSize );

    // Init descriptor sets
    {
      // MRT descriptor
      descriptorSets.mrt = _device->allocateDescriptorSet( descriptorPool,
        descriptorSetLayouts.mrt );
      std::vector<lava::WriteDescriptorSet> wdss =
      {
        // Binding 0: Uniform MVP buffer
        lava::WriteDescriptorSet(
          descriptorSets.mrt, 0, 0, vk::DescriptorType::eUniformBuffer,
          1, nullptr, DescriptorBufferInfo( uniforms.mrt, 0, sizeof( uboMRT ) )
        ),
        // Binding 1: Color texture target
        lava::WriteDescriptorSet(
          descriptorSets.mrt, 1, 0,
          vk::DescriptorType::eCombinedImageSampler, 1,
          texDiffuse->descriptor, nullptr
        ),
        // Binding 2: Specular texture target
        lava::WriteDescriptorSet(
          descriptorSets.mrt, 2, 0,
          vk::DescriptorType::eCombinedImageSampler, 1,
          texSpecular->descriptor, nullptr
        )
      };
      _device->updateDescriptorSets( wdss, {} );

      // MRT descriptor
      descriptorSets.postprocess = _device->allocateDescriptorSet( descriptorPool,
        descriptorSetLayouts.postprocess );
      wdss =
      {
        // Binding 0: Fragment shader uniform buffer (lights + viewPos)
        lava::WriteDescriptorSet(
          descriptorSets.postprocess, 0, 0, vk::DescriptorType::eUniformBuffer,
          1, nullptr, DescriptorBufferInfo( uniforms.mrt, 0, sizeof( uboMRT ) )
        ),
        // Binding 1: Position texture target
        lava::WriteDescriptorSet(
          descriptorSets.postprocess, 1, 0,
          vk::DescriptorType::eCombinedImageSampler, 1,
          DescriptorImageInfo(
            vk::ImageLayout::eShaderReadOnlyOptimal,
            std::make_shared<vk::ImageView>( *fbo->_colorAttachments[ 0 ].view ),
            std::make_shared<vk::Sampler>( fbo->colorSampler )
          ), nullptr
        ),
        // Binding 2: Normal texture target
        lava::WriteDescriptorSet(
          descriptorSets.postprocess, 2, 0,
          vk::DescriptorType::eCombinedImageSampler, 1,
          DescriptorImageInfo(
            vk::ImageLayout::eShaderReadOnlyOptimal,
            std::make_shared<vk::ImageView>( *fbo->_colorAttachments[ 1 ].view ),
            std::make_shared<vk::Sampler>( fbo->colorSampler )
          ), nullptr
        ),
        // Binding 3: Albedo texture target
        lava::WriteDescriptorSet(
          descriptorSets.postprocess, 3, 0,
          vk::DescriptorType::eCombinedImageSampler, 1,
          DescriptorImageInfo(
            vk::ImageLayout::eShaderReadOnlyOptimal,
            std::make_shared<vk::ImageView>( *fbo->_colorAttachments[ 2 ].view ),
            std::make_shared<vk::Sampler>( fbo->colorSampler )
          ), nullptr
        )
      };
      _device->updateDescriptorSets( wdss, {} );
    }
  }
  
  // Update fragment shader light position uniform block
  void updateUniformBufferDeferredLights( float timer )
  {
    // White
    uboPP.lights[ 0 ].position = glm::vec4( 0.0f, 0.0f, 1.0f, 0.0f );
    uboPP.lights[ 0 ].color = glm::vec3( 1.5f );
    //uboPP.lights[ 0 ].radius = 15.0f * 0.25f;
    // Red
    uboPP.lights[ 1 ].position = glm::vec4( -2.0f, 0.0f, 0.0f, 0.0f );
    uboPP.lights[ 1 ].color = glm::vec3( 1.0f, 0.0f, 0.0f );
    //uboPP.lights[ 1 ].radius = 15.0f;
    // Blue
    uboPP.lights[ 2 ].position = glm::vec4( 2.0f, 1.0f, 0.0f, 0.0f );
    uboPP.lights[ 2 ].color = glm::vec3( 0.0f, 0.0f, 2.5f );
    //uboPP.lights[ 2 ].radius = 5.0f;
    // Yellow
    uboPP.lights[ 3 ].position = glm::vec4( 0.0f, 0.9f, 0.5f, 0.0f );
    uboPP.lights[ 3 ].color = glm::vec3( 1.0f, 1.0f, 0.0f );
    //uboPP.lights[ 3 ].radius = 2.0f;
    // Green
    uboPP.lights[ 4 ].position = glm::vec4( 0.0f, 0.5f, 0.0f, 0.0f );
    uboPP.lights[ 4 ].color = glm::vec3( 0.0f, 1.0f, 0.2f );
    //uboPP.lights[ 4 ].radius = 5.0f;
    // Yellow
    uboPP.lights[ 5 ].position = glm::vec4( 0.0f, 1.0f, 0.0f, 0.0f );
    uboPP.lights[ 5 ].color = glm::vec3( 1.0f, 0.7f, 0.3f );
    //uboPP.lights[ 5 ].radius = 25.0f;

    uboPP.lights[ 0 ].position.x = sin( glm::radians( 360.0f * timer ) ) * 5.0f;
    uboPP.lights[ 0 ].position.z = cos( glm::radians( 360.0f * timer ) ) * 5.0f;

    uboPP.lights[ 1 ].position.x = -4.0f + sin( glm::radians( 360.0f * timer ) + 45.0f ) * 2.0f;
    uboPP.lights[ 1 ].position.z = 0.0f + cos( glm::radians( 360.0f * timer ) + 45.0f ) * 2.0f;

    uboPP.lights[ 2 ].position.x = 4.0f + sin( glm::radians( 360.0f * timer ) ) * 2.0f;
    uboPP.lights[ 2 ].position.z = 0.0f + cos( glm::radians( 360.0f * timer ) ) * 2.0f;

    uboPP.lights[ 4 ].position.x = 0.0f + sin( glm::radians( 360.0f * timer + 90.0f ) ) * 5.0f;
    uboPP.lights[ 4 ].position.z = 0.0f - cos( glm::radians( 360.0f * timer + 45.0f ) ) * 5.0f;

    uboPP.lights[ 5 ].position.x = 0.0f + sin( glm::radians( -360.0f * timer + 135.0f ) ) * 10.0f;
    uboPP.lights[ 5 ].position.z = 0.0f - cos( glm::radians( -360.0f * timer - 45.0f ) ) * 10.0f;

    // Current view position
    uboPP.viewPos = camera.Position * glm::vec3( -1.0f, 1.0f, -1.0f );

    uniforms.postprocess->writeData( 0, sizeof( uboPP ), &uboPP );
  }
  void updateUniformBuffers( void )
  {
    uint32_t width = _window->getWidth( );
    uint32_t height = _window->getHeight( );

    static auto startTime = std::chrono::high_resolution_clock::now( );

    auto currentTime = std::chrono::high_resolution_clock::now( );
    float time = std::chrono::duration_cast<std::chrono::milliseconds>( currentTime - startTime ).count( ) / 1000.0f;

    float currentFrame = time;
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    uboMRT.model = glm::rotate( glm::mat4( 1.0f ), time * glm::radians( 90.0f ), glm::vec3( 0.0f, 1.0f, 0.0f ) );
    glm::vec3 cameraPos = glm::vec3( 0.0f, 0.0f, 3.0f );
    glm::vec3 cameraFront = glm::vec3( 0.0f, 0.0f, -1.0f );
    glm::vec3 cameraUp = glm::vec3( 0.0f, 1.0f, 0.0f );

    uboMRT.view = glm::lookAt( cameraPos, cameraPos + cameraFront, cameraUp );
    uboMRT.proj = glm::perspective( glm::radians( 45.0f ), width / ( float ) height, 0.1f, 10.0f );
    
    uboMRT.view = camera.GetViewMatrix( );
    uboMRT.proj = glm::perspective(glm::radians(camera.Zoom), (float)width / (float)height, 0.1f, 100.0f);

    uboMRT.proj[ 1 ][ 1 ] *= -1;

    uniforms.mrt->writeData( 0, sizeof( uboMRT ), &uboMRT );

    updateUniformBufferDeferredLights( time );
  }

  std::shared_ptr<CommandBuffer> cmdMRTBuffer;
  void buildCmdBuffers( void )
  {
    uint32_t width = _defaultFramebuffer->getExtent( ).width;
    uint32_t height = _defaultFramebuffer->getExtent( ).height;

    std::shared_ptr<CommandPool> commandPool =
      _device->createCommandPool(
        vk::CommandPoolCreateFlagBits::eResetCommandBuffer, _queueFamilyIndex );

    cmdMRTBuffer = commandPool->allocateCommandBuffer( );

    cmdMRTBuffer->beginSimple( );

    std::array<float, 4> ccv = { 0.2f, 0.3f, 0.3f, 1.0f };

    cmdMRTBuffer->beginRenderPass( fbo->renderPass,
      fbo->_fbo,
      vk::Rect2D( { 0, 0 },
        _defaultFramebuffer->getExtent( ) ),
        { 
          vk::ClearValue( ccv ),
          vk::ClearValue( ccv ),
          vk::ClearValue( ccv ),
          vk::ClearValue( vk::ClearDepthStencilValue( 1.0f, 0 ) )
        }, vk::SubpassContents::eInline
    );

    cmdMRTBuffer->setViewportScissors( width, height );
    cmdMRTBuffer->bindDescriptorSets( vk::PipelineBindPoint::eGraphics,
      pipelineLayouts.mrt, 0, { descriptorSets.mrt }, nullptr );
    cmdMRTBuffer->bindGraphicsPipeline( pipelines.mrt );
    geometry->render( cmdMRTBuffer, 9 );
    cmdMRTBuffer->endRenderPass( );

    cmdMRTBuffer->end( );


    fbo->commandBuffer = commandPool->allocateCommandBuffer( );

    fbo->commandBuffer->begin( );

    fbo->commandBuffer->beginRenderPass( _renderPass,
      _defaultFramebuffer->getFramebuffer( ),
      vk::Rect2D( { 0, 0 },
        _defaultFramebuffer->getExtent( ) ),
        { vk::ClearValue( ccv ), vk::ClearValue(
          vk::ClearDepthStencilValue( 1.0f, 0 ) )
        }, vk::SubpassContents::eInline );

    fbo->commandBuffer->setViewportScissors( width, height );
    fbo->commandBuffer->bindDescriptorSets( vk::PipelineBindPoint::eGraphics,
      pipelineLayouts.postprocess, 0, { descriptorSets.postprocess }, nullptr );
    fbo->commandBuffer->bindGraphicsPipeline( pipelines.postprocess );
    fbo->commandBuffer->draw( 4, 1, 0, 0 );
    fbo->commandBuffer->endRenderPass( );

    fbo->commandBuffer->end( );
  }

  void doPaint( void ) override
  {
    updateUniformBuffers( );
    buildCmdBuffers( );

    // Offscreen rendering
    _graphicsQueue->submit( SubmitInfo{
      { _defaultFramebuffer->getPresentSemaphore( ) }, // wait 
      { vk::PipelineStageFlagBits::eColorAttachmentOutput },
      cmdMRTBuffer,
      { fbo->semaphore }// signal
    } );

    // Scene rendering
    _graphicsQueue->submit( SubmitInfo{
      { fbo->semaphore },
      { vk::PipelineStageFlagBits::eColorAttachmentOutput },
      fbo->commandBuffer,
      _renderComplete
    } );
  }

  virtual void cursorPosEvent( double xPos, double yPos )
  {
    if (firstMouse)
    {
      lastX = xPos;
      lastY = yPos;
      firstMouse = false;
    }

    float xoffset = xPos - lastX;
    float yoffset = lastY - yPos; // reversed since y-coordinates go from bottom to top

    lastX = xPos;
    lastY = yPos;

    camera.ProcessMouseMovement(xoffset, yoffset);
  }
  virtual void keyEvent( int key, int scancode, int action, int mods )
  {
    switch ( key )
    {
    case GLFW_KEY_ESCAPE:
      switch ( action )
      {
      case GLFW_PRESS:
        getWindow( )->close( );
        break;
      default:
        break;
      }
      break;
    case GLFW_KEY_W:
       camera.ProcessKeyboard(FORWARD, deltaTime);
      break;
    case GLFW_KEY_S:
      camera.ProcessKeyboard(BACKWARD, deltaTime);
      break;
    case GLFW_KEY_A:
      camera.ProcessKeyboard(LEFT, deltaTime);
      break;
    case GLFW_KEY_D:
      camera.ProcessKeyboard(RIGHT, deltaTime);
      break;
    default:
      break;
    }
  }
};

void glfwErrorCallback( int error, const char* description )
{
  fprintf( stderr, "GLFW Error %d: %s\n", error, description );
}

int main( void )
{
  try
  {
    VulkanApp* app = new MyApp( "Deferred Shading", 
      SCR_WIDTH, SCR_HEIGHT );

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