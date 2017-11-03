#include <lava/lava.h>
using namespace lava;

#include <routes.h>

namespace material
{
  class UVMaterial: public lava::engine::Material
  {
  public:
    struct
    {
      glm::mat4 view;
      glm::mat4 proj;
    } uboVS;

    std::array<glm::mat4, 1> pushConstants; // Model matrix

    struct Vertex
    {
      glm::vec3 pos;
      glm::vec2 texCoord;
    };

    virtual void configure( const std::string& dir,
      std::shared_ptr< Device > dev,
      std::shared_ptr<RenderPass> renderPass )
    {
      // MVP buffer
      {
        uint32_t mvpBufferSize = sizeof( uboVS );
        uniformBufferMVP = dev->createBuffer( mvpBufferSize,
          vk::BufferUsageFlagBits::eUniformBuffer,
          vk::SharingMode::eExclusive, nullptr,
          vk::MemoryPropertyFlagBits::eHostVisible |
          vk::MemoryPropertyFlagBits::eHostCoherent );
      }

      // Init descriptor and pipeline layouts
      std::vector<DescriptorSetLayoutBinding> dslbs = 
      {
        DescriptorSetLayoutBinding( 0, 
          vk::DescriptorType::eUniformBuffer,
          vk::ShaderStageFlagBits::eVertex
        ),
        DescriptorSetLayoutBinding( 1, 
          vk::DescriptorType::eCombinedImageSampler,
          vk::ShaderStageFlagBits::eFragment
        )
      };
      std::shared_ptr<DescriptorSetLayout> descriptorSetLayout = 
        dev->createDescriptorSetLayout( dslbs );

      vk::PushConstantRange pushConstantRange( vk::ShaderStageFlagBits::eVertex,
        0, sizeof( pushConstants ) );

      _pipelineLayout = dev->createPipelineLayout( descriptorSetLayout, pushConstantRange );

      std::array<vk::DescriptorPoolSize, 2> poolSize =
      {
        // Binding 0: MVP uniform buffer
        vk::DescriptorPoolSize( vk::DescriptorType::eUniformBuffer, 1 ),
        // Binding 1: Texture
        vk::DescriptorPoolSize( vk::DescriptorType::eCombinedImageSampler, 1 )
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
          DescriptorImageInfo
          (
            vk::ImageLayout::eGeneral,
            std::make_shared<vk::ImageView>( tex->view ),
            std::make_shared<vk::Sampler>( tex->sampler )
          ), nullptr
        )
      };
      dev->updateDescriptorSets( wdss, {} );

      // init shaders
      std::shared_ptr<ShaderModule> vertexShaderModule =
        dev->createShaderModule( LAVA_EXAMPLES_SPV_ROUTE +
          std::string( "planar_reflection_vert.spv" ), vk::ShaderStageFlagBits::eVertex );
      std::shared_ptr<ShaderModule> fragmentShaderModule =
        dev->createShaderModule( LAVA_EXAMPLES_SPV_ROUTE +
          std::string( "planar_reflection_frag.spv" ), vk::ShaderStageFlagBits::eFragment );

      // init pipeline
      std::shared_ptr<PipelineCache> pipelineCache = dev->createPipelineCache( 0, nullptr );
      PipelineShaderStageCreateInfo vertexStage( vk::ShaderStageFlagBits::eVertex, vertexShaderModule );
      PipelineShaderStageCreateInfo fragmentStage( vk::ShaderStageFlagBits::eFragment, fragmentShaderModule );
      vk::VertexInputBindingDescription binding( 0, sizeof( Vertex ), vk::VertexInputRate::eVertex );

