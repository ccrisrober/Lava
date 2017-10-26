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
  class MultiLightMaterial : public lava::engine::Material
  {
  public:
    struct
    {
      glm::mat4 model;
      glm::mat4 view;
      glm::mat4 proj;
    } uboVS;


    struct Material
    {
      glm::vec3 ambient;
      glm::vec3 diffuse;
      glm::vec3 specular;
      float shininess;
    };

    struct Light
    {
      glm::vec3 position;

      glm::vec3 ambient;
      glm::vec3 diffuse;
      glm::vec3 specular;
    };

    struct
    {
      glm::vec3 viewPos;
      Material material;
      Light light;
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
        uint32_t bufferSize = sizeof( uboVS );
        uniformBufferFS = dev->createBuffer( bufferSize,
          vk::BufferUsageFlagBits::eUniformBuffer,
          vk::SharingMode::eExclusive, nullptr,
          vk::MemoryPropertyFlagBits::eHostVisible |
          vk::MemoryPropertyFlagBits::eHostCoherent );
      }

      // Init descriptor and pipeline layouts
      std::vector<DescriptorSetLayoutBinding> dslbs =
      {
        // Binding 0: MVP buffer
        DescriptorSetLayoutBinding( 0,
          vk::DescriptorType::eUniformBuffer,
          vk::ShaderStageFlagBits::eVertex
        ),
        DescriptorSetLayoutBinding( 1,
          vk::DescriptorType::eUniformBuffer,
          vk::ShaderStageFlagBits::eFragment
        )
      };
      std::shared_ptr<DescriptorSetLayout> descriptorSetLayout =
        dev->createDescriptorSetLayout( dslbs );

      _pipelineLayout = dev->createPipelineLayout( descriptorSetLayout, nullptr );

      std::array<vk::DescriptorPoolSize, 2> poolSize =
      {
        // Binding 0: MVP uniform buffers
        vk::DescriptorPoolSize( vk::DescriptorType::eUniformBuffer, 2 ),
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
          _descriptorSet, 1, 0, vk::DescriptorType::eUniformBuffer,
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
        LAVA_EXAMPLES_SPV_ROUTE + std::string( "basicLighting_vert.spv" ),
        vk::ShaderStageFlagBits::eVertex
      );
      PipelineShaderStageCreateInfo fragmentStage = dev->createShaderPipelineShaderStage(
        LAVA_EXAMPLES_SPV_ROUTE + std::string( "basicLighting_frag.spv" ),
        vk::ShaderStageFlagBits::eFragment
      );
      vk::VertexInputBindingDescription binding( 0, sizeof( Vertex ), vk::VertexInputRate::eVertex );

      PipelineVertexInputStateCreateInfo vertexInput( binding,
      {
        vk::VertexInputAttributeDescription( 0, 0, vk::Format::eR32G32B32Sfloat,
        offsetof( Vertex, pos )
        ),
        vk::VertexInputAttributeDescription( 1, 0, vk::Format::eR32G32B32Sfloat,
          offsetof( Vertex, normal )
        )
      }
      );
      vk::PipelineInputAssemblyStateCreateInfo assembly( {}, vk::PrimitiveTopology::eTriangleList, VK_FALSE );
      PipelineViewportStateCreateInfo viewport( { {} }, { {} } );
      vk::PipelineRasterizationStateCreateInfo rasterization( {}, true,
        false, vk::PolygonMode::eFill, vk::CullModeFlagBits::eBack,
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
    }
    std::shared_ptr<Buffer> uniformBufferMVP;
    std::shared_ptr<Buffer> uniformBufferFS;
  protected:
    std::shared_ptr<DescriptorSet> _descriptorSet;
  };

  class LightMaterial : public lava::engine::Material
  {
  public:
    struct
    {
      glm::mat4 model;
      glm::mat4 view;
      glm::mat4 proj;
      glm::vec3 color;
    } uboVS;

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

      // Init descriptor and pipeline layouts
      std::vector<DescriptorSetLayoutBinding> dslbs =
      {
        // Binding 0: MVP buffer
        DescriptorSetLayoutBinding( 0,
          vk::DescriptorType::eUniformBuffer,
          vk::ShaderStageFlagBits::eVertex
        )
      };
      std::shared_ptr<DescriptorSetLayout> descriptorSetLayout =
        dev->createDescriptorSetLayout( dslbs );

      _pipelineLayout = dev->createPipelineLayout( descriptorSetLayout, nullptr );

      std::array<vk::DescriptorPoolSize, 2> poolSize =
      {
        // Binding 0: MVP uniform buffer
        vk::DescriptorPoolSize( vk::DescriptorType::eUniformBuffer, 1 )
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
        )
      };
      dev->updateDescriptorSets( wdss, {} );

      // init pipeline
      std::shared_ptr<PipelineCache> pipelineCache = dev->createPipelineCache( 0, nullptr );
      PipelineShaderStageCreateInfo vertexStage = dev->createShaderPipelineShaderStage(
        LAVA_EXAMPLES_SPV_ROUTE + std::string( "lightOne_vert.spv" ),
        vk::ShaderStageFlagBits::eVertex
      );
      PipelineShaderStageCreateInfo fragmentStage = dev->createShaderPipelineShaderStage(
        LAVA_EXAMPLES_SPV_ROUTE + std::string( "light_frag.spv" ),
        vk::ShaderStageFlagBits::eFragment
      );
      vk::VertexInputBindingDescription binding( 0, sizeof( Vertex ), vk::VertexInputRate::eVertex );

      PipelineVertexInputStateCreateInfo vertexInput( binding,
      {
        vk::VertexInputAttributeDescription( 0, 0, vk::Format::eR32G32B32Sfloat,
          offsetof( Vertex, pos )
        )
      }
      );
      vk::PipelineInputAssemblyStateCreateInfo assembly( {}, vk::PrimitiveTopology::eTriangleList, VK_FALSE );
      PipelineViewportStateCreateInfo viewport( { {} }, { {} } );
      vk::PipelineRasterizationStateCreateInfo rasterization( {}, true,
        false, vk::PolygonMode::eFill, vk::CullModeFlagBits::eBack,
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
    }
    std::shared_ptr<Buffer> uniformBufferMVP;
  protected:
    std::shared_ptr<DescriptorSet> _descriptorSet;
  };
}

