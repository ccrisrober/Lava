#include <lava/lava.h>
using namespace lava;

#include <routes.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>

#include <routes.h>

struct Particle
{
  glm::vec4 pos;
  glm::vec4 color;
  float alpha;
  float size;
}

struct UniformBufferObject
{
  glm::mat4 model;
  glm::mat4 view;
  glm::mat4 proj;
};


#define PARTICLE_COUNT 512
#define PARTICLE_SIZE 10.0f

#define FLAME_RADIUS 8.0f

class MyApp : public VulkanApp
{
public:
  std::shared_ptr<PipelineLayout> _pipelineLayout;
  std::shared_ptr<Texture2D> tex;

  struct
  {
    struct
    {
      std::shared_ptr<Texture2D> fire;
    } particles;
  } textures;


  glm::vec3 emitterPos = glm::vec3(0.0f, -FLAME_RADIUS + 2.0f, 0.0f);
  glm::vec3 minVel = glm::vec3(-3.0f, 0.5f, -3.0f);
  glm::vec3 maxVel = glm::vec3(3.0f, 7.0f, 3.0f);

  struct
  {
    std::shared_ptr<Buffer> buffer;
    size_t size;
  } particles;

  struct
  {
    std::shared_ptr<Pipeline> particles;
  } pipelines;

  struct
  {
    std::shared_ptr<DescriptorSet> particles;
  } descriptorSets;

  std::vector<Particle> particleBuffer;

  float rnd(float range)
  {
    return range * (rand() / float(RAND_MAX));
  }

  void initParticle(Particle *particle, glm::vec3 emitterPos)
  {
    particle->vel = glm::vec4(0.0f, minVel.y + rnd(maxVel.y - minVel.y), 0.0f, 0.0f);
    particle->alpha = rnd(0.75f);
    particle->size = 1.0f + rnd(0.5f);
    particle->color = glm::vec4(1.0f);

    // Get random sphere point
    float theta = rnd(2.0f * float(M_PI));
    float phi = rnd(float(M_PI)) - float(M_PI) / 2.0f;
    float r = rnd(FLAME_RADIUS);

    particle->pos.x = r * cos(theta) * cos(phi);
    particle->pos.y = r * sin(phi);
    particle->pos.z = r * sin(theta) * cos(phi);

    particle->pos += glm::vec4(emitterPos, 0.0f);
  }

