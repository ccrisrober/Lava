#include <lava/lava.h>
using namespace lava;

#include <routes.h>

#include "utils/Camera.h"

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// camera
Camera camera( glm::vec3( 0.0f, 0.0f, 3.0f ) );
// timing
float deltaTime = 0.0f; // time between current frame and last frame
float lastFrame = 0.0f;

float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

namespace material
{
  class SpecularMaterial : public lava::engine::Material
  {
  public:
    struct
    {
      glm::mat4 model;
      glm::mat4 view;
      glm::mat4 proj;
    } uboVS;
    struct
    {
      //bool mode;
      glm::vec4 lightPositions[ 4 ];
      glm::vec4 lightColor[ 4 ];
      glm::vec3 viewPos;
      //float gamma = 0.0f;
    } uboFS;

    struct Vertex
    {
      glm::vec3 pos;
      glm::vec3 normal;
      glm::vec2 texCoord;
    };

    virtual void configure( const std::string& dir,
      std::shared_ptr< Device > dev,
      std::shared_ptr<RenderPass> renderPass )
    {
      // MVP buffer
      {
        uint32_t bufferSize = sizeof( uboVS );
        uniformBufferMVP = dev->createBuffer( bufferSize,
          vk::BufferUsageFlagBits::eUniformBuffer,
          vk::SharingMode::eExclusive, nullptr,
          vk::MemoryPropertyFlagBits::eHostVisible |
          vk::MemoryPropertyFlagBits::eHostCoherent );
      }
      {
        uint32_t bufferSize = sizeof( uboFS );
        uniformBufferFS = dev->createBuffer( bufferSize,
          vk::BufferUsageFlagBits::eUniformBuffer,
          vk::SharingMode::eExclusive, nullptr,
          vk::MemoryPropertyFlagBits::eHostVisible |
          vk::MemoryPropertyFlagBits::eHostCoherent );
      }

      // Init descriptor and pipeline layouts
      std::vector<DescriptorSetLayoutBinding> dslbs =
      {
        // Binding 0: MVP uniform buffer
        DescriptorSetLayoutBinding( 0,
        vk::DescriptorType::eUniformBuffer,
        vk::ShaderStageFlagBits::eVertex
        ),
        // Binding 1: Albedo texture
        DescriptorSetLayoutBinding( 1,
          vk::DescriptorType::eCombinedImageSampler,
          vk::ShaderStageFlagBits::eFragment
        ),
        // Binding 2: Specular texture
        DescriptorSetLayoutBinding( 2,
          vk::DescriptorType::eCombinedImageSampler,
          vk::ShaderStageFlagBits::eFragment
        ),
        // Binding 3: Fragment shader uniform buffer
        DescriptorSetLayoutBinding( 3,
          vk::DescriptorType::eUniformBuffer,
          vk::ShaderStageFlagBits::eFragment
        ),
      };
      std::shared_ptr<DescriptorSetLayout> descriptorSetLayout =
        dev->createDescriptorSetLayout( dslbs );

      vk::PushConstantRange pushConstantRange( vk::ShaderStageFlagBits::eFragment,
        0, sizeof( pushConstants ) );

      _pipelineLayout = dev->createPipelineLayout( descriptorSetLayout, pushConstantRange );

      std::array<vk::DescriptorPoolSize, 2> poolSize =
      {
        // MVP + Fragment Shader uniform buffer
        vk::DescriptorPoolSize( vk::DescriptorType::eUniformBuffer, 2 ),
        // Albedo + Specular Texture
        vk::DescriptorPoolSize( vk::DescriptorType::eCombinedImageSampler, 2 )
      };
      std::shared_ptr<DescriptorPool> descriptorPool =
        dev->createDescriptorPool( {}, 1, poolSize );


      // Init descriptor set
      _descriptorSet = dev->allocateDescriptorSet( descriptorPool, descriptorSetLayout );
      std::vector<WriteDescriptorSet> wdss =
      {
        WriteDescriptorSet(
          _descriptorSet, 0, 0, vk::DescriptorType::eUniformBuffer,
          1, nullptr,
          DescriptorBufferInfo(
            uniformBufferMVP, 0, sizeof( uboVS )
          )
        ),
        WriteDescriptorSet(
          _descriptorSet, 1, 0,
          vk::DescriptorType::eCombinedImageSampler, 1,
          texAlbedo->descriptor, nullptr
        ),
        WriteDescriptorSet(
          _descriptorSet, 2, 0,
          vk::DescriptorType::eCombinedImageSampler, 1,
          texSpec->descriptor, nullptr
        ),
        WriteDescriptorSet(
          _descriptorSet, 3, 0, vk::DescriptorType::eUniformBuffer,
          1, nullptr,
          DescriptorBufferInfo(
            uniformBufferFS, 0, sizeof( uboFS )
          )
        )
      };
      dev->updateDescriptorSets( wdss, {} );

      // init pipeline
      std::shared_ptr<PipelineCache> pipelineCache = dev->createPipelineCache( 0, nullptr );
      PipelineShaderStageCreateInfo vertexStage = dev->createShaderPipelineShaderStage(
        LAVA_EXAMPLES_SPV_ROUTE + std::string( "blinnspec_vert.spv" ),
        vk::ShaderStageFlagBits::eVertex
      );
      PipelineShaderStageCreateInfo fragmentStage = dev->createShaderPipelineShaderStage(
        LAVA_EXAMPLES_SPV_ROUTE + std::string( "blinnspec_frag.spv" ),
        vk::ShaderStageFlagBits::eFragment
      );
      vk::VertexInputBindingDescription binding( 0, sizeof( Vertex ), vk::VertexInputRate::eVertex );

      PipelineVertexInputStateCreateInfo vertexInput( binding,
      {
        vk::VertexInputAttributeDescription( 0, 0,
        vk::Format::eR32G32B32Sfloat, offsetof( Vertex, pos )
        ),
        vk::VertexInputAttributeDescription( 1, 0,
          vk::Format::eR32G32B32Sfloat, offsetof( Vertex, normal )
        ),
        vk::VertexInputAttributeDescription( 2, 0,
          vk::Format::eR32G32Sfloat, offsetof( Vertex, texCoord )
        )
      }
      );
      vk::PipelineInputAssemblyStateCreateInfo assembly( {}, vk::PrimitiveTopology::eTriangleList, VK_FALSE );
      PipelineViewportStateCreateInfo viewport( { {} }, { {} } );
      vk::PipelineRasterizationStateCreateInfo rasterization( {}, true,
        false, vk::PolygonMode::eFill, vk::CullModeFlagBits::eNone,
        vk::FrontFace::eCounterClockwise, false, 0.0f, 0.0f, 0.0f, 1.0f );
      PipelineMultisampleStateCreateInfo multisample( vk::SampleCountFlagBits::e1, false, 0.0f, nullptr, false, false );
      vk::StencilOpState stencilOpState( vk::StencilOp::eKeep, vk::StencilOp::eKeep, vk::StencilOp::eKeep, vk::CompareOp::eAlways, 0, 0, 0 );
      vk::PipelineDepthStencilStateCreateInfo depthStencil( {}, true, true, vk::CompareOp::eLessOrEqual, false, false, stencilOpState, stencilOpState, 0.0f, 0.0f );
      vk::PipelineColorBlendAttachmentState colorBlendAttachment( false, vk::BlendFactor::eZero, vk::BlendFactor::eZero, vk::BlendOp::eAdd, vk::BlendFactor::eZero, vk::BlendFactor::eZero, vk::BlendOp::eAdd,
        vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA );
      PipelineColorBlendStateCreateInfo colorBlend( false, vk::LogicOp::eNoOp, colorBlendAttachment, { 1.0f, 1.0f, 1.0f, 1.0f } );
      PipelineDynamicStateCreateInfo dynamic( { vk::DynamicState::eViewport, vk::DynamicState::eScissor } );


      _pipeline = dev->createGraphicsPipeline( pipelineCache, {},
      { vertexStage, fragmentStage }, vertexInput, assembly,
        nullptr, viewport, rasterization, multisample,
        depthStencil, colorBlend, dynamic,
        _pipelineLayout, renderPass );
    }
    virtual void bind( std::shared_ptr< CommandBuffer > cmd )
    {
      lava::engine::Material::bind( cmd );
      if ( _descriptorSet )
      {
        cmd->bindDescriptorSets( vk::PipelineBindPoint::eGraphics,
          _pipelineLayout, 0, { _descriptorSet }, nullptr );
      }
      cmd->pushConstants<VkBool32>( *_pipelineLayout,
        vk::ShaderStageFlagBits::eFragment, 0, pushConstants );
    }
    std::shared_ptr<Texture2D> texAlbedo;
    std::shared_ptr<Texture2D> texSpec;
    std::shared_ptr<Buffer> uniformBufferMVP;
    std::shared_ptr<Buffer> uniformBufferFS;