      PipelineVertexInputStateCreateInfo vertexInput( binding, {
        vk::VertexInputAttributeDescription( 0, 0, vk::Format::eR32G32B32Sfloat, offsetof( Vertex, pos ) ),
        vk::VertexInputAttributeDescription( 1, 0, vk::Format::eR32G32Sfloat, offsetof( Vertex, texCoord ) ) }
      );
      vk::PipelineInputAssemblyStateCreateInfo assembly( {}, vk::PrimitiveTopology::eTriangleList, VK_FALSE );
      PipelineViewportStateCreateInfo viewport( { {} }, { {} } );   // one dummy viewport and scissor, as dynamic state sets them
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
    }
    std::shared_ptr<Texture2D> tex;
    std::shared_ptr<Buffer> uniformBufferMVP;
  protected:
    std::shared_ptr<DescriptorSet> _descriptorSet;
  };


  class UVMaterialPlane : public lava::engine::Material
  {
  public:
    struct
    {
      glm::mat4 view;
      glm::mat4 proj;
    } uboVS;

    std::array<glm::mat4, 1> pushConstants; // Model matrix

    struct Vertex
    {
      glm::vec3 pos;
      glm::vec2 texCoord;
    };

    virtual void configure( const std::string& dir,
      std::shared_ptr< Device > dev,
      std::shared_ptr<RenderPass> renderPass )
    {
      // MVP buffer
      {
        uint32_t mvpBufferSize = sizeof( uboVS );
        uniformBufferMVP = dev->createBuffer( mvpBufferSize,
          vk::BufferUsageFlagBits::eUniformBuffer,
          vk::SharingMode::eExclusive, nullptr,
          vk::MemoryPropertyFlagBits::eHostVisible |
          vk::MemoryPropertyFlagBits::eHostCoherent );
      }

      // Init descriptor and pipeline layouts
      std::vector<DescriptorSetLayoutBinding> dslbs =
      {
        DescriptorSetLayoutBinding( 0,
        vk::DescriptorType::eUniformBuffer,
        vk::ShaderStageFlagBits::eVertex
        ),
        DescriptorSetLayoutBinding( 1,
          vk::DescriptorType::eCombinedImageSampler,
          vk::ShaderStageFlagBits::eFragment
        )
      };
      std::shared_ptr<DescriptorSetLayout> descriptorSetLayout =
        dev->createDescriptorSetLayout( dslbs );

      vk::PushConstantRange pushConstantRange( vk::ShaderStageFlagBits::eVertex,
        0, sizeof( pushConstants ) );

      _pipelineLayout = dev->createPipelineLayout( descriptorSetLayout, pushConstantRange );

      std::array<vk::DescriptorPoolSize, 2> poolSize =
      {
        // Binding 0: MVP uniform buffer
        vk::DescriptorPoolSize( vk::DescriptorType::eUniformBuffer, 1 ),
        // Binding 1: Texture
        vk::DescriptorPoolSize( vk::DescriptorType::eCombinedImageSampler, 1 )
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
          DescriptorImageInfo
          (
            vk::ImageLayout::eGeneral,
            std::make_shared<vk::ImageView>( tex->view ),
            std::make_shared<vk::Sampler>( tex->sampler )
          ), nullptr
        )
      };
      dev->updateDescriptorSets( wdss, {} );

      // init shaders
      std::shared_ptr<ShaderModule> vertexShaderModule =
        dev->createShaderModule( LAVA_EXAMPLES_SPV_ROUTE +
          std::string( "planar_reflection_vert.spv" ), vk::ShaderStageFlagBits::eVertex );
      std::shared_ptr<ShaderModule> fragmentShaderModule =
        dev->createShaderModule( LAVA_EXAMPLES_SPV_ROUTE +
          std::string( "planar_reflection_frag.spv" ), vk::ShaderStageFlagBits::eFragment );

      // init pipeline
      std::shared_ptr<PipelineCache> pipelineCache = dev->createPipelineCache( 0, nullptr );
      PipelineShaderStageCreateInfo vertexStage( vk::ShaderStageFlagBits::eVertex, vertexShaderModule );
      PipelineShaderStageCreateInfo fragmentStage( vk::ShaderStageFlagBits::eFragment, fragmentShaderModule );
      vk::VertexInputBindingDescription binding( 0, sizeof( Vertex ), vk::VertexInputRate::eVertex );

      PipelineVertexInputStateCreateInfo vertexInput( binding, {
        vk::VertexInputAttributeDescription( 0, 0, vk::Format::eR32G32B32Sfloat, offsetof( Vertex, pos ) ),
        vk::VertexInputAttributeDescription( 1, 0, vk::Format::eR32G32Sfloat, offsetof( Vertex, texCoord ) ) }
      );
      vk::PipelineInputAssemblyStateCreateInfo assembly( {}, vk::PrimitiveTopology::eTriangleList, VK_FALSE );
      PipelineViewportStateCreateInfo viewport( { {} }, { {} } );   // one dummy viewport and scissor, as dynamic state sets them
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
    }
    std::shared_ptr<Texture2D> tex;
    std::shared_ptr<Buffer> uniformBufferMVP;
  protected:
    std::shared_ptr<DescriptorSet> _descriptorSet;
  };


  class UVMaterialReflection : public lava::engine::Material
  {
  public:
    struct
    {
      glm::mat4 view;
      glm::mat4 proj;
    } uboVS;

    std::array<glm::mat4, 1> pushConstants; // Model matrix

    struct Vertex
    {
      glm::vec3 pos;
      glm::vec2 texCoord;
    };

    virtual void configure( const std::string& dir,
      std::shared_ptr< Device > dev,
      std::shared_ptr<RenderPass> renderPass )
    {
      // MVP buffer
      {
        uint32_t mvpBufferSize = sizeof( uboVS );
        uniformBufferMVP = dev->createBuffer( mvpBufferSize,
          vk::BufferUsageFlagBits::eUniformBuffer,
          vk::SharingMode::eExclusive, nullptr,
          vk::MemoryPropertyFlagBits::eHostVisible |
          vk::MemoryPropertyFlagBits::eHostCoherent );
      }

      // Init descriptor and pipeline layouts
      std::vector<DescriptorSetLayoutBinding> dslbs =
      {
        DescriptorSetLayoutBinding( 0,
        vk::DescriptorType::eUniformBuffer,
        vk::ShaderStageFlagBits::eVertex
        ),
        DescriptorSetLayoutBinding( 1,
          vk::DescriptorType::eCombinedImageSampler,
          vk::ShaderStageFlagBits::eFragment
        )
      };
      std::shared_ptr<DescriptorSetLayout> descriptorSetLayout =
        dev->createDescriptorSetLayout( dslbs );

      vk::PushConstantRange pushConstantRange( vk::ShaderStageFlagBits::eVertex,
        0, sizeof( pushConstants ) );

      _pipelineLayout = dev->createPipelineLayout( descriptorSetLayout, pushConstantRange );

      std::array<vk::DescriptorPoolSize, 2> poolSize =
      {
        // Binding 0: MVP uniform buffer
        vk::DescriptorPoolSize( vk::DescriptorType::eUniformBuffer, 1 ),
        // Binding 1: Texture
        vk::DescriptorPoolSize( vk::DescriptorType::eCombinedImageSampler, 1 )
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
          DescriptorImageInfo
          (
            vk::ImageLayout::eGeneral,
            std::make_shared<vk::ImageView>( tex->view ),
            std::make_shared<vk::Sampler>( tex->sampler )
          ), nullptr
        )
      };
      dev->updateDescriptorSets( wdss, {} );

      // init shaders
      std::shared_ptr<ShaderModule> vertexShaderModule =
        dev->createShaderModule( LAVA_EXAMPLES_SPV_ROUTE +
          std::string( "planar_reflection_vert.spv" ), vk::ShaderStageFlagBits::eVertex );
      std::shared_ptr<ShaderModule> fragmentShaderModule =
        dev->createShaderModule( LAVA_EXAMPLES_SPV_ROUTE +
          std::string( "planar_reflection_frag.spv" ), vk::ShaderStageFlagBits::eFragment );

      // init pipeline
      std::shared_ptr<PipelineCache> pipelineCache = dev->createPipelineCache( 0, nullptr );
      PipelineShaderStageCreateInfo vertexStage( vk::ShaderStageFlagBits::eVertex, vertexShaderModule );
      PipelineShaderStageCreateInfo fragmentStage( vk::ShaderStageFlagBits::eFragment, fragmentShaderModule );
      vk::VertexInputBindingDescription binding( 0, sizeof( Vertex ), vk::VertexInputRate::eVertex );

      PipelineVertexInputStateCreateInfo vertexInput( binding, {
        vk::VertexInputAttributeDescription( 0, 0, vk::Format::eR32G32B32Sfloat, offsetof( Vertex, pos ) ),
        vk::VertexInputAttributeDescription( 1, 0, vk::Format::eR32G32Sfloat, offsetof( Vertex, texCoord ) ) }
      );
      vk::PipelineInputAssemblyStateCreateInfo assembly( {}, vk::PrimitiveTopology::eTriangleList, VK_FALSE );
      PipelineViewportStateCreateInfo viewport( { {} }, { {} } );   // one dummy viewport and scissor, as dynamic state sets them
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
    }
    std::shared_ptr<Texture2D> tex;
    std::shared_ptr<Buffer> uniformBufferMVP;
  protected:
    std::shared_ptr<DescriptorSet> _descriptorSet;
  };
}

