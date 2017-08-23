#include "VulkanTexture.h"
#include <assert.h>

namespace lava
{
  VulkanImage::VulkanImage( VulkanDevicePtr device, const VULKAN_IMAGE_DESC& desc )
    : VulkanResource( device )
  {
    _imageViewCI = { };
    _imageViewCI.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    _imageViewCI.pNext = nullptr;
    _imageViewCI.flags = 0;
    _imageViewCI.image = desc.image;

    switch ( desc.type )
    {
    case TEX_TYPE_1D:
      _imageViewCI.viewType = VK_IMAGE_VIEW_TYPE_1D;
      break;
    default:
    case TEX_TYPE_2D:
      _imageViewCI.viewType = VK_IMAGE_VIEW_TYPE_2D;
      break;
    case TEX_TYPE_3D:
      _imageViewCI.viewType = VK_IMAGE_VIEW_TYPE_3D;
      break;
    case TEX_TYPE_CUBE_MAP:
      _imageViewCI.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
      break;
    }

    _imageViewCI.format = desc.format;
    _imageViewCI.components = {
      VK_COMPONENT_SWIZZLE_R,
      VK_COMPONENT_SWIZZLE_G,
      VK_COMPONENT_SWIZZLE_B,
      VK_COMPONENT_SWIZZLE_A
    };
  }
  VulkanImage::~VulkanImage( void )
  {
    VkDevice vkDevice = _device->getLogical( );

    /*UINT32 numSubresources = mNumFaces * mNumMipLevels;
    for ( UINT32 i = 0; i < numSubresources; i++ )
    {
      assert( !mSubresources[ i ]->isBound( ) ); // Image beeing freed but its subresources are still bound somewhere

      mSubresources[ i ]->destroy( );
    }

    for ( auto& entry : mImageInfos )
      vkDestroyImageView( vkDevice, entry.view, gVulkanAllocator );*/

    //vkDestroyImageView( _device->getLogical( ), _view )

    //vkDestroyImage( vkDevice, _image, nullptr );
    //_device->freeMemory( _memory );
  }
  void* VulkanImage::map( uint32_t offset, uint32_t size ) const
  {
    void* data;
    VkResult result = vkMapMemory( _device->getLogical( ), _memory, 
      offset, size, 0, ( void** ) &data );
    assert( result == VK_SUCCESS );

    return data;
  }
  void VulkanImage::unmap( void )
  {
    vkUnmapMemory( _device->getLogical( ), _memory );
  }
  VkImageView VulkanImage::getView( void ) const
  {
    return createView( );
  }
  VkImageView VulkanImage::createView( void ) const
  {
    _imageViewCI.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    _imageViewCI.subresourceRange.baseMipLevel = 0;
    _imageViewCI.subresourceRange.levelCount = 1;
    _imageViewCI.subresourceRange.baseArrayLayer = 0;
    _imageViewCI.subresourceRange.layerCount = 1;

    VkImageView view;
    VkResult result = vkCreateImageView( _device->getLogical( ), &_imageViewCI, nullptr,
      &view );
    assert( result == VK_SUCCESS );

    return view;
  }

  void Texture2D::loadFromFile( const std::string& filename, VkFormat format,
    VulkanDevicePtr device, VkImageUsageFlags imageUsageFlags,
    VkImageLayout imageLayout,
    bool forceLinear )
  {
    /*VkBool32 useStaging = !forceLinear;
    
    VkMemoryAllocateInfo memAllocInfo;
    VkMemoryRequirements memReqs;*/

  }
}