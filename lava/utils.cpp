#include "utils.hpp"

#include <sstream>
#include <fstream>

#define STB_IMAGE_IMPLEMENTATION
#include <stbi/stb_image.h>

#include "Device.h"
#include "PhysicalDevice.h"

namespace lava
{
  void utils::saveToImage( const std::string & filename, vk::Format colorFormat, 
    std::shared_ptr<Device> dev, std::shared_ptr<Image> currentImage,
    uint32_t width, uint32_t height, std::shared_ptr<CommandPool> cmdPool,
    std::shared_ptr<Queue> queue )
  {
    bool supportBlit = true;

    vk::FormatProperties formatProps = dev->_physicalDevice->getFormatProperties( colorFormat );
    // Check if the device supports blitting from optimal images (the swapchain images are in optimal format)
    if ( !( formatProps.optimalTilingFeatures & vk::FormatFeatureFlagBits::eBlitSrc ) )
    {
      supportBlit = false;
    }
    // Check if the device supports blitting to linear images 
    formatProps = dev->_physicalDevice->getFormatProperties( vk::Format::eR8G8B8A8Snorm );
    if ( !( formatProps.optimalTilingFeatures & vk::FormatFeatureFlagBits::eBlitDst ) )
    {
      supportBlit = false;
    }

    // Source for the copy in the last renderer swapchain image
    std::shared_ptr<Image> srcImage = currentImage;

    std::shared_ptr<Image> dstImage = dev->createImage( { },
      vk::ImageType::e2D, vk::Format::eR8G8B8A8Unorm, { width, height, 1 }, 1, 1,
      vk::SampleCountFlagBits::e1, vk::ImageTiling::eLinear, 
      vk::ImageUsageFlagBits::eTransferDst, vk::SharingMode::eExclusive, { }, 
      vk::ImageLayout::eUndefined, vk::MemoryPropertyFlagBits::eHostVisible | 
      vk::MemoryPropertyFlagBits::eHostCoherent
    );  // create, allocate + bind

    std::shared_ptr<CommandBuffer> copyCmd = cmdPool->allocateCommandBuffer( );

    copyCmd->beginSimple( );

    // Transition destination image to transfer destination layout
    copyCmd->pipelineBarrier( vk::PipelineStageFlagBits::eTransfer,
      vk::PipelineStageFlagBits::eTransfer, {}, {}, {}, {
        lava::ImageMemoryBarrier(
          ( vk::AccessFlagBits )0, vk::AccessFlagBits::eTransferWrite,
          vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal,
          0, 0, dstImage,
          vk::ImageSubresourceRange( vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 )
        )
      }
    );

    // Transition swapchain image from present to transfer source layout
    copyCmd->pipelineBarrier( vk::PipelineStageFlagBits::eTransfer,
      vk::PipelineStageFlagBits::eTransfer, {}, {}, {}, {
        lava::ImageMemoryBarrier(
          vk::AccessFlagBits::eMemoryRead, vk::AccessFlagBits::eTransferRead,
          vk::ImageLayout::ePresentSrcKHR, vk::ImageLayout::eTransferSrcOptimal,
          0, 0, srcImage,
          vk::ImageSubresourceRange( vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 )
        )
      }
    );

    // If source and destination support blit we'll blit as this also does automatic format conversion (e.g. from BGR to RGB)
    if ( supportBlit )
    {
      // Define the region to blit (we will blit the whole swapchain image)
      vk::Offset3D blitSize( width, height, 1 );
      vk::ImageBlit imageBlitRegion;
      imageBlitRegion.srcSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
      imageBlitRegion.srcSubresource.layerCount = 1;
      imageBlitRegion.srcOffsets[ 1 ] = blitSize;
      imageBlitRegion.dstSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
      imageBlitRegion.dstSubresource.layerCount = 1;
      imageBlitRegion.dstOffsets[ 1 ] = blitSize;

      // Issue the blit command
      copyCmd->blitImage( 
        srcImage, vk::ImageLayout::eTransferSrcOptimal, 
        dstImage, vk::ImageLayout::eTransferDstOptimal, 
        imageBlitRegion, vk::Filter::eNearest
      );
    }
    else
    {
      // Otherwise use image copy (requires us to manually flip components)
      vk::ImageCopy imageCopyRegion;
      imageCopyRegion.srcSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
      imageCopyRegion.srcSubresource.layerCount = 1;
      imageCopyRegion.dstSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
      imageCopyRegion.dstSubresource.layerCount = 1;
      imageCopyRegion.extent.width = width;
      imageCopyRegion.extent.height = height;
      imageCopyRegion.extent.depth = 1;

      // Issue the copy command
      copyCmd->copyImage(
        srcImage, vk::ImageLayout::eTransferSrcOptimal,
        dstImage, vk::ImageLayout::eTransferDstOptimal,
        imageCopyRegion
      );
    }

    // Transition destination image to general layout, which is the required layout for mapping the image memory later on
    copyCmd->pipelineBarrier( vk::PipelineStageFlagBits::eTransfer,
      vk::PipelineStageFlagBits::eTransfer, {}, {}, {}, {
        lava::ImageMemoryBarrier(
          vk::AccessFlagBits::eTransferWrite, vk::AccessFlagBits::eMemoryRead,
          vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eGeneral,
          0, 0, dstImage,
          vk::ImageSubresourceRange( vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 )
        )
      }
    );

    // Transition back the swap chain image after the blit is done
    copyCmd->pipelineBarrier( vk::PipelineStageFlagBits::eTransfer,
      vk::PipelineStageFlagBits::eTransfer, {}, {}, {}, {
        lava::ImageMemoryBarrier(
          vk::AccessFlagBits::eTransferRead, vk::AccessFlagBits::eMemoryRead,
          vk::ImageLayout::eTransferSrcOptimal, vk::ImageLayout::ePresentSrcKHR,
          0, 0, srcImage,
          vk::ImageSubresourceRange( vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 )
        )
      }
    );
    // Send command buffer
    copyCmd->end( );

    queue->submitAndWait( copyCmd );

    // Get layout of the image (including row pitch)
    vk::ImageSubresource subResource;
    subResource.aspectMask = vk::ImageAspectFlagBits::eColor;
    vk::SubresourceLayout subResourceLayout;

    static_cast< vk::Device >( *dev ).getImageSubresourceLayout( 
      static_cast< vk::Image >( *dstImage ), subResource );
    const char* data;
    static_cast< vk::Device >( *dev ).mapMemory( 
      dstImage->imageMemory, 0, VK_WHOLE_SIZE, { }, ( void** ) &data );
    data += subResourceLayout.offset;

    std::ofstream file( filename, std::ios::out | std::ios::binary );


    // ppm header
    file << "P6\n" << width << "\n" << height << "\n" << 255 << "\n";

    // If source is BGR (destination is always RGB) and we can't use blit (which does automatic conversion), we'll have to manually swizzle color components
    bool colorSwizzle = false;
    // Check if source is BGR 
    // Note: Not complete, only contains most common and basic BGR surface formats for demonstation purposes
    if ( !supportBlit )
    {
      std::vector<vk::Format> formatsBGR = { vk::Format::eB8G8R8A8Srgb, vk::Format::eB8G8R8A8Unorm, vk::Format::eB8G8R8A8Snorm };
      colorSwizzle = ( std::find( formatsBGR.begin( ), formatsBGR.end( ), colorFormat ) != formatsBGR.end( ) );
    }

    // ppm binary pixel data
    for ( uint32_t y = 0; y < height; ++y )
    {
      unsigned int *row = ( unsigned int* ) data;
      for ( uint32_t x = 0; x < width; ++x )
      {
        if ( colorSwizzle )
        {
          file.write( ( char* ) row + 2, 1 );
          file.write( ( char* ) row + 1, 1 );
          file.write( ( char* ) row, 1 );
        }
        else
        {
          file.write( ( char* ) row, 3 );
        }
        row++;
      }
      data += subResourceLayout.rowPitch;
    }
    file.close( );

    std::cout << "Screenshot saved to disk" << std::endl;

    // Clean up resources
    // TODO: vkUnmapMemory( device, dstImageMemory );
    // TODO: vkFreeMemory( device, dstImageMemory, nullptr );
    // TODO: vkDestroyImage( device, dstImage, nullptr );
  }
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
    // Create an image barrier object
    vk::ImageMemoryBarrier imageMemoryBarrier;
    imageMemoryBarrier.oldLayout = oldImageLayout;
    imageMemoryBarrier.newLayout = newImageLayout;
    imageMemoryBarrier.image = image;
    imageMemoryBarrier.subresourceRange = subresourceRange;

