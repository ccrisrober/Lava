#include <lava/lava.h>
using namespace lava;
#include <routes.h>

int main( void )
{
  std::cout << "Create Vulkan Instance...";
  std::shared_ptr<Instance> instance;

  // Create instance
  vk::ApplicationInfo appInfo(
    "App Name",
    VK_MAKE_VERSION( 1, 0, 0 ),
    "FooEngine",
    VK_MAKE_VERSION( 1, 0, 0 ),
    VK_API_VERSION_1_0
  );

  std::vector<const char*> layers =
  {
#ifndef NDEBUG
    "VK_LAYER_LUNARG_standard_validation",
#endif
  };
  std::vector<const char*> extensions =
  {
    VK_EXT_DEBUG_REPORT_EXTENSION_NAME
  };

  instance = Instance::create( vk::InstanceCreateInfo(
    { },
    &appInfo,
    layers.size( ),
    layers.data( ),
    extensions.size( ),
    extensions.data( )
  ) );
  std::cout << "OK" << std::endl;

  std::cout << "Find Vulkan physical device...";
  
  // Find a physical device with presentation support
  assert( instance->getPhysicalDeviceCount( ) != 0 );
  auto physicalDevice = instance->getPhysicalDevice( 0 );
  if ( !physicalDevice )
  {
    LAVA_RUNTIME_ERROR( "Failed to find a device with presentation support" );
  }
  std::cout << "OK" << std::endl;

  std::cout << "Create logical device...";
  // Search for a compute queue in the array of 
  //    queue families, try to find one that support
  auto queueFamilyIndices = physicalDevice->getComputeQueueFamilyIndices( );
  assert( !queueFamilyIndices.empty( ) );
  const uint32_t queueFamilyIndex = queueFamilyIndices[ 0 ];
  std::vector<float> queuePriorities = { 1.0f };
  vk::DeviceQueueCreateInfo queueCreateInfo;
  queueCreateInfo.queueFamilyIndex = static_cast<uint32_t>( queueFamilyIndex );
  queueCreateInfo.queueCount = static_cast<uint32_t>( queuePriorities.size( ) );
  queueCreateInfo.pQueuePriorities = &queuePriorities[ 0 ];
  vk::DeviceCreateInfo deviceCreateInfo;
  deviceCreateInfo.enabledExtensionCount = 0;
  deviceCreateInfo.ppEnabledExtensionNames = nullptr;
  deviceCreateInfo.queueCreateInfoCount = 1;
  deviceCreateInfo.pQueueCreateInfos = &queueCreateInfo;

  auto device = physicalDevice->createDevice(
    { queueCreateInfo }, { }, { },  physicalDevice->getDeviceFeatures( )
  );

  std::cout << "OK" << std::endl;

  const uint32_t bufferElements = 10;
  const vk::DeviceSize bufferSize = bufferElements * sizeof( int32_t );

  std::cout << "Allocate buffers...";
  std::shared_ptr< StorageBuffer > inABuffer, inBBuffer, outBuffer;
  inABuffer = device->createStorageBuffer( bufferSize );
  inBBuffer = device->createStorageBuffer( bufferSize );
  outBuffer = device->createStorageBuffer( bufferSize );
  std::cout << "OK" << std::endl;


  std::cout << "Loading shader... ";
  auto computeStage = device->createShaderPipelineShaderStage(
    LAVA_EXAMPLES_SPV_ROUTE + std::string( "arraySum_comp.spv" ),
    vk::ShaderStageFlagBits::eCompute
  );

  std::array<DescriptorSetLayoutBinding, 3> dslb =
  {
    DescriptorSetLayoutBinding( 0, vk::DescriptorType::eStorageBuffer, 
      vk::ShaderStageFlagBits::eCompute ),
    DescriptorSetLayoutBinding( 1, vk::DescriptorType::eStorageBuffer, 
      vk::ShaderStageFlagBits::eCompute ),
    DescriptorSetLayoutBinding( 2, vk::DescriptorType::eStorageBuffer, 
      vk::ShaderStageFlagBits::eCompute )
  };
  auto descriptorSetLayout = device->createDescriptorSetLayout( dslb );

  std::shared_ptr<DescriptorPool> descriptorPool =
    device->createDescriptorPool( 1, {
      { vk::DescriptorType::eStorageBuffer, 3 }
    } );
  auto descriptorSet = device->allocateDescriptorSet( descriptorPool, descriptorSetLayout );
  std::cout << "OK" << std::endl;

  std::cout << "Create pipeline...";
  auto pipelineLayout = device->createPipelineLayout( descriptorSetLayout );
  auto pipeline = device->createComputePipeline(
    nullptr, {}, computeStage, pipelineLayout );

  std::cout << "OK" << std::endl;


  std::cout << "Prepare commands buffers...";
  auto commandPool = device->createCommandPool( { }, queueFamilyIndex );
  std::cout << "OK" << std::endl;

  std::cout << "Prepare descriptors set...";
  std::vector< WriteDescriptorSet > wdss =
  {
    WriteDescriptorSet( descriptorSet, 0, 0,
      vk::DescriptorType::eStorageBuffer, 1, nullptr,
      DescriptorBufferInfo( inABuffer, 0, bufferSize )
    ),
    WriteDescriptorSet( descriptorSet, 1, 0,
      vk::DescriptorType::eStorageBuffer, 1, nullptr,
      DescriptorBufferInfo( inBBuffer, 0, bufferSize )
    ),
    WriteDescriptorSet( descriptorSet, 2, 0,
      vk::DescriptorType::eStorageBuffer, 1, nullptr,
      DescriptorBufferInfo( outBuffer, 0, bufferSize )
    )
  };
  device->updateDescriptorSets( wdss, { } );
  std::cout << "OK" << std::endl;

  std::cout << "Upload input data...";

  std::vector<float> hostDataA( bufferElements );
  const float min = 1.0f, max = 99.0f;
  for ( float& n : hostDataA )
  {
    n = ( max - min ) * ( ( ( ( float ) rand( ) ) / ( float ) RAND_MAX ) ) + min;
  }
  inABuffer->writeData( 0, bufferSize, hostDataA.data( ) );

  std::vector<float> hostDataB( bufferElements );

  std::vector<float> cpuResult( bufferElements );
  int i = 0;
  for ( float& n : hostDataB )
  {
    n = min + ( max - min ) * ( ( ( ( float ) rand( ) ) / ( float ) RAND_MAX ) ) + min;

    cpuResult[ i ] = hostDataA[ i ] + n;
    ++i;
  }
  inBBuffer->writeData( 0, bufferSize, hostDataB.data( ) );

  std::cout << "OK" << std::endl;

  std::cout << "Input data:" << std::endl;

  for ( int32_t i = 0; i < 10; ++i )
  {
    std::cout << (hostDataA[ i ] + hostDataB[ i ]) << ", ";
  }
  std::cout << "..., " << ( hostDataA[ bufferElements - 1 ] + 
    hostDataB[ bufferElements - 1 ]) << std::endl;

  std::cout << "Run computations...";

  auto commandBuffer = commandPool->allocateCommandBuffer( );
  commandBuffer->beginSimple( vk::CommandBufferUsageFlagBits::eOneTimeSubmit );
  commandBuffer->bindComputePipeline( pipeline );
  commandBuffer->bindDescriptorSets( vk::PipelineBindPoint::eCompute, 
    pipelineLayout, 0, { descriptorSet }, { } );
  commandBuffer->dispatch( bufferElements, 1, 1 );
  commandBuffer->end( );

  auto queue = device->getQueue( queueFamilyIndex, 0 );

  queue->submit( commandBuffer );
  queue->waitIdle( );

  std::cout << "OK" << std::endl;

  std::cout << "Read results...";
  std::vector<float> result( bufferElements );
  outBuffer->readData( 0, bufferSize, result.data( ) );

  if ( std::equal( result.begin( ), result.end( ), 
    cpuResult.begin( ), []( float r, float h ) { return r == h; } ) )
  {
    std::cout << "OK" << std::endl;
  }
  else
  {
    std::cout << "Fail. Invalid result" << std::endl;
  }

  std::cout << "Output data:" << std::endl;
  for ( int32_t i = 0; i < 10; ++i )
  {
    std::cout << result[ i ] << ", ";
  }
  std::cout << "..., " << result[ bufferElements - 1 ] << std::endl;
  std::cout << std::endl;

  return 0;
}