  MyApp( char const* title, uint32_t width, uint32_t height )
    : VulkanApp( title, width, height )
  {
    srand( time( nullptr ) );

    // LOADING ASSETS
    {
      std::shared_ptr<CommandPool> commandPool = _device->createCommandPool(
        vk::CommandPoolCreateFlagBits::eResetCommandBuffer, _queueFamilyIndex );

      textures.particles.fire = std::make_shared<Texture2D>( _device, LAVA_EXAMPLES_RESOURCES_ROUTE + 
        std::string( "/random.png" ), commandPool, _graphicsQueue );
    }
    // PREPARING PARTICLES
    {
      particleBuffer.resize( PARTICLE_COUNT );
      for( auto& particle: particleBuffer )
      {
        initParticle( &particle, emitterPos );
        particle.alpha = 1.0f - (abs(particle.pos.y) / (FLAME_RADIUS * 2.0f));
      }

      particles.size = particleBuffer.size( ) * sizeof( Particle );

      particleBuffer = _device->createBuffer( particles.size, 
        vk::BufferUsageFlagBits::eUniformBuffer, 
        vk::SharingMode::eExclusive, nullptr,
        vk::MemoryPropertyFlagBits::eHostVisible | 
          vk::MemoryPropertyFlagBits::eHostCoherent );
    }

    // PREPARING UNIFORMS
    // MVP buffer
    {
      uint32_t mvpBufferSize = sizeof(UniformBufferObject);
      _uniformBufferMVP = _device->createBuffer( mvpBufferSize, 
        vk::BufferUsageFlagBits::eUniformBuffer, 
        vk::SharingMode::eExclusive, nullptr,
        vk::MemoryPropertyFlagBits::eHostVisible | 
          vk::MemoryPropertyFlagBits::eHostCoherent );

      updateUniformBuffers( );
    }

    // SETUP DESCRIPTOR SET LAYOUT
    {
      // Init descriptor and pipeline layouts
      std::vector<DescriptorSetLayoutBinding> dslbs;
      dslbs.push_back( DescriptorSetLayoutBinding( 0, 
        vk::DescriptorType::eUniformBuffer, 
        vk::ShaderStageFlagBits::eVertex ) );
      dslbs.push_back( DescriptorSetLayoutBinding( 1, 
        vk::DescriptorType::eCombinedImageSampler, 
        vk::ShaderStageFlagBits::eFragment ) );
      std::shared_ptr<DescriptorSetLayout> descriptorSetLayout = 
        _device->createDescriptorSetLayout( dslbs );

      _pipelineLayout = _device->createPipelineLayout( 
        descriptorSetLayout, nullptr );
    }

    // PREPARING PIPELINES
    {
      vk::PipelineInputAssemblyStateCreateInfo inputAssemblyState( {}, 
        vk::PrimitiveTopology::ePointList, VK_FALSE
      );
      vk::PipelineRasterizationStateCreateInfo rasterizationState( {}, true,
        false, vk::PolygonMode::eFill, vk::CullModeFlagBits::eBack,
        vk::FrontFace::eCounterClockwise, false, 0.0f, 0.0f, 0.0f, 1.0f
      );
      vk::PipelineColorBlendAttachmentState blendAttachmentState( false, 
        vk::BlendFactor::eZero, vk::BlendFactor::eZero, vk::BlendOp::eAdd, 
        vk::BlendFactor::eZero, vk::BlendFactor::eZero, vk::BlendOp::eAdd,
        vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | 
        vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA
      );
    
      PipelineColorBlendStateCreateInfo colorBlendState( false, 
        vk::LogicOp::eNoOp, blendAttachmentState, { 1.0f, 1.0f, 1.0f, 1.0f } 
      );
    
      vk::StencilOpState stencilOpState( vk::StencilOp::eKeep, 
        vk::StencilOp::eKeep, vk::StencilOp::eKeep, 
        vk::CompareOp::eAlways, 0, 0, 0 
      );
      vk::PipelineDepthStencilStateCreateInfo depthStencilState( 
        { }, true, true, vk::CompareOp::eLessOrEqual, false, 
        false, stencilOpState, stencilOpState, 0.0f, 0.0f
      );
    
      PipelineViewportStateCreateInfo viewportState( { {} }, { {} } );

      PipelineMultisampleStateCreateInfo multisampleState( 
        vk::SampleCountFlagBits::e1, false, 0.0f, nullptr, false, false
      );

      PipelineDynamicStateCreateInfo dynamicState( 
        { vk::DynamicState::eViewport, vk::DynamicState::eScissor } );

      // init shaders
      std::shared_ptr<ShaderModule> vertexShaderModule =
        _device->createShaderModule( LAVA_EXAMPLES_RESOURCES_ROUTE +
          std::string( "/cubeUV_vert.spv" ), 
          vk::ShaderStageFlagBits::eVertex
      );
      std::shared_ptr<ShaderModule> fragmentShaderModule =
        _device->createShaderModule( LAVA_EXAMPLES_RESOURCES_ROUTE +
          std::string( "/cubeUV_frag.spv" ), 
          vk::ShaderStageFlagBits::eFragment
      );


      // init pipeline
      std::shared_ptr<PipelineCache> pipelineCache = 
        _device->createPipelineCache( 0, nullptr );
      PipelineShaderStageCreateInfo vertexStage( 
        vk::ShaderStageFlagBits::eVertex, vertexShaderModule
      );
      PipelineShaderStageCreateInfo fragmentStage( 
        vk::ShaderStageFlagBits::eFragment, fragmentShaderModule
      );
      vk::VertexInputBindingDescription binding( 0, sizeof( Particle ), 
        vk::VertexInputRate::eVertex
      );

      PipelineVertexInputStateCreateInfo vertexInputState( binding,
        {
          vk::VertexInputAttributeDescription( 0, 0, 
            vk::Format::eR32G32B32Sfloat, offsetof( Particle, pos )
          ),
          vk::VertexInputAttributeDescription( 1, 0, 
            vk::Format::eR32G32Sfloat, offsetof( Particle, color )
          ),
          vk::VertexInputAttributeDescription( 2, 0, 
            vk::Format::eR32float, offsetof( Particle, alpha )
          ),
          vk::VertexInputAttributeDescription( 3, 0, 
            vk::Format::eR32float, offsetof( Particle, size )
          )
        }
      );

      // Dont' write to depth buffer
      depthStencilState.depthWriteEnable = VK_FALSE;

      // Premulitplied alpha
      blendAttachmentState.blendEnable = VK_TRUE;
      blendAttachmentState.srcColorBlendFactor = vk::BlendFactor::eOne;
      blendAttachmentState.dstColorBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha;
      blendAttachmentState.colorBlendOp = vk::BlendOp::eAdd;
      blendAttachmentState.srcAlphaBlendFactor = vk::BlendFactor::eOne;
      blendAttachmentState.dstAlphaBlendFactor = vk::BlendFactor::eZero;
      blendAttachmentState.alphaBlendOp = vk::BlendOp::eAdd;
      blendAttachmentState.colorWriteMask = 
      vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | 
      vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;


      pipelines.particles = _device->createGraphicsPipeline( pipelineCache, {}, 
        { vertexStage, fragmentStage }, vertexInputState, inputAssemblyState, nullptr, viewportState, rasterizationState, multisampleState, depthStencilState, colorBlendState, dynamicState,
        _pipelineLayout, _renderPass );
    }


    


    std::array<vk::DescriptorPoolSize, 2> poolSize;
    poolSize[ 0 ] = vk::DescriptorPoolSize( vk::DescriptorType::eUniformBuffer, 1 );
    poolSize[ 1 ] = vk::DescriptorPoolSize( vk::DescriptorType::eCombinedImageSampler, 1 );
    std::shared_ptr<DescriptorPool> descriptorPool = _device->createDescriptorPool( {}, 1, poolSize );

    // Init descriptor set
    _descriptorSet = _device->allocateDescriptorSet( descriptorPool, descriptorSetLayout );
    std::vector<WriteDescriptorSet> wdss;

    WriteDescriptorSet w( _descriptorSet, 0, 0, vk::DescriptorType::eUniformBuffer, 1, nullptr, 
      DescriptorBufferInfo( _uniformBufferMVP, 0, sizeof( UniformBufferObject ) ) );
    wdss.push_back( w );

    WriteDescriptorSet w2( _descriptorSet, 1, 0, vk::DescriptorType::eCombinedImageSampler, 1, 
      DescriptorImageInfo( 
        vk::ImageLayout::eGeneral, 
        std::make_shared<vk::ImageView>( tex->view ), 
        std::make_shared<vk::Sampler>( tex->sampler )
      ), nullptr
    );
    wdss.push_back( w2 );
    _device->updateDescriptorSets( wdss, {} );
  }
  void updateUniformBuffers( )
  {
    uint32_t width = _window->getWidth( );
    uint32_t height = _window->getHeight( );

    static auto startTime = std::chrono::high_resolution_clock::now();

    auto currentTime = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - startTime).count() / 1000.0f;