    // Source layouts (old)
    // Source access mask controls actions that have to be finished on the old layout
    // before it will be transitioned to the new layout
    switch ( oldImageLayout )
    {
      case vk::ImageLayout::eUndefined:
        // Image layout is undefined (or does not matter)
        // Only valid as initial layout
        // No flags required, listed only for completeness
        imageMemoryBarrier.srcAccessMask = vk::AccessFlags( );
        break;

      case vk::ImageLayout::ePreinitialized:
        // Image is preinitialized
        // Only valid as initial layout for linear images, preserves memory contents
        // Make sure host writes have been finished
        imageMemoryBarrier.srcAccessMask = vk::AccessFlagBits::eHostWrite;
        break;

      case vk::ImageLayout::eColorAttachmentOptimal:
        // Image is a color attachment
        // Make sure any writes to the color buffer have been finished
        imageMemoryBarrier.srcAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
        break;

      case vk::ImageLayout::eDepthStencilAttachmentOptimal:
        // Image is a depth/stencil attachment
        // Make sure any writes to the depth/stencil buffer have been finished
        imageMemoryBarrier.srcAccessMask = vk::AccessFlagBits::eDepthStencilAttachmentWrite;
        break;

      case vk::ImageLayout::eTransferSrcOptimal:
        // Image is a transfer source 
        // Make sure any reads from the image have been finished
        imageMemoryBarrier.srcAccessMask = vk::AccessFlagBits::eTransferRead;
        break;

      case vk::ImageLayout::eTransferDstOptimal:
        // Image is a transfer destination
        // Make sure any writes to the image have been finished
        imageMemoryBarrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
        break;

      case vk::ImageLayout::eShaderReadOnlyOptimal:
        // Image is read by a shader
        // Make sure any shader reads from the image have been finished
        imageMemoryBarrier.srcAccessMask = vk::AccessFlagBits::eShaderRead;
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
        imageMemoryBarrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;
        break;

      case vk::ImageLayout::eTransferSrcOptimal:
        // Image will be used as a transfer source
        // Make sure any reads from the image have been finished
        imageMemoryBarrier.dstAccessMask = vk::AccessFlagBits::eTransferRead;
        break;

      case vk::ImageLayout::eColorAttachmentOptimal:
        // Image will be used as a color attachment
        // Make sure any writes to the color buffer have been finished
        imageMemoryBarrier.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
        break;

      case vk::ImageLayout::eDepthStencilAttachmentOptimal:
        // Image layout will be used as a depth/stencil attachment
        // Make sure any writes to depth/stencil buffer have been finished
        imageMemoryBarrier.dstAccessMask = imageMemoryBarrier.dstAccessMask |
          vk::AccessFlagBits::eDepthStencilAttachmentWrite;
        break;

      case vk::ImageLayout::eShaderReadOnlyOptimal:
        // Image will be read in a shader (sampler, input attachment)
        // Make sure any writes to the image have been finished
        if ( imageMemoryBarrier.srcAccessMask == vk::AccessFlags( ) )
        {
          imageMemoryBarrier.srcAccessMask = vk::AccessFlagBits::eHostWrite |
            vk::AccessFlagBits::eTransferWrite;
        }
        imageMemoryBarrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;
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
      imageMemoryBarrier
    );
  }
}