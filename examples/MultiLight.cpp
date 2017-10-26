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
      glm::mat4 view;
      glm::mat4 proj;

      glm::mat4 modelInstance[ 10 ];
    } uboVS;

    struct DirLight
    {
      glm::vec3 direction;

      glm::vec3 ambient;
      glm::vec3 diffuse;
      glm::vec3 specular;
    };

    struct PointLight
    {
      glm::vec3 position;

      glm::vec3 ambient;
      glm::vec3 diffuse;
      glm::vec3 specular;
    };

    struct SpotLight
    {
      glm::vec3 position;
      glm::vec3 direction;
      float cutOff;
      float outerCutOff;

      glm::vec3 ambient;
      glm::vec3 diffuse;
      glm::vec3 specular;
    };

    struct
    {
      glm::vec3 viewPos;
      DirLight dirLight;
      PointLight pointLights[ 4 ];
      SpotLight spotLight;
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
      // Lights buffer
      {
        uint32_t bufferSize = sizeof( uboFS );
        uniformBufferLights = dev->createBuffer( bufferSize,
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
        // Binding 3: Lights uniform buffer
        DescriptorSetLayoutBinding( 3,
          vk::DescriptorType::eUniformBuffer,
          vk::ShaderStageFlagBits::eFragment
        )
      };
      std::shared_ptr<DescriptorSetLayout> descriptorSetLayout =
        dev->createDescriptorSetLayout( dslbs );

      _pipelineLayout = dev->createPipelineLayout( descriptorSetLayout, nullptr );

      std::array<vk::DescriptorPoolSize, 2> poolSize =
      {
        // Binding 0: MVP and light uniform buffers
        vk::DescriptorPoolSize( vk::DescriptorType::eUniformBuffer, 2 ),
        // Binding 1: Textures (albedo + specular)
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
            uniformBufferLights, 0, sizeof( uboFS )
          )
        )
      };
      dev->updateDescriptorSets( wdss, {} );

      // init pipeline
      std::shared_ptr<PipelineCache> pipelineCache = dev->createPipelineCache( 0, nullptr );
      PipelineShaderStageCreateInfo vertexStage = dev->createShaderPipelineShaderStage(
        LAVA_EXAMPLES_SPV_ROUTE + std::string( "cubeMultiLight_vert.spv" ),
        vk::ShaderStageFlagBits::eVertex
      );
      PipelineShaderStageCreateInfo fragmentStage = dev->createShaderPipelineShaderStage(
        LAVA_EXAMPLES_SPV_ROUTE + std::string( "cubeMultiLight_frag.spv" ),
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
        ),
        vk::VertexInputAttributeDescription( 2, 0, vk::Format::eR32G32Sfloat,
          offsetof( Vertex, texCoord )
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
    std::shared_ptr<Texture2D> texAlbedo;
    std::shared_ptr<Texture2D> texSpec;
    std::shared_ptr<Buffer> uniformBufferMVP;
    std::shared_ptr<Buffer> uniformBufferLights;
  protected:
    std::shared_ptr<DescriptorSet> _descriptorSet;
  };

  class LightMaterial : public lava::engine::Material
  {
  public:
    struct
    {
      glm::mat4 view;
      glm::mat4 proj;

      glm::mat4 modelInstance[ 4 ];
      glm::vec3 colorInstance[ 4 ];
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
        LAVA_EXAMPLES_SPV_ROUTE + std::string( "light_vert.spv" ),
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

// positions all containers
const std::vector<glm::vec3> cubePositions =
{
  glm::vec3( 0.0f,  0.0f,  0.0f ),
  glm::vec3( 2.0f,  5.0f, -15.0f ),
  glm::vec3( -1.5f, -2.2f, -2.5f ),
  glm::vec3( -3.8f, -2.0f, -12.3f ),
  glm::vec3( 2.4f, -0.4f, -3.5f ),
  glm::vec3( -1.7f,  3.0f, -7.5f ),
  glm::vec3( 1.3f, -2.0f, -2.5f ),
  glm::vec3( 1.5f,  2.0f, -2.5f ),
  glm::vec3( 1.5f,  0.2f, -1.5f ),
  glm::vec3( -1.3f,  1.0f, -1.5f )
};

// positions of the point lights
const std::vector<glm::vec3> pointLightPositions =
{
  glm::vec3( 0.7f,  0.2f,  2.0f ),
  glm::vec3( 2.3f, -3.3f, -4.0f ),
  glm::vec3( -4.0f,  2.0f, -12.0f ),
  glm::vec3( 0.0f,  0.0f, -3.0f )
};

// colors of the point lights
const std::vector<glm::vec3> pointLightColors =
{
  glm::vec3( 0.1f, 0.1f, 0.1f ),
  glm::vec3( 0.1f, 0.1f, 0.1f ),
  glm::vec3( 0.1f, 0.1f, 0.1f ),
  glm::vec3( 0.3f, 0.1f, 0.1f )
};


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

    material = std::make_shared<material::MultiLightMaterial>( );

    std::shared_ptr<CommandPool> commandPool = _device->createCommandPool(
      vk::CommandPoolCreateFlagBits::eResetCommandBuffer, _queueFamilyIndex );
    material->texAlbedo = std::make_shared<Texture2D>( _device, 
      LAVA_EXAMPLES_IMAGES_ROUTE + std::string( "cube_diffuse.png" ), 
      commandPool, _graphicsQueue, vk::Format::eR8G8B8A8Unorm );
    material->texSpec = std::make_shared<Texture2D>( _device, 
      LAVA_EXAMPLES_IMAGES_ROUTE + std::string( "cube_specular.png" ), 
      commandPool, _graphicsQueue, vk::Format::eR8G8B8A8Unorm );

    material->configure( LAVA_EXAMPLES_SPV_ROUTE, _device, _renderPass );

    material2 = std::make_shared<material::LightMaterial>( );
    material2->configure( LAVA_EXAMPLES_SPV_ROUTE, _device, _renderPass );

    material->uboFS.dirLight.direction = glm::vec3( -0.2f, -1.0f, -0.3f );
    material->uboFS.dirLight.ambient = glm::vec3( 0.05f, 0.05f, 0.1f );
    material->uboFS.dirLight.diffuse = glm::vec3( 0.2f, 0.2f, 0.7f );
    material->uboFS.dirLight.specular = glm::vec3( 0.7f, 0.7f, 0.7f );
    // point light 1
    material->uboFS.pointLights[0].position = pointLightPositions[0];
    material->uboFS.pointLights[ 0 ].ambient = glm::vec3( pointLightColors[ 0 ].x * 0.1,
      pointLightColors[ 0 ].y * 0.1, pointLightColors[ 0 ].z * 0.1 );
    material->uboFS.pointLights[ 0 ].diffuse = pointLightColors[ 0 ];
    material->uboFS.pointLights[ 0 ].specular = pointLightColors[ 0 ];
    // point light 2
    material->uboFS.pointLights[1].position = pointLightPositions[1];
    material->uboFS.pointLights[ 1 ].ambient = glm::vec3(pointLightColors[ 1 ].x * 0.1, 
      pointLightColors[ 1 ].y * 0.1, pointLightColors[ 1 ].z * 0.1);
    material->uboFS.pointLights[ 1 ].diffuse = pointLightColors[ 1 ];
    material->uboFS.pointLights[ 1 ].specular = pointLightColors[ 1 ];
    // point light 3
    material->uboFS.pointLights[2].position = pointLightPositions[2];
    material->uboFS.pointLights[ 2 ].ambient = glm::vec3( pointLightColors[ 2 ].x * 0.1,
      pointLightColors[ 2 ].y * 0.1, pointLightColors[ 2 ].z * 0.1 );
    material->uboFS.pointLights[ 2 ].diffuse = pointLightColors[ 2 ];
    material->uboFS.pointLights[ 2 ].specular = pointLightColors[ 2 ];
    // point light 4
    material->uboFS.pointLights[3].position = pointLightPositions[3];
    material->uboFS.pointLights[ 3 ].ambient = glm::vec3( pointLightColors[ 3 ].x * 0.1,
      pointLightColors[ 3 ].y * 0.1, pointLightColors[ 3 ].z * 0.1 );
    material->uboFS.pointLights[ 3 ].diffuse = pointLightColors[ 3 ];
    material->uboFS.pointLights[ 3 ].specular = pointLightColors[ 3 ];
    // spotLight
    material->uboFS.spotLight.position = camera.Position;
    material->uboFS.spotLight.direction = camera.Front;
    material->uboFS.spotLight.ambient = glm::vec3( 0.0f, 0.0f, 0.0f );
    material->uboFS.spotLight.diffuse = glm::vec3( 0.8f, 0.8f, 0.0f );
    material->uboFS.spotLight.specular = glm::vec3( 1.8f, 1.8f, 1.0f );
    material->uboFS.spotLight.cutOff = glm::cos( glm::radians( 10.0f ) );
    material->uboFS.spotLight.outerCutOff = glm::cos( glm::radians( 12.5f ) );

    material->uboFS.viewPos = camera.Position;

    material->uniformBufferLights->writeData( 0, sizeof( material->uboFS ), &material->uboFS );


    for ( unsigned int i = 0; i < cubePositions.size( ); ++i )
    {
      glm::mat4 model;
      model = glm::translate( model, cubePositions[ i ] );
      float angle = 20.0f * i;
      model = glm::rotate( model, glm::radians( angle ), glm::vec3( 1.0f, 0.3f, 0.5f ) );
      material->uboVS.modelInstance[ i ] = model;
    }

    glm::mat4 model = glm::mat4( );
    for ( uint32_t i = 0; i < 4; ++i )
    {
      model = glm::mat4( );
      model = glm::translate( model, pointLightPositions[ i ] );
      model = glm::scale( model, glm::vec3( 0.2f ) );
      material2->uboVS.modelInstance[ i ] = model;
      material2->uboVS.colorInstance[ i ] = material->uboFS.pointLights[ i ].diffuse;
    }

    material2->uniformBufferMVP->writeData( 0, sizeof( material2->uboVS ), &material2->uboVS );
  }
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

    //material->uboVS.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    material->uboVS.view = camera.GetViewMatrix( );
    material->uboVS.proj = glm::perspective( glm::radians( camera.Zoom ), ( float ) width / ( float ) height, 0.1f, 100.0f );
    material->uboVS.proj[1][1] *= -1;
    material->uniformBufferMVP->writeData( 0, sizeof( material->uboVS ), &material->uboVS );

    material2->uboVS.view = material->uboVS.view;
    material2->uboVS.proj = material->uboVS.proj;
    material2->uniformBufferMVP->writeData( 0, sizeof( material2->uboVS ), &material2->uboVS );

    material->uboFS.spotLight.position = camera.Position;
    material->uboFS.spotLight.direction = camera.Front;
    material->uboFS.viewPos = camera.Position;
    material->uniformBufferLights->writeData( 0, sizeof( material->uboFS ), &material->uboFS );
  }

  void doPaint( void ) override
  {
    updateUniformBuffers( );

    // create a command pool for command buffer allocation
    std::shared_ptr<CommandPool> commandPool = 
      _device->createCommandPool( 
          vk::CommandPoolCreateFlagBits::eResetCommandBuffer, _queueFamilyIndex );
    std::shared_ptr<CommandBuffer> commandBuffer = commandPool->allocateCommandBuffer( );

    commandBuffer->begin( );

    std::array<float, 4> ccv = { 0.0f, 0.0f, 0.0f, 1.0f };
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
    commandBuffer->drawIndexed( indices.size( ), cubePositions.size( ), 0, 0, 0 );

    material2->bind( commandBuffer );
    commandBuffer->drawIndexed( indices.size( ), 4, 0, 0, 0 );


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
    VulkanApp* app = new MyApp( "MultiLight", SCR_WIDTH, SCR_HEIGHT );

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