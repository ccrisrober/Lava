#include "VulkanDevice.h"
#include <vector>
#include <assert.h>

VulkanDevice::VulkanDevice( VkPhysicalDevice device, uint32_t deviceIdx )
  : _physicalDevice( device )
  , _logicalDevice( nullptr )
  , _isPrimary( false )
  ,_deviceIdx( deviceIdx )
{

  vkGetPhysicalDeviceProperties( device, &_deviceProperties );
  vkGetPhysicalDeviceFeatures( device, &_deviceFeatures );
  //vkGetPhysicalDeviceMemoryProperties( device, &_memoryProperties );

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

  // Looks for graphics queues
  for ( uint32_t i = 0, size = queueFamilyProperties.size( ); i < size; ++i )
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

VulkanDevice::~VulkanDevice( )
{
  vkDestroyDevice( _logicalDevice, nullptr );
}

void VulkanDevice::waitIdle( void ) const
{
  VkResult result = vkDeviceWaitIdle( _logicalDevice );
  assert( result == VK_SUCCESS );
}