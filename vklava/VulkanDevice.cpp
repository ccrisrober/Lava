#include "VulkanDevice.h"
#include <vector>
#include <assert.h>

namespace lava
{
  VulkanDevice::VulkanDevice( VkPhysicalDevice device, uint32_t deviceIdx )
    : _physicalDevice( device )
    , _logicalDevice( nullptr )
    , _isPrimary( false )
    , _deviceIdx( deviceIdx )
  {

    vkGetPhysicalDeviceProperties( device, &_deviceProperties );

    VkPhysicalDeviceProperties deviceProperties;
    memset( &deviceProperties, 0, sizeof deviceProperties );
    vkGetPhysicalDeviceProperties( device, &deviceProperties );
    printf( "Driver Version: %d\n", deviceProperties.driverVersion );
    printf( "Device Name:    %s\n", deviceProperties.deviceName );
    printf( "Device Type:    %d\n", deviceProperties.deviceType );
    printf( "API Version:    %d.%d.%d\n",
      // See note below regarding this:
      ( deviceProperties.apiVersion >> 22 ) & 0x3FF,
      ( deviceProperties.apiVersion >> 12 ) & 0x3FF,
      ( deviceProperties.apiVersion & 0xFFF ) );

    vkGetPhysicalDeviceFeatures( device, &_deviceFeatures );
    vkGetPhysicalDeviceMemoryProperties( device, &_memoryProperties );

    uint32_t numQueueFamilies;
    vkGetPhysicalDeviceQueueFamilyProperties( device, &numQueueFamilies, nullptr );

    std::vector<VkQueueFamilyProperties> queueFamilyProperties( numQueueFamilies );
    vkGetPhysicalDeviceQueueFamilyProperties( device, &numQueueFamilies,
      queueFamilyProperties.data( ) );


    // Create queues
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;

    auto populateQueueInfo = [ &] ( GpuQueueType type, uint32_t familyIdx )
    {
      VkDeviceQueueCreateInfo queueCreateInfo;
      queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
      queueCreateInfo.pNext = nullptr;
      queueCreateInfo.flags = 0;
      queueCreateInfo.queueFamilyIndex = familyIdx;
      queueCreateInfo.queueCount = 1;// queueFamilyProperties[ familyIdx ].queueCount;

      float queuePriority = 1.0f;
      queueCreateInfo.pQueuePriorities = &queuePriority;

      _queueInfos[ type ].familyIdx = familyIdx;
      _queueInfos[ type ].queues.resize( queueCreateInfo.queueCount, nullptr );
      queueCreateInfos.push_back( queueCreateInfo );
    };

    // Looks for dedicated upload queues
    for ( uint32_t i = 0; i < numQueueFamilies; ++i )
    {
      if ( ( queueFamilyProperties[ i ].queueFlags & VK_QUEUE_TRANSFER_BIT ) &&
        ( ( queueFamilyProperties[ i ].queueFlags & VK_QUEUE_GRAPHICS_BIT ) == 0 ) &&
        ( ( queueFamilyProperties[ i ].queueFlags & VK_QUEUE_COMPUTE_BIT ) == 0 ) )
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
    }

    // Looks for graphics queues
    for ( uint32_t i = 0; i < numQueueFamilies; ++i )
    {
      if ( queueFamilyProperties[ i ].queueFlags & VK_QUEUE_GRAPHICS_BIT )
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

    VkDeviceCreateInfo deviceInfo;
    deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceInfo.pNext = nullptr;
    deviceInfo.flags = 0;
    deviceInfo.queueCreateInfoCount = ( uint32_t ) queueCreateInfos.size( );
    deviceInfo.pQueueCreateInfos = queueCreateInfos.data( );
    deviceInfo.pEnabledFeatures = &_deviceFeatures;
    deviceInfo.enabledExtensionCount = numExtensions;
    deviceInfo.ppEnabledExtensionNames = extensions.data( );
    deviceInfo.enabledLayerCount = 0;
    deviceInfo.ppEnabledLayerNames = nullptr;

    VkResult result = vkCreateDevice( device, &deviceInfo, nullptr, &_logicalDevice );
    assert( result == VK_SUCCESS );

    // Retrieve queues
    for ( uint32_t i = 0; i < GPUT_COUNT; ++i )
    {
      uint32_t numQueues = ( uint32_t ) _queueInfos[ i ].queues.size( );
      for ( uint32_t j = 0; j < numQueues; ++j )
      {
        VkQueue queue;
        vkGetDeviceQueue( _logicalDevice, _queueInfos[ i ].familyIdx, j, &queue );

        _queueInfos[ i ].queues[ j ] = new VulkanQueue(
          *this, queue, ( GpuQueueType ) i, j );
      }
    }
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
    vkDestroyDevice( _logicalDevice, nullptr );
  }

  void VulkanDevice::waitIdle( void ) const
  {
    VkResult result = vkDeviceWaitIdle( _logicalDevice );
    assert( result == VK_SUCCESS );
  }
  void VulkanDevice::freeMemory( VkDeviceMemory memory )
  {
    vkFreeMemory( _logicalDevice, memory, nullptr );
  }

  uint32_t VulkanDevice::findMemoryType( uint32_t requirementBits, 
    VkMemoryPropertyFlags wantedFlags )
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

  VkDeviceMemory VulkanDevice::allocateImageMemory( VkImage image, 
    VkMemoryPropertyFlags flags )
  {
    VkMemoryRequirements memReq;
    vkGetImageMemoryRequirements( _logicalDevice, image, &memReq );

    VkDeviceMemory memory = allocateMemReqMemory( memReq, flags );

    VkResult result = vkBindImageMemory( _logicalDevice, image, memory, 0 );
    assert( result == VK_SUCCESS );

    return memory;
  }

  VkDeviceMemory VulkanDevice::allocateBufferMemory( VkBuffer buffer, 
    VkMemoryPropertyFlags flags )
  {
    VkMemoryRequirements memReq;
    vkGetBufferMemoryRequirements( _logicalDevice, buffer, &memReq );

    VkDeviceMemory memory = allocateMemReqMemory( memReq, flags );

    VkResult result = vkBindBufferMemory( _logicalDevice, buffer, memory, 0 );
    assert( result == VK_SUCCESS );

    return memory;
  }
  VkDeviceMemory VulkanDevice::allocateMemReqMemory( const VkMemoryRequirements& reqs,
    VkMemoryPropertyFlags flags )
  {
    VkMemoryAllocateInfo allocateInfo;
    allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocateInfo.pNext = nullptr;
    allocateInfo.memoryTypeIndex = findMemoryType( reqs.memoryTypeBits, flags );
    allocateInfo.allocationSize = reqs.size;

    if ( allocateInfo.memoryTypeIndex == -1 )
      return VK_NULL_HANDLE;

    VkDeviceMemory memory;

    VkResult result = vkAllocateMemory( _logicalDevice, &allocateInfo, 
      nullptr, &memory );
    assert( result == VK_SUCCESS );

    return memory;
  }
}