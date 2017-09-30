#include <lava/lava.h>
using namespace lava;

#include <routes.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <random>

#define PARTICLE_COUNT 256 * 1024

class MyApp : public VulkanApp
{
  // SSBO particle declaration
  struct Particle
  {
    glm::vec2 pos;                // Particle position
    glm::vec2 vel;                // Particle velocity
  };
  struct
  {
    std::shared_ptr<Buffer> storageBuffer;
    std::shared_ptr<Buffer> uniformBuffer;
    struct computeUBO
    {
      float deltaT;
      float destX;
      float destY;
      int32_t particleCount = PARTICLE_COUNT;
    } ubo;
    VkDeviceSize storageBufferSize;
  } compute;
public:
  std::shared_ptr<Pipeline> _pipeline;
  std::shared_ptr<PipelineLayout> _pipelineLayout;
  std::shared_ptr<DescriptorSet> _descriptorSet;

  void updateUniformBuffers( void )
  {
    float frameTimer = 1.0f, timer = 1.0f;
    compute.ubo.deltaT = frameTimer * 2.5f;
    compute.ubo.destX = sin(glm::radians(timer * 360.0f)) * 0.75f;
    compute.ubo.destY = 0.0f;
    
    compute.uniformBuffer->writeData( 0, sizeof( compute.ubo ), &compute.ubo );
  }

  MyApp(char const* title, uint32_t width, uint32_t height)
    : VulkanApp( title, width, height )
  {
    // PREPARE STORAGE BUFFERS
    std::mt19937 rGenerator;
    std::uniform_real_distribution<float> rDistribution(-1.0f, 1.0f);

    // Initial particle positions
    std::vector<Particle> particleBuffer(PARTICLE_COUNT);
    for (auto& particle : particleBuffer)
    {
      particle.pos = glm::vec2(rDistribution(rGenerator), rDistribution(rGenerator));
      particle.vel = glm::vec2(0.0f);
    }

    compute.storageBufferSize = particleBuffer.size() * sizeof(Particle);

    // PREPARE UNIFORM BUFFERS
    compute.uniformBuffer = _device->createBuffer( sizeof( compute.ubo ),
      vk::BufferUsageFlagBits::eUniformBuffer, vk::SharingMode::eExclusive, 
      nullptr, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent );

    updateUniformBuffers( );

    // Compute buffer
    {
      compute.storageBuffer = _device->createBuffer( compute.storageBufferSize, 
        vk::BufferUsageFlagBits::eStorageBuffer, vk::SharingMode::eExclusive, 
        nullptr, vk::MemoryPropertyFlagBits::eHostCoherent | 
          vk::MemoryPropertyFlagBits::eHostVisible );
    }

    // PREPARE COMPUTE
    // todo: i'm using graphics queue for compute work ...
    std::vector<DescriptorSetLayoutBinding> dslbs;
    dslbs.push_back( DescriptorSetLayoutBinding
      ( 0, vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eCompute )
    );
    dslbs.push_back( DescriptorSetLayoutBinding
      ( 1, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eCompute )
    );
    std::shared_ptr<DescriptorSetLayout> descriptorSetLayout = _device->createDescriptorSetLayout( dslbs );
    _pipelineLayout = _device->createPipelineLayout( descriptorSetLayout, nullptr );

    std::array<vk::DescriptorPoolSize, 3> poolSize;
    poolSize[ 0 ] = vk::DescriptorPoolSize( vk::DescriptorType::eUniformBuffer, 1 );
    poolSize[ 1 ] = vk::DescriptorPoolSize( vk::DescriptorType::eStorageBuffer, 1 );
    poolSize[ 2 ] = vk::DescriptorPoolSize( vk::DescriptorType::eCombinedImageSampler, 2 );
    std::shared_ptr<DescriptorPool> descriptorPool = 
      _device->createDescriptorPool( {}, 2, poolSize );


    // Init descriptor set
    _descriptorSet = _device->allocateDescriptorSet( descriptorPool, descriptorSetLayout );
    std::vector<WriteDescriptorSet> wdss;
    wdss.push_back( WriteDescriptorSet (
        _descriptorSet, 0, 0, 
        vk::DescriptorType::eStorageBuffer, 1, nullptr, 
        lava::DescriptorBufferInfo( compute.storageBuffer, 0, compute.storageBufferSize )
      )
    );
    wdss.push_back( WriteDescriptorSet (
        _descriptorSet, 1, 0, 
        vk::DescriptorType::eUniformBuffer, 1, nullptr, 
        lava::DescriptorBufferInfo( compute.uniformBuffer, 0, sizeof( compute.ubo ) )
      )
    );
    _device->updateDescriptorSets( wdss, {} );

    // init pipeline
    std::shared_ptr<PipelineCache> pipelineCache = _device->createPipelineCache( 0, nullptr );

    std::shared_ptr<ShaderModule> computeShaderModule = 
    _device->createShaderModule( LAVA_EXAMPLES_RESOURCES_ROUTE + 
      std::string("/compute_example.spv"), vk::ShaderStageFlagBits::eCompute );

    PipelineShaderStageCreateInfo computeStage( 
      vk::ShaderStageFlagBits::eCompute, computeShaderModule );

    std::cout << "CREATE PIPELINE" << std::endl;

    _pipeline = _device->createComputePipeline( 
      pipelineCache, {}, computeStage, _pipelineLayout );

    std::cout << "PIPELINE CREATED" << std::endl;
  }
  void keyEvent(int key, int scancode, int action, int mods)
  {
    switch (key)
    {
    case GLFW_KEY_ESCAPE:
      switch (action)
      {
      case GLFW_PRESS:
        glfwSetWindowShouldClose(getWindow()->getWindow( ), GLFW_TRUE);
        break;
      default:
        break;
      }
      break;
    default:
      break;
    }
  }
  void doPaint( void ) override
  {
    uint32_t width = _window->getWidth( );
    uint32_t height = _window->getHeight( );

    // create a command pool for command buffer allocation
    std::shared_ptr<CommandPool> commandPool = _device->createCommandPool( 
      vk::CommandPoolCreateFlagBits::eResetCommandBuffer, _queueFamilyIndex );
    std::shared_ptr<CommandBuffer> commandBuffer = commandPool->allocateCommandBuffer( );

    commandBuffer->begin( );

    commandBuffer->bindComputePipeline( _pipeline );
    commandBuffer->bindDescriptorSets( vk::PipelineBindPoint::eCompute,
      _pipelineLayout, 0, { _descriptorSet }, nullptr );

    commandBuffer->dispatch( PARTICLE_COUNT / 256, 1, 1 );

    commandBuffer->end( );

    _graphicsQueue->submit( SubmitInfo {
      { _defaultFramebuffer->getPresentSemaphore( ) },
      { vk::PipelineStageFlagBits::eColorAttachmentOutput },
      commandBuffer,
      _renderComplete
    } );
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
    //if (glfwInit())
    //{
    VulkanApp* app = new MyApp( "Compute Example", 100, 100 );

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