    std::array<VkBool32, 1> pushConstants;
  protected:
    std::shared_ptr<DescriptorSet> _descriptorSet;
  };
}

const std::vector<material::SpecularMaterial::Vertex> vertices =
{
  { { 10.0f, -0.5f,  10.0f },{ 0.0f, 1.0f, 0.0f },{ 1.0f, 0.0f } },
  { { -10.0f, -0.5f,  10.0f },{ 0.0f, 1.0f, 0.0f },{ 0.0f, 0.0f } },
  { { -10.0f, -0.5f, -10.0f },{ 0.0f, 1.0f, 0.0f },{ 0.0f, 1.0f } },

  { { 10.0f, -0.5f,  10.0f },{ 0.0f, 1.0f, 0.0f },{ 1.0f, 0.0f } },
  { { -10.0f, -0.5f, -10.0f },{ 0.0f, 1.0f, 0.0f },{ 0.0f, 1.0f } },
  { { 10.0f, -0.5f, -10.0f },{ 0.0f, 1.0f, 0.0f },{ 1.0f, 1.0f } }
};
class MyApp : public VulkanApp
{
public:
  std::shared_ptr<VertexBuffer> _vertexBuffer;
  std::shared_ptr<material::SpecularMaterial> material;
  std::shared_ptr<CommandPool> commandPool;
  MyApp( char const* title, uint32_t width, uint32_t height )
    : VulkanApp( title, width, height )
  {
    // create a command pool for command buffer allocation
    commandPool = _device->createCommandPool(
      vk::CommandPoolCreateFlagBits::eResetCommandBuffer, _queueFamilyIndex );
    
    // Vertex buffer
    {
      uint32_t vertexBufferSize = vertices.size( ) * sizeof( material::SpecularMaterial::Vertex );
      _vertexBuffer = std::make_shared<VertexBuffer>( _device, vertexBufferSize );
      _vertexBuffer->writeData( 0, vertexBufferSize, vertices.data( ) );
    }
    material = std::make_shared<material::SpecularMaterial>( );

    material->texAlbedo = std::make_shared<Texture2D>( _device, LAVA_EXAMPLES_IMAGES_ROUTE +
      std::string( "tileable-light-wood-textures-5.jpg" ), commandPool, _graphicsQueue,
      vk::Format::eR8G8B8A8Unorm );
    material->texSpec = std::make_shared<Texture2D>( _device, LAVA_EXAMPLES_IMAGES_ROUTE +
      std::string( "WoodSpecularMap.png" ), commandPool, _graphicsQueue,
      vk::Format::eR8G8B8A8Unorm );

    material->configure( LAVA_EXAMPLES_SPV_ROUTE, _device, _renderPass );

    material->uboFS.lightPositions[ 0 ] = glm::vec4( -3.0f, 0.0f, 0.0f, 1.0f );
    material->uboFS.lightPositions[ 1 ] = glm::vec4( -1.0f, 0.0f, 0.0f, 1.0f );
    material->uboFS.lightPositions[ 2 ] = glm::vec4( 1.0f, 0.0f, 0.0f, 1.0f );
    material->uboFS.lightPositions[ 3 ] = glm::vec4( 3.0f, 0.0f, 0.0f, 1.0f );

    material->uboFS.lightColor[ 0 ] = glm::vec4( 1.0f, 0.0f, 0.0f, 1.0f );
    material->uboFS.lightColor[ 1 ] = glm::vec4( 0.0f, 1.0f, 0.0f, 1.0f );
    material->uboFS.lightColor[ 2 ] = glm::vec4( 0.0f, 0.0f, 1.0f, 1.0f );
    material->uboFS.lightColor[ 3 ] = glm::vec4( 1.0f, 1.0f, 1.0f, 1.0f );
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

    material->uboVS.model = glm::mat4( 1.0f );
    material->uboVS.view = camera.GetViewMatrix( );
    material->uboVS.proj = glm::perspective( glm::radians( camera.Zoom ), ( float ) width / ( float ) height, 0.1f, 100.0f );
    material->uboVS.proj[ 1 ][ 1 ] *= -1;

    material->uniformBufferMVP->writeData( 0, sizeof( material->uboVS ), &material->uboVS );

    material->uboFS.viewPos = camera.Position;

    material->uniformBufferFS->writeData( 0, sizeof( material->uboFS ), &material->uboFS );

    std::cout << material->pushConstants[0] << std::endl;
  }

