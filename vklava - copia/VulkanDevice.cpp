#include "VulkanDevice.h"
#include <vector>
#include <assert.h>
#include <iostream>
#include "VulkanCommandBuffer.h"

namespace lava
{
  VulkanDevice::VulkanDevice(vk::PhysicalDevice device, uint32_t deviceIdx )
    : _physicalDevice( device )
    , _logicalDevice( nullptr )
    , _isPrimary( false )
    , _deviceIdx( deviceIdx )
  {

	std::vector<vk::LayerProperties> properties = _physicalDevice.enumerateDeviceLayerProperties();
    for ( const auto& prop : properties )
    {
      std::cout << "\t" << prop.layerName << std::endl;
    }

	_deviceProperties = _physicalDevice.getProperties();
    const char* devTypeStr = "";
    switch ( _deviceProperties.deviceType )
    {
	case vk::PhysicalDeviceType::eOther:
        devTypeStr = "VK_PHYSICAL_DEVICE_TYPE_OTHER";
        break;
	case vk::PhysicalDeviceType::eIntegratedGpu:
        devTypeStr = "VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU";
        break;
      case vk::PhysicalDeviceType::eDiscreteGpu:
        devTypeStr = "VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU";
        break;
      case vk::PhysicalDeviceType::eVirtualGpu:
        devTypeStr = "VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU";
        break;
      case vk::PhysicalDeviceType::eCpu:
        devTypeStr = "VK_PHYSICAL_DEVICE_TYPE_CPU";
        break;
    }

    VkPhysicalDeviceProperties deviceProperties = _physicalDevice.getProperties();
    printf( "Driver Version: %d\n", deviceProperties.driverVersion );
    printf( "Device Name:    %s\n", deviceProperties.deviceName );
    printf( "Device Type:    %s(%d)\n", devTypeStr, deviceProperties.deviceType );
    printf( "API Version:    %d.%d.%d\n",
      // See note below regarding this:
      ( deviceProperties.apiVersion >> 22 ) & 0x3FF,
      ( deviceProperties.apiVersion >> 12 ) & 0x3FF,
      ( deviceProperties.apiVersion & 0xFFF ) );

	_deviceFeatures = _physicalDevice.getFeatures();
	_memoryProperties = _physicalDevice.getMemoryProperties();

	std::vector<vk::QueueFamilyProperties> queueFamilyProperties = 
		_physicalDevice.getQueueFamilyProperties();


    // Create queues
    std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;

    auto populateQueueInfo = [ &] ( GpuQueueType type, uint32_t familyIdx )
    {
	  float queuePriority = 1.0f;
      vk::DeviceQueueCreateInfo queueCreateInfo(
		  vk::DeviceQueueCreateFlags(),
		  familyIdx,
		  1,
		  &queuePriority
	  );

      _queueInfos[ type ].familyIdx = familyIdx;
      _queueInfos[ type ].queues.resize( queueCreateInfo.queueCount, nullptr );
      queueCreateInfos.push_back( queueCreateInfo );
    };
	uint32_t numQueueFamilies = queueFamilyProperties.size();
    // Looks for dedicated upload queues
	/*
    for ( uint32_t i = 0; i < numQueueFamilies; ++i )
    {
      if ( ( queueFamilyProperties[ i ].queueFlags & vk::QueueFlagBits::eTransfer) &&
        ( ( queueFamilyProperties[ i ].queueFlags & vk::QueueFlagBits::eGraphics) == 0 ) &&
        ( ( queueFamilyProperties[ i ].queueFlags & vk::QueueFlagBits::eCompute) == 0 ) )
      {
        populateQueueInfo( GPUT_TRANSFER, i );
        break;
      }
    }

    // Looks for dedicated compute queues
    for ( uint32_t i = 0; i < numQueueFamilies; ++i )
    {
      if ( ( queueFamilyProperties[ i ].queueFlags & VK_QUEUE_COMPUTE_BIT ) && 
        ( queueFamilyProperties[ i ].queueFlags & VK_QUEUE_GRAPHICS_BIT ) == 0 )
      {
        populateQueueInfo( GPUT_COMPUTE, i );
        break;
      }
    }*/

    // Looks for graphics queues
    for ( uint32_t i = 0; i < numQueueFamilies; ++i )
    {
      if ( queueFamilyProperties[ i ].queueFlags & vk::QueueFlagBits::eGraphics)
      {
        populateQueueInfo( GPUT_GRAPHICS, i );
        break;
      }
    }

    // Create logical device
    std::vector< const char*> extensions =
    {
      VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };
    uint32_t numExtensions = extensions.size( );

	vk::DeviceCreateInfo deviceInfo(
		vk::DeviceCreateFlags(),
		queueCreateInfos.size(),
		queueCreateInfos.data(),
		0,
		nullptr,
		numExtensions,
		extensions.data(),
		&_deviceFeatures
	);

	_logicalDevice = _physicalDevice.createDevice(deviceInfo);

    // Retrieve queues
    for ( uint32_t i = 0; i < GPUT_COUNT; ++i )
    {
      uint32_t numQueues = ( uint32_t ) _queueInfos[ i ].queues.size( );
      for ( uint32_t j = 0; j < numQueues; ++j )
      {
        vk::Queue queue =_logicalDevice.getQueue(_queueInfos[i].familyIdx, j);

        _queueInfos[ i ].queues[ j ] = new VulkanQueue(
          *this, queue, ( GpuQueueType ) i, j );
      }
    }

    _commandBufferPool = new VulkanCmdBufferPool( *this );
  }

