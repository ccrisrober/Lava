#include <lava/lava.h>
using namespace lava;

#include <routes.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "lodepng.h" //Used for png encoding.

class MyApp : public VulkanApp
{
  struct Pixel {
    float r, g, b, a;
  };
public:
  std::shared_ptr<Pipeline> _pipeline;
  std::shared_ptr<PipelineLayout> _pipelineLayout;
  std::shared_ptr<DescriptorSet> _descriptorSet;

  std::shared_ptr<Buffer> _computeBuffer;

  uint32_t bufferSize = sizeof( Pixel ) * 100 * 100;

  MyApp(char const* title, uint32_t width, uint32_t height)
    : VulkanApp( title, width, height )
  {
    // Compute buffer
    {
      _computeBuffer = _device->createBuffer( bufferSize, 
        vk::BufferUsageFlagBits::eStorageBuffer, vk::SharingMode::eExclusive, 
        nullptr, vk::MemoryPropertyFlagBits::eHostCoherent | 
          vk::MemoryPropertyFlagBits::eHostVisible );
      std::vector<Pixel> values;// ( 100 * 100 );
      uint32_t numPixels = 100 * 100;
      for ( uint32_t i = 0; i < numPixels; ++i )
      {
        values.push_back( Pixel{ i * 1.0f, i * 1.0f, i * 1.0f, 1.0f } );
      }
      _computeBuffer->writeData( 0, bufferSize, values.data( ) );
    }

    // init descriptor and pipeline layouts
    std::vector<DescriptorSetLayoutBinding> dslbs;
    DescriptorSetLayoutBinding mvpDescriptor( 0, vk::DescriptorType::eStorageBuffer,
      vk::ShaderStageFlagBits::eCompute );
    dslbs.push_back( mvpDescriptor );
    std::shared_ptr<DescriptorSetLayout> descriptorSetLayout = _device->createDescriptorSetLayout( dslbs );
    _pipelineLayout = _device->createPipelineLayout( descriptorSetLayout, nullptr );

    std::array<vk::DescriptorPoolSize, 1> poolSize;
    poolSize[ 0 ] = vk::DescriptorPoolSize( vk::DescriptorType::eStorageBuffer, 1 );
    std::shared_ptr<DescriptorPool> descriptorPool = 
      _device->createDescriptorPool( {}, 1, poolSize );

    // Init descriptor set
    _descriptorSet = _device->allocateDescriptorSet( descriptorPool, descriptorSetLayout );
    std::vector<WriteDescriptorSet> wdss;
    WriteDescriptorSet w( _descriptorSet, 0, 0, 
      vk::DescriptorType::eStorageBuffer, 1, nullptr, 
      lava::DescriptorBufferInfo( _computeBuffer, 0, bufferSize ) );
    wdss.push_back( w );
    _device->updateDescriptorSets( wdss, {} );



    // init pipeline
    std::shared_ptr<PipelineCache> pipelineCache = _device->createPipelineCache( 0, nullptr );

    std::shared_ptr<ShaderModule> computeShaderModule = 
    _device->createShaderModule( LAVA_EXAMPLES_RESOURCES_ROUTE + 
      std::string("/compute_example.spv"), vk::ShaderStageFlagBits::eCompute );

    PipelineShaderStageCreateInfo computeStage( 
      vk::ShaderStageFlagBits::eCompute, computeShaderModule, "main" );

    std::cout << "CREATE PIPELINE" << std::endl;

    _pipeline = _device->createComputePipeline( pipelineCache, {}, computeStage, _pipelineLayout );

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
  /*
  The mandelbrot set will be rendered to this buffer.
  The memory that backs the buffer is bufferMemory. 
  */
  vk::Buffer buffer;
  vk::DeviceMemory bufferMemory;
  void saveRenderedImage( void )
  {
    uint32_t width = _window->getWidth( );
    uint32_t height = _window->getHeight( );

    void* mappedMemory = nullptr;
    // Map the buffer memory, so that we can read from it on the CPU.
    mappedMemory = static_cast< vk::Device > (*_device).mapMemory( bufferMemory, 0, bufferSize, { } );
    Pixel* pmappedMemory = (Pixel *)mappedMemory;

    // Get the color data from the buffer, and cast it to bytes.
    // We save the data to a vector.
    std::vector<unsigned char> image;
    image.reserve(width * height * 4);
    for ( uint32_t i = 0, size = width * height; i < size; ++i )
    {
        image.push_back((unsigned char)(255.0f * (pmappedMemory[ i ].r)));
        image.push_back((unsigned char)(255.0f * (pmappedMemory[ i ].g)));
        image.push_back((unsigned char)(255.0f * (pmappedMemory[ i ].b)));
        image.push_back((unsigned char)(255.0f * (pmappedMemory[ i ].a)));
    }
    // Done reading, so unmap.
    static_cast< vk::Device > (*_device).unmapMemory( bufferMemory );

    // Now we save the acquired color data to a .png.
    unsigned error = lodepng::encode((LAVA_EXAMPLES_RESOURCES_ROUTE + 
      std::string("/mandelbrot.png")).c_str( ), image, width, height);
    if (error) printf("encoder error %d: %s", error, lodepng_error_text(error));
  }
  const int WorkgroupSize = 32; // Workgroup size in compute shader.
  void doPaint( void ) override
  {
    uint32_t width = _window->getWidth( );
    uint32_t height = _window->getHeight( );

    // create a command pool for command buffer allocation
    std::shared_ptr<CommandPool> commandPool = _device->createCommandPool( 
      vk::CommandPoolCreateFlagBits::eResetCommandBuffer, _queueFamilyIndex );
    std::shared_ptr<CommandBuffer> commandBuffer = commandPool->allocateCommandBuffer( );

    commandBuffer->begin( );

    std::array<float, 4> ccv = { 0.2f, 0.3f, 0.3f, 1.0f };
    commandBuffer->bindComputePipeline( _pipeline );
    commandBuffer->bindDescriptorSets( vk::PipelineBindPoint::eCompute,
      _pipelineLayout, 0, { _descriptorSet }, nullptr );

    commandBuffer->dispatch( 
      (uint32_t)ceil(width / float(WorkgroupSize)), 
      (uint32_t)ceil(height / float(WorkgroupSize)), 
      1
    );

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