  void doPaint( void ) override
  {
    updateUniformBuffers( );

    std::shared_ptr<CommandBuffer> commandBuffer = 
      commandPool->allocateCommandBuffer( );

    commandBuffer->beginSimple( );

    std::array<float, 4> ccv = { 0.1f, 0.1f, 0.1f, 1.0f };
    commandBuffer->beginRenderPass( _renderPass,
      _defaultFramebuffer->getFramebuffer( ),
      vk::Rect2D( { 0, 0 },
        _defaultFramebuffer->getExtent( ) ),
        { vk::ClearValue( ccv ), vk::ClearValue(
          vk::ClearDepthStencilValue( 1.0f, 0 ) )
        }, vk::SubpassContents::eInline );

    material->bind( commandBuffer );

    _vertexBuffer->bind( commandBuffer );
    commandBuffer->setViewportScissors( _defaultFramebuffer->getExtent( ) );

    commandBuffer->draw( 6, 1, 0, 0 );
    commandBuffer->endRenderPass( );

    commandBuffer->end( );

    _graphicsQueue->submit( SubmitInfo{
      { _defaultFramebuffer->getPresentSemaphore( ) },
      { vk::PipelineStageFlagBits::eColorAttachmentOutput },
      commandBuffer,
      _renderComplete
    } );
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
    case GLFW_KEY_C:
      material->pushConstants[ 0 ] = false;
      break;
    case GLFW_KEY_V:
      material->pushConstants[ 0 ] = true;
      break;
      /*case GLFW_KEY_X:
      material->uboFS.mode = true;
      break;
    case GLFW_KEY_Z:
      material->uboFS.mode = false;
      break;*/
    case GLFW_KEY_W:
      camera.ProcessKeyboard( FORWARD, deltaTime );
      break;
    case GLFW_KEY_S:
      camera.ProcessKeyboard( BACKWARD, deltaTime );
      break;
    case GLFW_KEY_A:
      camera.ProcessKeyboard( LEFT, deltaTime );
      break;
    case GLFW_KEY_D:
      camera.ProcessKeyboard( RIGHT, deltaTime );
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
void glfwErrorCallback( int error, const char* description )
{
  fprintf( stderr, "GLFW Error %d: %s\n", error, description );
}

int main( void )
{
  try
  {
    VulkanApp* app = new MyApp( "Specular Lights", SCR_WIDTH, SCR_HEIGHT );

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