  VulkanDevice::~VulkanDevice( void )
  {
    waitIdle( );
    for ( uint32_t i = 0; i < GPUT_COUNT; ++i )
    {
      uint32_t numQueues = ( uint32_t ) _queueInfos[ i ].queues.size( );
      for ( uint32_t j = 0; j < numQueues; ++j )
      {
        _queueInfos[ i ].queues[ j ]->waitIdle( );
        // _queueInfos[ i ].queues[ j ]->refreshStates( true, true );
        delete _queueInfos[ i ].queues[ j ];
      }
      _queueInfos[ i ].queues.clear( );
    }

    delete _commandBufferPool;

	_logicalDevice.destroy();
  }

  void VulkanDevice::waitIdle( void ) const
  {
	  _logicalDevice.waitIdle( );
  }
  void VulkanDevice::freeMemory( vk::DeviceMemory memory )
  {
	  _logicalDevice.freeMemory(memory);
  }

  uint32_t VulkanDevice::findMemoryType( uint32_t requirementBits, 
    vk::MemoryPropertyFlags wantedFlags )
  {
    for ( uint32_t i = 0; i < _memoryProperties.memoryTypeCount; i++ )
    {
      if ( requirementBits & ( 1 << i ) )
      {
        if ( ( _memoryProperties.memoryTypes[ i ].propertyFlags & 
          wantedFlags ) == wantedFlags )
          return i;
      }
    }

    return -1;
  }

  vk::DeviceMemory VulkanDevice::allocateImageMemory( vk::Image image,
    vk::MemoryPropertyFlags flags )
  {
	vk::MemoryRequirements memReq = _logicalDevice.getImageMemoryRequirements( image );

	vk::DeviceMemory memory = allocateMemReqMemory( memReq, flags );

	_logicalDevice.bindImageMemory(image, memory, 0);

	return memory;
  }

  vk::DeviceMemory VulkanDevice::allocateBufferMemory(vk::Buffer buffer,
	  vk::MemoryPropertyFlags flags )
  {
	  vk::MemoryRequirements memReq = _logicalDevice.getBufferMemoryRequirements(buffer);

    vk::DeviceMemory memory = allocateMemReqMemory( memReq, flags );

	_logicalDevice.bindBufferMemory(buffer, memory, 0);

    return memory;
  }
  vk::DeviceMemory VulkanDevice::allocateMemReqMemory( const vk::MemoryRequirements& reqs,
	  vk::MemoryPropertyFlags flags )
  {
	  vk::MemoryAllocateInfo allocateInfo(
		  reqs.size,
		  findMemoryType(reqs.memoryTypeBits, flags)
	  );

    if ( allocateInfo.memoryTypeIndex == -1 )
      return VK_NULL_HANDLE;

	vk::DeviceMemory memory = _logicalDevice.allocateMemory(&allocateInfo);

    return memory;
  }
}