const std::vector<material::MultiLightMaterial::Vertex> vertices =
{
  {{-0.5f, -0.5f,  0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
  {{ 0.5f, -0.5f,  0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},
  {{-0.5f,  0.5f,  0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
  {{ 0.5f,  0.5f,  0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},

  {{-0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f}},
  {{ 0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {1.0f, 0.0f}},
  {{-0.5f,  0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {0.0f, 1.0f}},
  {{ 0.5f,  0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {1.0f, 1.0f}},

  {{ 0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
  {{ 0.5f, -0.5f,  0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
  {{ 0.5f,  0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}},
  {{ 0.5f,  0.5f,  0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}},

  {{-0.5f, -0.5f, -0.5f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
  {{-0.5f, -0.5f,  0.5f}, {-1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
  {{-0.5f,  0.5f, -0.5f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}},
  {{-0.5f,  0.5f,  0.5f}, {-1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}},

  {{-0.5f,  0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
  {{-0.5f,  0.5f,  0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
  {{ 0.5f,  0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f}},
  {{ 0.5f,  0.5f,  0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f}},

  {{-0.5f, -0.5f, -0.5f}, {0.0f, -1.0f, 0.0f}, {0.0f, 0.0f}},
  {{-0.5f, -0.5f,  0.5f}, {0.0f, -1.0f, 0.0f}, {1.0f, 0.0f}},
  {{ 0.5f, -0.5f, -0.5f}, {0.0f, -1.0f, 0.0f}, {0.0f, 1.0f}},
  {{ 0.5f, -0.5f,  0.5f}, {0.0f, -1.0f, 0.0f}, {1.0f, 1.0f}} 
};
const std::vector<uint16_t> indices =
{
  0,1,2,      1,3,2,
  4,6,5,      5,6,7,
  8,10,9,     9,10,11,
  12,13,14,   13,15,14,
  16,17,18,   17,19,18,
  20,22,21,   21,22,23,
};

class MyApp : public VulkanApp
{
public:
  std::shared_ptr<VertexBuffer> _vertexBuffer;
  std::shared_ptr<IndexBuffer> _indexBuffer;
  std::shared_ptr<material::MultiLightMaterial> material;
  std::shared_ptr<material::LightMaterial> material2;
  std::shared_ptr<CommandPool> commandPool;
  MyApp( char const* title, uint32_t width, uint32_t height )
    : VulkanApp( title, width, height )
  {
    // Vertex buffer
    {
      uint32_t vertexBufferSize = vertices.size( ) * sizeof( material::MultiLightMaterial::Vertex );
      _vertexBuffer = std::make_shared<VertexBuffer>( _device, vertexBufferSize );
      _vertexBuffer->writeData( 0, vertexBufferSize, vertices.data( ) );
    }

    // Index buffer
    {
      uint32_t indexBufferSize = indices.size( ) * sizeof( indices[ 0 ] );
      _indexBuffer = std::make_shared<IndexBuffer>( _device, 
        vk::IndexType::eUint16, indices.size( ) );
      _indexBuffer->writeData( 0, indexBufferSize, indices.data( ) );
    }

    // create a command pool for command buffer allocation
    commandPool = _device->createCommandPool(
      vk::CommandPoolCreateFlagBits::eResetCommandBuffer, _queueFamilyIndex );

    material = std::make_shared<material::MultiLightMaterial>( );
    material->configure( LAVA_EXAMPLES_SPV_ROUTE, _device, _renderPass );

    material2 = std::make_shared<material::LightMaterial>( );
    material2->configure( LAVA_EXAMPLES_SPV_ROUTE, _device, _renderPass );
  }
  const glm::vec3 lightPos = glm::vec3( 1.2f, 1.0f, 2.0f );
  void updateUniformBuffers( void )
  {
    uint32_t width = _window->getWidth( );
    uint32_t height = _window->getHeight( );

    static auto startTime = std::chrono::high_resolution_clock::now();

    auto currentTime = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - startTime).count() / 1000.0f;

    float currentFrame = time;
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    glm::mat4 model;
    material->uboVS.model = model;
    material->uboVS.view = camera.GetViewMatrix( );
    material->uboVS.proj = glm::perspective( glm::radians( camera.Zoom ), ( float ) width / ( float ) height, 0.1f, 100.0f );
    material->uboVS.proj[1][1] *= -1;



    material->uboFS.light.position = lightPos;
    material->uboFS.viewPos = camera.Position;

    // Light properties
    glm::vec3 lightColor;
    lightColor.x = sin( time * 2.0f );
    lightColor.y = sin( time * 0.7f );
    lightColor.z = sin( time * 1.3f );
    glm::vec3 diffuseColor = lightColor   * glm::vec3( 0.5f ); // decrease the influence
    glm::vec3 ambientColor = diffuseColor * glm::vec3( 0.2f ); // low influence
    material->uboFS.light.ambient = glm::vec3( 1.0f, 1.0f, 1.0f );
    material->uboFS.light.ambient = ambientColor;
    material->uboFS.light.diffuse = diffuseColor;

    // Material properties
    material->uboFS.material.ambient = glm::vec3( 1.0f, 0.5f, 0.31f );
    material->uboFS.material.diffuse = glm::vec3( 1.0f, 0.5f, 0.31f );
    material->uboFS.material.specular = glm::vec3( 0.5f, 0.5f, 0.5f ); // specular lighting doesn't have full effect on this object's material
    material->uboFS.material.shininess = 32.0f;

    material2->uboVS.color = lightColor;

    model = glm::mat4( );
    model = glm::translate( model, lightPos );
    model = glm::scale( model, glm::vec3( 0.2f ) ); // a smaller cube
    material2->uboVS.model = model;
    material2->uboVS.view = material->uboVS.view;
    material2->uboVS.proj = material->uboVS.proj;


    material->uniformBufferMVP->writeData( 0, sizeof( material->uboVS ), &material->uboVS );
    material->uniformBufferFS->writeData( 0, sizeof( material->uboFS ), &material->uboFS );
    material2->uniformBufferMVP->writeData( 0, sizeof( material2->uboVS ), &material2->uboVS );
  }

  void doPaint( void ) override
  {
    updateUniformBuffers( );

    std::shared_ptr<CommandBuffer> commandBuffer = commandPool->allocateCommandBuffer( );

    commandBuffer->beginSimple( );

    std::array<float, 4> ccv = { 0.1f, 0.1f, 0.1f, 1.0f };
    commandBuffer->beginRenderPass( _renderPass, 
      _defaultFramebuffer->getFramebuffer( ), 
      vk::Rect2D( { 0, 0 }, 
      _defaultFramebuffer->getExtent( ) ),
      { vk::ClearValue( ccv ), vk::ClearValue( 
        vk::ClearDepthStencilValue( 1.0f, 0 ) )
      }, vk::SubpassContents::eInline );

    commandBuffer->setViewportScissors( _defaultFramebuffer->getExtent( ) );

    _vertexBuffer->bind( commandBuffer );
    _indexBuffer->bind( commandBuffer );

    material->bind( commandBuffer );
    commandBuffer->drawIndexed( indices.size( ), 1, 0, 0, 0 );

    material2->bind( commandBuffer );
    commandBuffer->drawIndexed( indices.size( ), 1, 0, 0, 0 );


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
    case GLFW_KEY_W:
      switch ( action )
      {
      case GLFW_PRESS:
        camera.ProcessKeyboard( FORWARD, deltaTime );
        break;
      default:
        break;
      }
      break;
    case GLFW_KEY_S:
      switch ( action )
      {
      case GLFW_PRESS:
        camera.ProcessKeyboard( BACKWARD, deltaTime );
        break;
      default:
        break;
      }
      break;
    case GLFW_KEY_A:
      switch ( action )
      {
      case GLFW_PRESS:
        camera.ProcessKeyboard( LEFT, deltaTime );
        break;
      default:
        break;
      }
      break;
    case GLFW_KEY_D:
      switch ( action )
      {
      case GLFW_PRESS:
        camera.ProcessKeyboard( RIGHT, deltaTime );
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

void glfwErrorCallback( int error, const char* description )
{
  fprintf( stderr, "GLFW Error %d: %s\n", error, description );
}

int main( void )
{
  try
  {
    VulkanApp* app = new MyApp( "Cube UV", SCR_WIDTH, SCR_HEIGHT );

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