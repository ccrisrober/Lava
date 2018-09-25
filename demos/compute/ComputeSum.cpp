/**
 * Copyright (c) 2017 - 2018, Pompeii
 * All rights reserved.
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 **/

#include <pompeii/pompeii.h>
using namespace pompeii;

#include <routes.h>

int main( void )
{
  std::cout << "Create Vulkan Instance...";
    auto instance = Instance::createDefault( "Compute Sum" );
  std::cout << "OK" << std::endl;

  std::cout << "Find Vulkan physical device...";
    // Find a physical device with presentation support
    assert( instance->getPhysicalDeviceCount( ) != 0 );
    auto physicalDevice = instance->getPhysicalDevice( 0 );
    if ( !physicalDevice )
    {
      POMPEII_RUNTIME_ERROR( "Failed to find a device with presentation support" );
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

  std::cout << "Allocate buffers...";
    const uint32_t bufferElements = 10;
    const vk::DeviceSize bufferSize = bufferElements * sizeof( int32_t );
    std::shared_ptr< StorageBuffer > inABuffer, inBBuffer, outBuffer;
    inABuffer = device->createStorageBuffer( bufferSize );
    inBBuffer = device->createStorageBuffer( bufferSize );
    outBuffer = device->createStorageBuffer( bufferSize );
  std::cout << "OK" << std::endl;


  std::cout << "Loading shader... ";
    auto computeStage = device->createShaderPipelineShaderStage(
      POMPEII_EXAMPLES_SPV_ROUTE + std::string( "arraySum_comp.spv" ),
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
    size_t i = 0;
    for ( float& n : hostDataB )
    {
      n = min + ( max - min ) * ( ( ( ( float ) rand( ) ) / ( float ) RAND_MAX ) ) + min;

      cpuResult[ i ] = hostDataA[ i ] + n;
      ++i;
    }
    inBBuffer->writeData( 0, bufferSize, hostDataB.data( ) );
  std::cout << "OK" << std::endl;

  std::cout << "Input data:" << std::endl;

  for ( i = 0; i < bufferElements; ++i )
  {
    std::cout << (hostDataA[ i ] + hostDataB[ i ]) << ", ";
  }
  std::cout << "..., " << ( hostDataA[ bufferElements - 1 ] + 
    hostDataB[ bufferElements - 1 ]) << std::endl;

  std::cout << "Run computations...";

  // Now create an event and wait for it on the GPU
  auto ev = device->createEvent( );

  auto commandBuffer = commandPool->allocateCommandBuffer( );
  commandBuffer->begin( vk::CommandBufferUsageFlagBits::eOneTimeSubmit );

  commandBuffer->setEvent( ev, vk::PipelineStageFlagBits::eBottomOfPipe );

  commandBuffer->bindComputePipeline( pipeline );
  commandBuffer->bindDescriptorSets( vk::PipelineBindPoint::eCompute, 
    pipelineLayout, 0, { descriptorSet }, { } );
  commandBuffer->dispatch( bufferElements, 1, 1 );
  commandBuffer->end( );

  auto queue = device->getQueue( queueFamilyIndex, 0 );

  // Look for the event on the CPU. It should be RESET since we haven't sent
  // the command buffer yet.
  auto res = ev->getStatus( );
  assert( res == vk::Result::eEventReset );

  // Send the command buffer and loop waiting for the event
  queue->submit( commandBuffer );

  int polls = 0;
  do
  {
    res = ev->getStatus( );
    ++polls;
  } while ( res != vk::Result::eEventSet );
  printf( "%d polls to find the event set\n", polls );

  queue->waitIdle( );

  std::cout << "OK" << std::endl;

  std::cout << "Read results...";
  std::vector<float> result( bufferElements );
  outBuffer->readData( 0, bufferSize, result.data( ) );

  if ( std::equal( result.begin( ), result.end( ), 
    cpuResult.begin( ), []( float r, float h ) { return r == h; } ) )
  {
    std::cout << "Ok. Same arays as equals." << std::endl;
  }
  else
  {
    std::cout << "Fail. Invalid result." << std::endl;
  }

  std::cout << "Output data:" << std::endl;
  for ( i = 0; i < bufferElements; ++i )
  {
    std::cout << result[ i ] << ", ";
  }
  std::cout << "..., " << result[ bufferElements - 1 ] << std::endl;
  std::cout << std::endl;

  system( "PAUSE" );

  return 0;
}