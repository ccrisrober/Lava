#include "VulkanSwapChain.h"
#include <iostream>

VulkanSwapChain::VulkanSwapChain( )
{
}


VulkanSwapChain::~VulkanSwapChain( )
{
  VkDevice logicalDevice = _device->getLogical( );
  for ( uint32_t i = 0, s = swapChainImageViews.size( ); i < s; ++i )
  {
    vkDestroyImageView( logicalDevice, swapChainImageViews[ i ], nullptr );
  }
  if ( _swapChain != VK_NULL_HANDLE )
  {
    vkDestroySwapchainKHR( logicalDevice, _swapChain, nullptr );
  }
}

void VulkanSwapChain::rebuild( VulkanDevicePtr device, VkSurfaceKHR& surface, 
  uint32_t w, uint32_t h, bool vsync, VkFormat colorFormat, VkColorSpaceKHR colorSpace )
{
  _device = device;

  VkResult result;
  VkPhysicalDevice physicalDevice = _device->getPhysical( );


  // Determine swap chain dimensions
  VkSurfaceCapabilitiesKHR surfaceCaps;
  result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR( physicalDevice, surface,
    &surfaceCaps );
  assert( result == VK_SUCCESS );

  if ( surfaceCaps.currentExtent.width != std::numeric_limits<uint32_t>::max( ) )
  {
    swapchainExtent = surfaceCaps.currentExtent;
  }
  else
  {
    VkExtent2D actualExtent = { w, h };

    actualExtent.width = std::max( surfaceCaps.minImageExtent.width,
      std::min( surfaceCaps.maxImageExtent.width, actualExtent.width ) );
    actualExtent.height = std::max( surfaceCaps.minImageExtent.height,
      std::min( surfaceCaps.maxImageExtent.height, actualExtent.height ) );

    swapchainExtent = actualExtent;
  }

  _width = swapchainExtent.width;
  _height = swapchainExtent.height;


  // Find present mode
  uint32_t numPresentModes;
  result = vkGetPhysicalDeviceSurfacePresentModesKHR( physicalDevice,
    surface, &numPresentModes, nullptr );
  assert( result == VK_SUCCESS );
  assert( numPresentModes > 0 );

  std::vector<VkPresentModeKHR> presentModes;
  presentModes.resize( numPresentModes );
  result = vkGetPhysicalDeviceSurfacePresentModesKHR( physicalDevice, surface,
    &numPresentModes, presentModes.data( ) );
  assert( result == VK_SUCCESS );

  VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;
  if ( !vsync )
  {
    for ( const auto& pr : presentModes )
    {
      if ( pr == VK_PRESENT_MODE_IMMEDIATE_KHR )
      {
        presentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
        break;
      }

      if ( pr == VK_PRESENT_MODE_FIFO_RELAXED_KHR )
      {
        presentMode = VK_PRESENT_MODE_FIFO_RELAXED_KHR;
      }
    }
  }
  else
  {
    // Mailbox comes with lower input latency than FIFO, but can waste GPU 
    //    power by rendering frames that are never
    // displayed, especially if the app runs much faster than the refresh 
    //    rate. This is a concern for mobiles.
    for ( const auto& pr : presentModes )
    {
      if ( pr == VK_PRESENT_MODE_MAILBOX_KHR )
      {
        presentMode = VK_PRESENT_MODE_MAILBOX_KHR;
        break;
      }
    }
  }
  presentModes.clear( );


  uint32_t numImages = surfaceCaps.minImageCount;

  VkSurfaceTransformFlagsKHR transform;
  if ( surfaceCaps.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR )
  {
    transform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
  }
  else
  {
    transform = surfaceCaps.currentTransform;
  }

  VkSwapchainCreateInfoKHR swapChainCI;// = { };
  swapChainCI.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  swapChainCI.pNext = nullptr;
  swapChainCI.flags = 0;
  swapChainCI.surface = surface;

  swapChainCI.minImageCount = numImages;
  swapChainCI.imageFormat = colorFormat;
  swapChainCI.imageColorSpace = colorSpace;
  swapChainCI.imageExtent = { swapchainExtent.width, swapchainExtent.height };
  swapChainCI.imageArrayLayers = 1;
  swapChainCI.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;


  swapChainCI.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
  swapChainCI.queueFamilyIndexCount = 0;
  swapChainCI.pQueueFamilyIndices = nullptr;

  swapChainCI.preTransform = ( VkSurfaceTransformFlagBitsKHR ) transform;
  swapChainCI.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
  swapChainCI.presentMode = presentMode;
  swapChainCI.clipped = VK_TRUE;

  swapChainCI.oldSwapchain = _swapChain; // At this time, _swapChain as VK_NULL_HANDLE;

  VkDevice logicalDevice = _device->getLogical( );
  result = vkCreateSwapchainKHR( logicalDevice, &swapChainCI, nullptr, &_swapChain );
  assert( result == VK_SUCCESS );


  result = vkGetSwapchainImagesKHR( logicalDevice, _swapChain, &numImages, nullptr );
  assert( result == VK_SUCCESS );

  std::vector<VkImage> swapChainImages( numImages );
  result = vkGetSwapchainImagesKHR( logicalDevice, _swapChain, &numImages, swapChainImages.data( ) );
  assert( result == VK_SUCCESS );


  VULKAN_IMAGE_DESC imageDesc;
  imageDesc.format = colorFormat;
  imageDesc.type = TEX_TYPE_2D;
  imageDesc.usage = TU_RENDERTARGET;
  imageDesc.layout = VK_IMAGE_LAYOUT_UNDEFINED;
  imageDesc.numFaces = 1;
  imageDesc.numMipLevels = 1;
  imageDesc.memory = VK_NULL_HANDLE;

  swapChainImageViews.resize( swapChainImages.size( ) );

  for ( size_t i = 0, s = swapChainImages.size( ); i < s; ++i )
  {
    imageDesc.image = swapChainImages[ i ];

    VkImageViewCreateInfo imageViewCI;
    imageViewCI.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    imageViewCI.pNext = nullptr;
    imageViewCI.flags = 0;
    imageViewCI.image = imageDesc.image;

    switch ( imageDesc.type )
    {
      case TEX_TYPE_1D:
        imageViewCI.viewType = VK_IMAGE_VIEW_TYPE_1D;
        break;
      default:
      case TEX_TYPE_2D:
        imageViewCI.viewType = VK_IMAGE_VIEW_TYPE_2D;
        break;
      case TEX_TYPE_3D:
        imageViewCI.viewType = VK_IMAGE_VIEW_TYPE_3D;
        break;
      case TEX_TYPE_CUBE_MAP:
        imageViewCI.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
        break;
    }
    imageViewCI.format = imageDesc.format;

    imageViewCI.components =
    {
      /*VK_COMPONENT_SWIZZLE_IDENTITY,
      VK_COMPONENT_SWIZZLE_IDENTITY,
      VK_COMPONENT_SWIZZLE_IDENTITY,
      VK_COMPONENT_SWIZZLE_IDENTITY*/

      VK_COMPONENT_SWIZZLE_R,
      VK_COMPONENT_SWIZZLE_G,
      VK_COMPONENT_SWIZZLE_B,
      VK_COMPONENT_SWIZZLE_A
    };
    imageViewCI.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    imageViewCI.subresourceRange.baseMipLevel = 0;
    imageViewCI.subresourceRange.levelCount = 1;
    imageViewCI.subresourceRange.baseArrayLayer = 0;
    imageViewCI.subresourceRange.layerCount = 1;

    VkResult result = vkCreateImageView( logicalDevice, &imageViewCI, nullptr, &swapChainImageViews[ i ] );
    assert( result == VK_SUCCESS );
  }
  std::cout << "ImageViews OK" << std::endl;
}