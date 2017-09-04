#include "utils.hpp"

#include <sstream>
#include <fstream>

#define STB_IMAGE_IMPLEMENTATION
#include <stbi/stb_image.h>

namespace lava
{
  unsigned char* utils::loadImageTexture( const std::string& fileName,
    uint32_t& width, uint32_t& height, uint32_t& numChannels )
  {
    int w, h, c;
    stbi_uc* pixels = stbi_load( fileName.c_str( ),
      &w, &h, &c, STBI_rgb_alpha );

    if ( !pixels )
    {
      throw std::runtime_error( "failed to load texture image!" );
    }

    width = w;
    height = h;
    numChannels = c;

    return pixels;
  }
  std::vector<char> utils::readBinaryile( const std::string& fileName )
  {
    std::ifstream file( fileName, std::ios::ate | std::ios::binary );

    if ( !file.is_open( ) )
    {
      throw std::runtime_error( "failed to open file!" );
    }

    size_t fileSize = ( size_t ) file.tellg( );
    std::vector<char> buffer( fileSize );

    file.seekg( 0 );
    file.read( buffer.data( ), fileSize );

    file.close( );

    return buffer;
  }
	const std::string utils::translateVulkanResult( vk::Result res )
	{
    VkResult result = VkResult( res );
    switch ( result ) {
      // Success codes
    case VK_SUCCESS:
      return "Command successfully completed.";
    case VK_NOT_READY:
      return "A fence or query has not yet completed.";
    case VK_TIMEOUT:
      return "A wait operation has not completed in the specified time.";
    case VK_EVENT_SET:
      return "An event is signaled.";
    case VK_EVENT_RESET:
      return "An event is unsignaled.";
    case VK_INCOMPLETE:
      return "A return array was too small for the result.";
    case VK_SUBOPTIMAL_KHR:
      return "A swapchain no longer matches the surface properties exactly, but can still be used to present to the surface successfully.";

      // Error codes
    case VK_ERROR_OUT_OF_HOST_MEMORY:
      return "A host memory allocation has failed.";
    case VK_ERROR_OUT_OF_DEVICE_MEMORY:
      return "A device memory allocation has failed.";
    case VK_ERROR_INITIALIZATION_FAILED:
      return "Initialization of an object could not be completed for implementation-specific reasons.";
    case VK_ERROR_DEVICE_LOST:
      return "The logical or physical device has been lost.";
    case VK_ERROR_MEMORY_MAP_FAILED:
      return "Mapping of a memory object has failed.";
    case VK_ERROR_LAYER_NOT_PRESENT:
      return "A requested layer is not present or could not be loaded.";
    case VK_ERROR_EXTENSION_NOT_PRESENT:
      return "A requested extension is not supported.";
    case VK_ERROR_FEATURE_NOT_PRESENT:
      return "A requested feature is not supported.";
    case VK_ERROR_INCOMPATIBLE_DRIVER:
      return "The requested version of Vulkan is not supported by the driver or is otherwise incompatible for implementation-specific reasons.";
    case VK_ERROR_TOO_MANY_OBJECTS:
      return "Too many objects of the type have already been created.";
    case VK_ERROR_FORMAT_NOT_SUPPORTED:
      return "A requested format is not supported on this device.";
    case VK_ERROR_SURFACE_LOST_KHR:
      return "A surface is no longer available.";
    case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR:
      return "The requested window is already connected to a VkSurfaceKHR, or to some other non-Vulkan API.";
    case VK_ERROR_OUT_OF_DATE_KHR:
      return "A surface has changed in such a way that it is no longer compatible with the swapchain, and further presentation requests using the swapchain will fail. Applications must query the new surface properties and recreate their swapchain if they wish to continue presenting to the surface.";
    case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR:
      return "The display used by a swapchain does not use the same presentable image layout, or is incompatible in a way that prevents sharing an image.";
    case VK_ERROR_VALIDATION_FAILED_EXT:
      return "A validation layer found an error.";
    default:
      std::stringstream ss;
      ss << "Unknown [" << result << "]";
      return ss.str( );
    }
	}
  void utils::setImageLayout( const std::shared_ptr<CommandBuffer>& cmd,
    vk::Image image,
    vk::ImageAspectFlags aspectMask,
    vk::ImageLayout oldImageLayout,
    vk::ImageLayout newImageLayout,
    vk::ImageSubresourceRange subresourceRange,
    vk::PipelineStageFlags srcStageMask,
    vk::PipelineStageFlags dstStageMask )
  {
    vk::ImageSubresourceRange subresourceRange;
    subresourceRange.aspectMask = aspectMask;
    subresourceRange.baseMipLevel = 0;
    subresourceRange.levelCount = 1;
    subresourceRange.layerCount = 1;
    setImageLayout( cmd, image, oldImageLayout, newImageLayout, 
      subresourceRange, srcStageMask, dstStageMask );
  }
  void utils::setImageLayout( const std::shared_ptr<CommandBuffer>& cmd,
    vk::Image image,
    vk::ImageLayout oldImageLayout,
    vk::ImageLayout newImageLayout,
    vk::ImageSubresourceRange subresourceRange,
    vk::PipelineStageFlags srcStageMask,
    vk::PipelineStageFlags dstStageMask )
  {



    vk::ImageMemoryBarrier barrier;
    barrier.oldLayout = oldImageLayout;
    barrier.newLayout = newImageLayout;
    barrier.image = image;
    barrier.subresourceRange = subresourceRange;

    // Source layouts (old)
    // Source access mask controls actions that have to be finished on the old layout
    // before it will be transitioned to the new layout
    switch ( oldImageLayout )
    {
    case vk::ImageLayout::eUndefined:
      // Image layout is undefined (or does not matter)
      // Only valid as initial layout
      // No flags required, listed only for completeness
      barrier.srcAccessMask = vk::AccessFlags( );
      break;

    case vk::ImageLayout::ePreinitialized:
      // Image is preinitialized
      // Only valid as initial layout for linear images, preserves memory contents
      // Make sure host writes have been finished
      barrier.srcAccessMask = vk::AccessFlagBits::eHostWrite;
      break;

    case vk::ImageLayout::eColorAttachmentOptimal:
      // Image is a color attachment
      // Make sure any writes to the color buffer have been finished
      barrier.srcAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
      break;

    case vk::ImageLayout::eDepthStencilAttachmentOptimal:
      // Image is a depth/stencil attachment
      // Make sure any writes to the depth/stencil buffer have been finished
      barrier.srcAccessMask = vk::AccessFlagBits::eDepthStencilAttachmentWrite;
      break;

    case vk::ImageLayout::eTransferSrcOptimal:
      // Image is a transfer source 
      // Make sure any reads from the image have been finished
      barrier.srcAccessMask = vk::AccessFlagBits::eTransferRead;
      break;

    case vk::ImageLayout::eTransferDstOptimal:
      // Image is a transfer destination
      // Make sure any writes to the image have been finished
      barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
      break;

    case vk::ImageLayout::eShaderReadOnlyOptimal:
      // Image is read by a shader
      // Make sure any shader reads from the image have been finished
      barrier.srcAccessMask = vk::AccessFlagBits::eShaderRead;
      break;
    default:
      // Other source layouts aren't handled (yet)
      break;
    }

    // Target layouts (new)
    // Destination access mask controls the dependency for the new image layout
    switch ( newImageLayout )
    {
    case vk::ImageLayout::eTransferDstOptimal:
      // Image will be used as a transfer destination
      // Make sure any writes to the image have been finished
      barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;
      break;

    case vk::ImageLayout::eTransferSrcOptimal:
      // Image will be used as a transfer source
      // Make sure any reads from the image have been finished
      barrier.dstAccessMask = vk::AccessFlagBits::eTransferRead;
      break;

    case vk::ImageLayout::eColorAttachmentOptimal:
      // Image will be used as a color attachment
      // Make sure any writes to the color buffer have been finished
      barrier.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
      break;

    case vk::ImageLayout::eDepthStencilAttachmentOptimal:
      // Image layout will be used as a depth/stencil attachment
      // Make sure any writes to depth/stencil buffer have been finished
      barrier.dstAccessMask = barrier.dstAccessMask | 
        vk::AccessFlagBits::eDepthStencilAttachmentWrite;
      break;

    case vk::ImageLayout::eShaderReadOnlyOptimal:
      // Image will be read in a shader (sampler, input attachment)
      // Make sure any writes to the image have been finished
      if ( barrier.srcAccessMask == vk::AccessFlags( ) )
      {
        barrier.srcAccessMask = vk::AccessFlagBits::eHostWrite | 
          vk::AccessFlagBits::eTransferWrite;
      }
      barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;
      break;
    default:
      // Other source layouts aren't handled (yet)
      break;
    }
    static_cast< vk::CommandBuffer >( *cmd ).pipelineBarrier(
      srcStageMask,
      dstStageMask,
      {},
      {},
      {},
      barrier
    );
  }
}