    UniformBufferObject ubo = {};
    ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    ubo.proj = glm::perspective(glm::radians(45.0f), width / (float) height, 0.1f, 10.0f);
    ubo.proj[1][1] *= -1;

    vk::Device device = static_cast<vk::Device>(*_device);

    uint32_t mvpBufferSize = sizeof(UniformBufferObject);
    void* data = _uniformBufferMVP->map( 0, mvpBufferSize );
    memcpy( data, &ubo, sizeof(ubo) );
    _uniformBufferMVP->unmap( );

    //std::cout<<glm::to_string(mvpc)<<std::endl;
  }

  void doPaint( ) override
  {
    updateUniformBuffers( );

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
    commandBuffer->bindGraphicsPipeline( _pipeline );
    commandBuffer->bindDescriptorSets( vk::PipelineBindPoint::eGraphics,
      _pipelineLayout, 0, { _descriptorSet }, nullptr );
    _vertexBuffer->bind( commandBuffer );
    _indexBuffer->bind( commandBuffer );
    commandBuffer->setViewport( 0, vk::Viewport( 0.0f, 0.0f, ( float ) _defaultFramebuffer->getExtent( ).width, ( float ) _defaultFramebuffer->getExtent( ).height, 0.0f, 1.0f ) );
    commandBuffer->setScissor( 0, vk::Rect2D( { 0, 0 }, _defaultFramebuffer->getExtent( ) ) );
    commandBuffer->drawIndexed( indices.size( ), 1, 0, 0, 1 );
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
        glfwSetWindowShouldClose( getWindow( )->getWindow( ), GLFW_TRUE );
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
    //if (glfwInit())
    //{
    VulkanApp* app = new MyApp( "Fire Particles", 500, 500 );

    app->getWindow( )->setErrorCallback( glfwErrorCallback );

    while ( app->isRunning( ) )
    {
      app->waitEvents( );
      app->paint( );
    }

    delete app;
    //}
  }
  catch ( std::system_error err )
  {
    std::cout << "System Error: " << err.what( ) << std::endl;
  }
  system( "PAUSE" );
  return 0;
}