const std::vector<material::UVMaterial::Vertex> vertices =
{
  { { -0.5f, -0.5f, -0.5f },{ 0.0f, 0.0f } },
  { { 0.5f, -0.5f, -0.5f },{ 1.0f, 0.0f } },
  { { 0.5f,  0.5f, -0.5f },{ 1.0f, 1.0f } },
  { { 0.5f,  0.5f, -0.5f },{ 1.0f, 1.0f } },
  { { -0.5f,  0.5f, -0.5f },{ 0.0f, 1.0f } },
  { { -0.5f, -0.5f, -0.5f },{ 0.0f, 0.0f } },

  { { -0.5f, -0.5f,  0.5f },{ 0.0f, 0.0f } },
  { { 0.5f, -0.5f,  0.5f },{ 1.0f, 0.0f } },
  { { 0.5f,  0.5f,  0.5f },{ 1.0f, 1.0f } },
  { { 0.5f,  0.5f,  0.5f },{ 1.0f, 1.0f } },
  { { -0.5f,  0.5f,  0.5f },{ 0.0f, 1.0f } },
  { { -0.5f, -0.5f,  0.5f },{ 0.0f, 0.0f } },

  { { -0.5f,  0.5f,  0.5f },{ 1.0f, 0.0f } },
  { { -0.5f,  0.5f, -0.5f },{ 1.0f, 1.0f } },
  { { -0.5f, -0.5f, -0.5f },{ 0.0f, 1.0f } },
  { { -0.5f, -0.5f, -0.5f },{ 0.0f, 1.0f } },
  { { -0.5f, -0.5f,  0.5f },{ 0.0f, 0.0f } },
  { { -0.5f,  0.5f,  0.5f },{ 1.0f, 0.0f } },

  { { 0.5f,  0.5f,  0.5f },{ 1.0f, 0.0f } },
  { { 0.5f,  0.5f, -0.5f },{ 1.0f, 1.0f } },
  { { 0.5f, -0.5f, -0.5f },{ 0.0f, 1.0f } },
  { { 0.5f, -0.5f, -0.5f },{ 0.0f, 1.0f } },
  { { 0.5f, -0.5f,  0.5f },{ 0.0f, 0.0f } },
  { { 0.5f,  0.5f,  0.5f },{ 1.0f, 0.0f } },

  { { -0.5f, -0.5f, -0.5f },{ 0.0f, 1.0f } },
  { { 0.5f, -0.5f, -0.5f },{ 1.0f, 1.0f } },
  { { 0.5f, -0.5f,  0.5f },{ 1.0f, 0.0f } },
  { { 0.5f, -0.5f,  0.5f },{ 1.0f, 0.0f } },
  { { -0.5f, -0.5f,  0.5f },{ 0.0f, 0.0f } },
  { { -0.5f, -0.5f, -0.5f },{ 0.0f, 1.0f } },

  { { -0.5f,  0.5f, -0.5f },{ 0.0f, 1.0f } },
  { { 0.5f,  0.5f, -0.5f },{ 1.0f, 1.0f } },
  { { 0.5f,  0.5f,  0.5f },{ 1.0f, 0.0f } },
  { { 0.5f,  0.5f,  0.5f },{ 1.0f, 0.0f } },
  { { -0.5f,  0.5f,  0.5f },{ 0.0f, 0.0f } },
  { { -0.5f,  0.5f, -0.5f },{ 0.0f, 1.0f } },

  { { -1.0f, -1.0f, -0.5f },{ 0.0f, 0.0f } },
  { { 1.0f, -1.0f, -0.5f },{ 1.0f, 0.0f } },
  { { 1.0f,  1.0f, -0.5f },{ 1.0f, 1.0f } },
  { { 1.0f,  1.0f, -0.5f },{ 1.0f, 1.0f } },
  { { -1.0f,  1.0f, -0.5f },{ 0.0f, 1.0f } },
  { { -1.0f, -1.0f, -0.5f },{ 0.0f, 0.0f } }
};

