#include <lava/lava.h>
using namespace lava;

#include <routes.h>

class MyApp : public VulkanApp
{
public:
  struct
  {
    std::shared_ptr<lava::Buffer> buffer;
    uint32_t bufferSize;
    uint32_t numElems;
    std::shared_ptr<lava::Queue> queue;
    std::shared_ptr<lava::CommandPool> commandPool;
    std::shared_ptr<lava::CommandBuffer> commandBuffer;
    std::shared_ptr<lava::Fence> fence;
    std::shared_ptr<lava::Pipeline> pipeline;
    std::shared_ptr<lava::DescriptorSetLayout> descriptorSetLayout;
    std::shared_ptr<lava::PipelineLayout> pipelineLayout;
    std::shared_ptr<lava::DescriptorSet> descriptorSet;
  } compute;

  MyApp(char const* title, uint32_t width, uint32_t height)
    : VulkanApp( title, width, height )
  {
    // Search for a compute queue in the array of 
    //    queue families, try to find one that support
    std::vector<uint32_t> queueFamilyIndices =
      _physicalDevice->getComputeQueueFamilyIndices( _surface );
    assert( !queueFamilyIndices.empty( ) );
    uint32_t _queueComputeFamilyIndex = queueFamilyIndices[ 0 ];

    compute.queue = _device->getQueue( _queueComputeFamilyIndex, 0 );

    std::cout << "Compute queue created" << std::endl;

    compute.numElems = 10;

    std::vector<uint32_t> indices;
    for ( uint32_t i = 0; i < compute.numElems; ++i )
    {
      indices.push_back( 666 );
    }

    compute.bufferSize = indices.size( ) * sizeof( indices.front( ) );
    compute.buffer = _device->createBuffer( compute.bufferSize,
      vk::BufferUsageFlagBits::eStorageBuffer, vk::SharingMode::eExclusive,
      nullptr, vk::MemoryPropertyFlagBits::eHostVisible | 
      vk::MemoryPropertyFlagBits::eHostCoherent
    );

    compute.buffer->writeData( 0, compute.bufferSize, indices.data( ) );

    void *dst = new void*[ compute.bufferSize ];
    compute.buffer->readData( 0, compute.bufferSize, dst );
    uint32_t* dst2 = ( uint32_t* ) dst;
    std::vector<uint32_t> v( dst2, dst2 + compute.numElems );

    std::cout << "Buffer data readed" << std::endl;

    std::vector<DescriptorSetLayoutBinding> setLayoutBindings = 
    {
      DescriptorSetLayoutBinding( 
        0,
        vk::DescriptorType::eStorageBuffer, 
        vk::ShaderStageFlagBits::eCompute
      )
    };
    
    compute.descriptorSetLayout = _device->createDescriptorSetLayout( setLayoutBindings );

    compute.pipelineLayout = _device->createPipelineLayout( compute.descriptorSetLayout );

    std::shared_ptr<DescriptorPool> descriptorPool =
      _device->createDescriptorPool( {}, 1,
      { 
        { vk::DescriptorType::eStorageBuffer, 1 }
      }
    );

    // Init descriptor set
    compute.descriptorSet = _device->allocateDescriptorSet( descriptorPool, 
      compute.descriptorSetLayout );
    std::vector<WriteDescriptorSet> wdss =
    {
      WriteDescriptorSet(
        compute.descriptorSet, 0, 0, vk::DescriptorType::eStorageBuffer,
        1, nullptr,
        DescriptorBufferInfo(
          compute.buffer, 0, compute.bufferSize
        )
      )
    };
    _device->updateDescriptorSets( wdss, {} );
  
    // init pipeline
    std::shared_ptr<PipelineCache> pipelineCache = 
      _device->createPipelineCache( 0, nullptr );

    std::shared_ptr<ShaderModule> computeShaderModule =
      _device->createShaderModule(
        LAVA_EXAMPLES_SPV_ROUTE + std::string( "example_compute_comp.spv" ),
        vk::ShaderStageFlagBits::eCompute
      );

    PipelineShaderStageCreateInfo computeStage(
      vk::ShaderStageFlagBits::eCompute, computeShaderModule );

    std::cout << "CREATE PIPELINE" << std::endl;

    compute.pipeline = _device->createComputePipeline(
      pipelineCache, {}, computeStage, compute.pipelineLayout );

    std::cout << "PIPELINE CREATED" << std::endl;

    compute.fence = _device->createFence( true );

    compute.commandPool = _device->createCommandPool( {}, _queueComputeFamilyIndex );

    compute.commandBuffer = compute.commandPool->allocateCommandBuffer( );

    compute.commandBuffer->beginSimple( );

    // TODO: add pipeline barrier if use compute data from graphics rendering

    compute.commandBuffer->bindComputePipeline( compute.pipeline );
    compute.commandBuffer->bindDescriptorSets( 
      vk::PipelineBindPoint::eCompute, compute.pipelineLayout, 0, 
      compute.descriptorSet, { }
    );
    compute.commandBuffer->dispatch( 1, 1, 1 );

    vk::BufferMemoryBarrier bufferBarrier(
      vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eShaderRead,
      compute.queue->getQueueFamilyIndex(),
      compute.queue->getQueueFamilyIndex(), *compute.buffer, 0, compute.bufferSize
    );

    compute.commandBuffer->pipelineBarrier( vk::PipelineStageFlagBits::eComputeShader, vk::PipelineStageFlagBits::eVertexShader, {}, {}, { bufferBarrier }, {} );

    compute.commandBuffer->end( );
  }
  void keyEvent(int key, int scancode, int action, int mods)
  {
    switch (key)
    {
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

  void doPaint( void ) override
  {
    // Submit compute commands
    lava::Fence::waitForFences( { compute.fence }, true, UINT64_MAX );
    lava::Fence::resetFences( { compute.fence } );

    compute.queue->submit( compute.commandBuffer, compute.fence );

    std::vector<uint32_t> v( compute.numElems );
    compute.buffer->readData( 0, compute.bufferSize, v.data( ) );

    std::shared_ptr<CommandPool> commandPool = _device->createCommandPool(
      vk::CommandPoolCreateFlagBits::eResetCommandBuffer, _queueFamilyIndex );
    std::shared_ptr<CommandBuffer> commandBuffer = commandPool->allocateCommandBuffer( );

    std::array<float, 4> ccv = { 1.0f, 1.0f, 1.0f, 1.0f };
    commandBuffer->begin( );
    commandBuffer->beginRenderPass( _renderPass, _defaultFramebuffer->getFramebuffer( ),
      vk::Rect2D( { 0, 0 }, _defaultFramebuffer->getExtent( ) ), { vk::ClearValue( ccv ),
      vk::ClearValue( vk::ClearDepthStencilValue( 1.0f, 0 ) ) },
      vk::SubpassContents::eInline );
    commandBuffer->setViewportScissors( _defaultFramebuffer->getExtent( ) );
    commandBuffer->endRenderPass( );
    commandBuffer->end( );

    _graphicsQueue->submit( SubmitInfo{
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
    VulkanApp* app = new MyApp( "MyApp", 800, 600 );

    app->getWindow( )->setErrorCallback( glfwErrorCallback );

    while ( app->isRunning( ) )
    {
      //app->waitEvents( );
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