class MyApp : public VulkanApp
{
public:
  std::shared_ptr<VertexBuffer> _vertexBuffer;
  std::shared_ptr<material::UVMaterial> material;

  MyApp( char const* title, uint32_t width, uint32_t height )
    : VulkanApp( title, width, height )
  {
    // Vertex buffer
    {
      uint32_t vertexBufferSize = vertices.size( ) * sizeof( material::UVMaterial::Vertex );
      _vertexBuffer = std::make_shared<VertexBuffer>( _device, vertexBufferSize );
      _vertexBuffer->writeData( 0, vertexBufferSize, vertices.data( ) );
    }

    material = std::make_shared<material::UVMaterial>( );

    std::shared_ptr<CommandPool> commandPool = _device->createCommandPool(
      vk::CommandPoolCreateFlagBits::eResetCommandBuffer, _queueFamilyIndex );
    material->tex = std::make_shared<Texture2D>( _device, LAVA_EXAMPLES_IMAGES_ROUTE + 
      std::string( "random.png" ), commandPool, _graphicsQueue,
      vk::Format::eR8G8B8A8Unorm );

    material->configure( LAVA_EXAMPLES_SPV_ROUTE, _device, _renderPass );

    material->uboVS.view = glm::lookAt(
      glm::vec3( 2.5f, 2.5f, 2.0f ),
      glm::vec3( 0.0f, 0.0f, 0.0f ),
      glm::vec3( 0.0f, 0.0f, 1.0f )
    );
    material->uboVS.proj = glm::perspective( glm::radians( 45.0f ), width / ( float ) height, 0.1f, 10.0f );
    material->uboVS.proj[ 1 ][ 1 ] *= -1;

    material->uniformBufferMVP->writeData( 0, sizeof( material->uboVS ), &material->uboVS );
  }

  void doPaint( void ) override
  {
    static auto startTime = std::chrono::high_resolution_clock::now( );

    auto currentTime = std::chrono::high_resolution_clock::now( );
    float time = std::chrono::duration_cast<std::chrono::milliseconds>( currentTime - startTime ).count( ) / 1000.0f;

    // create a command pool for command buffer allocation
    std::shared_ptr<CommandPool> commandPool = 
      _device->createCommandPool( 
          vk::CommandPoolCreateFlagBits::eResetCommandBuffer, _queueFamilyIndex );
    std::shared_ptr<CommandBuffer> commandBuffer = commandPool->allocateCommandBuffer( );

    commandBuffer->begin( );

    std::array<float, 4> ccv = { 0.2f, 0.3f, 0.3f, 1.0f };
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
    
    vk::PipelineLayout pipl = *material->pipelineLayout( );

    // Draw cube
    material->pushConstants[ 0 ] = glm::rotate( glm::mat4( 1.0f ), time * glm::radians( 180.0f ), glm::vec3( 0.0f, 0.0f, 1.0f ) );
    commandBuffer->pushConstants<glm::mat4>( pipl, vk::ShaderStageFlagBits::eVertex, 0, material->pushConstants );
    commandBuffer->draw( 36, 1, 0, 0 );

    // Draw floor
    commandBuffer->draw( 6, 1, 36, 0 );

    // Draw cube reflection
    material->pushConstants[ 0 ] = glm::scale(
      glm::translate( material->pushConstants[ 0 ], glm::vec3( 0.0f, 0.0f, -1.0f ) ),
      glm::vec3( 1.0f, 1.0f, -1.0f )
    );
    commandBuffer->pushConstants<glm::mat4>( pipl, vk::ShaderStageFlagBits::eVertex, 0, material->pushConstants );
    commandBuffer->draw( 36, 1, 0, 0 );
    commandBuffer->endRenderPass( );

    commandBuffer->end( );

    _graphicsQueue->submit( SubmitInfo{
      { _defaultFramebuffer->getPresentSemaphore( ) },
      { vk::PipelineStageFlagBits::eColorAttachmentOutput },
      commandBuffer,
      _renderComplete
    } );
  }
  void keyEvent( int key, int scancode, int action, int mods )
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
    VulkanApp* app = new MyApp( "Planar Reflection", 800, 600 );

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