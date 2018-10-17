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

#include "utilities.hpp"
#include "Log.h"

#include <sstream>
#include <fstream>

#define STB_IMAGE_IMPLEMENTATION
#include <stbi/stb_image.h>

namespace pompeii
{
   short utilities::channelsFromFormat( const vk::Format& )
   {
     short numChannels = 4;
     // TODO: https://github.com/googlesamples/vulkan-basic-samples/blob/master/layers/vk_format_utils.cpp
     /*switch ( format )
     {
     case vk::Format::eR8G8B8A8Uint:
     case vk::Format::eR8G8B8A8Sint:
     case vk::Format::eR8G8B8A8Srgb:
     case vk::Format::eR8G8B8A8Unorm:
     case vk::Format::eR8G8B8A8Sscaled:
     case vk::Format::eR16G16B16A16Uint:
     case vk::Format::eR16G16B16A16Sint:
     case vk::Format::eR16G16B16A16Sfloat:
     case vk::Format::eR32G32B32A32Uint:
     case vk::Format::eR32G32B32A32Sint:
     case vk::Format::eR32G32B32A32Sfloat:
     case vk::Format::eR4G4B4A4UnormPack16:
     case vk::Format::eB4G4R4A4UnormPack16:
     case vk::Format::eR5G5B5A1UnormPack16:
     case vk::Format::eB5G5R5A1UnormPack16:
     case vk::Format::eA1R5G5B5UnormPack16:
       numChannels = 4;
       break;
     case vk::Format::eR8G8B8Uint:
     case vk::Format::eR8G8B8Sint:
     case vk::Format::eR8G8B8Srgb:
     case vk::Format::eR8G8B8Unorm:
     case vk::Format::eR8G8B8Sscaled:
     case vk::Format::eR16G16B16Uint:
     case vk::Format::eR16G16B16Sint:
     case vk::Format::eR16G16B16Sfloat:
     case vk::Format::eR32G32B32Uint:
     case vk::Format::eR32G32B32Sint:
     case vk::Format::eR32G32B32Sfloat:
     case vk::Format::eR5G6B5UnormPack16:
     case vk::Format::eB5G6R5UnormPack16:
     case vk::Format::eR8G8B8Uscaled:
     case vk::Format::eB8G8R8Unorm:
     case vk::Format::eB8G8R8Snorm:
     case vk::Format::eR8G8B8Snorm:
     case vk::Format::eB8G8R8Uscaled:
     case vk::Format::eB8G8R8Uint:
     case vk::Format::eB8G8R8Sint:
     case vk::Format::eB8G8R8Sscaled:
       numChannels = 3;
       break;
     case vk::Format::eR8G8Uint:
     case vk::Format::eR8G8Sint:
     case vk::Format::eR8G8Srgb:
     case vk::Format::eR8G8Unorm:
     case vk::Format::eR8G8Sscaled:
     case vk::Format::eR16G16Uint:
     case vk::Format::eR16G16Sint:
     case vk::Format::eR16G16Sfloat:
     case vk::Format::eR32G32Uint:
     case vk::Format::eR32G32Sint:
     case vk::Format::eR32G32Sfloat:
     case vk::Format::eR4G4UnormPack8:
     case vk::Format::eR8G8Snorm:
     case vk::Format::eR8G8Uscaled:
       numChannels = 2;
       break;
     case vk::Format::eR8Uint:
     case vk::Format::eR8Sint:
     case vk::Format::eR8Snorm:
     case vk::Format::eR8Srgb:
     case vk::Format::eR8Unorm:
     case vk::Format::eR8Sscaled:
     case vk::Format::eR16Uint:
     case vk::Format::eR16Sint:
     case vk::Format::eR16Sfloat:
     case vk::Format::eR32Uint:
     case vk::Format::eR32Sint:
     case vk::Format::eR32Sfloat:
     case vk::Format::eR8Uscaled:
       numChannels = 1;
       break;
    case vk::Format::eUndefined:
      throw;
     }*/
     return numChannels;
   }
   void utilities::saveScreenshot( std::shared_ptr<Device> device,
    const char * filename, uint32_t width, uint32_t height, 
    vk::Format colorFormat, std::shared_ptr<Image> srcImage,
    std::shared_ptr<CommandPool> cmdPool, std::shared_ptr<Queue> queue )
  {
    // Get format properties for the swapchain color format
    vk::FormatProperties formatProps;
    bool supportsBlit = true;
    // Check blit support for source and destination

    // Check if the device supports blitting from optimal images 
    //      (the swapchain images are in optimal format)
    formatProps = device->getPhysicalDevice( )->getFormatProperties( colorFormat );
    if ( !( formatProps.optimalTilingFeatures & 
      vk::FormatFeatureFlagBits::eBlitSrc ) )
    {
      Log::error( "Device doesn't support blitting to linear tiled images, "
        "using copy instead of blit!" );
      supportsBlit = false;
    }

    // Check if the device supports blitting to linear images
    formatProps = device->getPhysicalDevice( )->getFormatProperties( 
      vk::Format::eR8G8B8A8Unorm );
    if ( !( formatProps.linearTilingFeatures &
      vk::FormatFeatureFlagBits::eBlitDst ) )
    {
      Log::error( "Device doesn't support blitting to linear tiled images, "
        "using copy instead of blit!" );
      supportsBlit = false;
    }

    // Create the linear tiled destination image to copy to and to read 
    //    the memory from
    std::shared_ptr<pompeii::Image> dstImage = device->createImage( {},
      vk::ImageType::e2D, vk::Format::eR8G8B8A8Unorm,
      vk::Extent3D( width, height, 1 ), 1, 1, vk::SampleCountFlagBits::e1,
      vk::ImageTiling::eLinear, vk::ImageUsageFlagBits::eTransferDst,
      vk::SharingMode::eExclusive, {}, vk::ImageLayout::eUndefined,
      vk::MemoryPropertyFlagBits::eHostVisible | 
      vk::MemoryPropertyFlagBits::eHostCoherent
    );

    // Do the actual blit from the swapchain image to our host visible 
    //      destination image
    auto copyCmd = cmdPool->allocateCommandBuffer( );

    copyCmd->begin( );

    // Transition destination image to transfer destination layout
    pompeii::utilities::insertImageMemoryBarrier( copyCmd, dstImage, { },
      vk::AccessFlagBits::eTransferWrite,
      vk::ImageLayout::eUndefined,
      vk::ImageLayout::eTransferDstOptimal,
      vk::PipelineStageFlagBits::eTransfer,
      vk::PipelineStageFlagBits::eTransfer,
      vk::ImageSubresourceRange(
        vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1
      )
    );
    // Transition swapchain image from present to transfer source layout
    pompeii::utilities::insertImageMemoryBarrier( copyCmd, srcImage,
      vk::AccessFlagBits::eMemoryRead,
      vk::AccessFlagBits::eTransferRead,
      vk::ImageLayout::ePresentSrcKHR,
      vk::ImageLayout::eTransferSrcOptimal,
      vk::PipelineStageFlagBits::eTransfer,
      vk::PipelineStageFlagBits::eTransfer,
      vk::ImageSubresourceRange(
        vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1
      )
    );

    // If source and destination support blit we'll blit as this also does 
    //    automatic format conversion (e.g. from BGR to RGB)
    if ( supportsBlit )
    {
      // Define the region to blit (we will blit the whole swapchain image)
      vk::Offset3D blitSize;
      blitSize.x = width;
      blitSize.y = height;
      blitSize.z = 1;
      vk::ImageBlit imageBlitRegion{};
      imageBlitRegion.srcSubresource.aspectMask = 
        vk::ImageAspectFlagBits::eColor;
      imageBlitRegion.srcSubresource.layerCount = 1;
      imageBlitRegion.srcOffsets[ 1 ] = blitSize;
      imageBlitRegion.dstSubresource.aspectMask = 
        vk::ImageAspectFlagBits::eColor;
      imageBlitRegion.dstSubresource.layerCount = 1;
      imageBlitRegion.dstOffsets[ 1 ] = blitSize;

      // Issue the blit command
      copyCmd->blitImage(
        srcImage, vk::ImageLayout::eTransferSrcOptimal,
        dstImage, vk::ImageLayout::eTransferDstOptimal,
        { imageBlitRegion }, vk::Filter::eNearest
      );
    }
    else
    {
      // Otherwise use image copy (requires us to manually flip components)
      vk::ImageCopy imageCopyRegion{};
      imageCopyRegion.srcSubresource.aspectMask = 
        vk::ImageAspectFlagBits::eColor;
      imageCopyRegion.srcSubresource.layerCount = 1;
      imageCopyRegion.dstSubresource.aspectMask = 
        vk::ImageAspectFlagBits::eColor;
      imageCopyRegion.dstSubresource.layerCount = 1;
      imageCopyRegion.extent.width = width;
      imageCopyRegion.extent.height = height;
      imageCopyRegion.extent.depth = 1;

      // Issue the copy command
      copyCmd->copyImage(
        srcImage, vk::ImageLayout::eTransferSrcOptimal,
        dstImage, vk::ImageLayout::eTransferDstOptimal,
        { imageCopyRegion }
      );
    }

    // Transition destination image to general layout, which is the required 
    //    layout for mapping the image memory later on
    insertImageMemoryBarrier(
      copyCmd,
      dstImage,
      vk::AccessFlagBits::eTransferWrite,
      vk::AccessFlagBits::eMemoryRead,
      vk::ImageLayout::eTransferDstOptimal,
      vk::ImageLayout::eGeneral,
      vk::PipelineStageFlagBits::eTransfer,
      vk::PipelineStageFlagBits::eTransfer,
      vk::ImageSubresourceRange(
        vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1
      )
    );

    // Transition back the swap chain image after the blit is done
    insertImageMemoryBarrier(
      copyCmd,
      srcImage,
      vk::AccessFlagBits::eTransferRead,
      vk::AccessFlagBits::eMemoryRead,
      vk::ImageLayout::eTransferSrcOptimal,
      vk::ImageLayout::ePresentSrcKHR,
      vk::PipelineStageFlagBits::eTransfer,
      vk::PipelineStageFlagBits::eTransfer,
      vk::ImageSubresourceRange(
        vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1
      )
    );

    copyCmd->end( );

    queue->submitAndWait( copyCmd );

    // Get layout of the image (including row pitch)
    vk::ImageSubresource isr;
    isr.aspectMask = vk::ImageAspectFlagBits::eColor;
    vk::SubresourceLayout subResourceLayout;

    vk::Device dev = static_cast< vk::Device > ( *device );

    dev.getImageSubresourceLayout(
      static_cast< vk::Image >( *dstImage ), &isr, &subResourceLayout
    );

    // Map image memory so we can start copying from it
    const char* data = ( const char* ) dev.mapMemory( dstImage->imageMemory, 0, 
      VK_WHOLE_SIZE, {} );
    data += subResourceLayout.offset;

    std::ofstream file( filename, std::ios::out | std::ios::binary );

    // ppm header
    file << "P6\n" << width << "\n" << height << "\n" << 255 << "\n";

    // If source is BGR (destination is always RGB) and we can't use blit 
    //    (which does automatic conversion), we'll have to manually swizzle 
    //    color components
    bool colorSwizzle = false;
    // Check if source is BGR 
    // TODO: Not complete, only contains most common BGR surface formats
    if ( !supportsBlit )
    {
      std::vector< vk::Format > formatsBGR = {
        vk::Format::eB8G8R8A8Srgb, vk::Format::eB8G8R8A8Unorm,
        vk::Format::eB8G8R8A8Snorm
      };
      colorSwizzle = ( std::find( formatsBGR.begin( ), formatsBGR.end( ),
        colorFormat ) != formatsBGR.end( ) );
    }

    // ppm binary pixel data
    for ( uint32_t y = 0; y < height; ++y )
    {
      unsigned int* row = ( unsigned int* ) data;
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
        ++row;
      }
      data += subResourceLayout.rowPitch;
    }
    file.close( );

    Log::info( "Screenshot saved to disk" );
  }

  /*void utilities::saveToPPM( const char* file, uint32_t width, uint32_t height, 
    const std::shared_ptr< pompeii::Image >& image, vk::ImageAspectFlags aspect )
  {
    vk::ImageSubresource isr;
    isr.aspectMask = aspect;
    vk::SubresourceLayout srl;


  }*/
  
  unsigned char* utilities::loadImageTexture( const std::string& fileName,
    uint32_t& width, uint32_t& height, uint32_t& numChannels )
  {
    int w, h, c;
    stbi_uc* pixels = stbi_load( fileName.c_str( ), &w, &h, &c, STBI_rgb_alpha );

    if ( pixels == nullptr )
    {
      const std::string reason( stbi_failure_reason( ) );
      Log::error( "%s failed to load. stb_image's reason: %s\n", fileName.c_str( ), 
        reason.c_str( ) );

      throw std::runtime_error( "failed to load texture image!" );
    }

    width = w;
    height = h;
    numChannels = c;

    return pixels;
  }
  
  std::vector<char> utilities::readBinaryile( const std::string& fileName )
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
	
  const std::string utilities::translateVulkanResult( vk::Result res )
	{
    VkResult result = VkResult( res );
    switch ( result ) {
      // Success codes
    case VK_SUCCESS:
      return std::string( "Command successfully completed." );
    case VK_NOT_READY:
      return std::string( "A fence or query has not yet completed." );
    case VK_TIMEOUT:
      return std::string( "A wait operation has not completed in the" ) + 
        std::string( " specified time." );
    case VK_EVENT_SET:
      return std::string( "An event is signaled." );
    case VK_EVENT_RESET:
      return std::string( "An event is unsignaled." );
    case VK_INCOMPLETE:
      return std::string( "A return array was too small for the result." );
    case VK_SUBOPTIMAL_KHR:
      return std::string( "A swapchain no longer matches the surface" ) + 
        std::string( "properties exactly, but can still be used to present" ) +
        std::string( " to the surface successfully." );

      // Error codes
    case VK_ERROR_OUT_OF_HOST_MEMORY:
      return std::string( "A host memory allocation has failed." );
    case VK_ERROR_OUT_OF_DEVICE_MEMORY:
      return std::string( "A device memory allocation has failed." );
    case VK_ERROR_INITIALIZATION_FAILED:
      return std::string( "Initialization of an object could not be" ) +
        std::string( " completed for implementation-specific reasons." );
    case VK_ERROR_DEVICE_LOST:
      return std::string( "The logical or physical device has been lost." );
    case VK_ERROR_MEMORY_MAP_FAILED:
      return std::string( "Mapping of a memory object has failed." );
    case VK_ERROR_LAYER_NOT_PRESENT:
      return std::string( "A requested layer is not present or could not be" ) +
        std::string( " loaded." );
    case VK_ERROR_EXTENSION_NOT_PRESENT:
      return std::string( "A requested extension is not supported." );
    case VK_ERROR_FEATURE_NOT_PRESENT:
      return std::string( "A requested feature is not supported." );
    case VK_ERROR_INCOMPATIBLE_DRIVER:
      return std::string( "The requested version of Vulkan is not supported" ) +
        std::string( " by the driver or is otherwise incompatible for" ) +
        std::string( " implementation-specific reasons." );
    case VK_ERROR_TOO_MANY_OBJECTS:
      return std::string( "Too many objects of the type have already been" ) +
        std::string( " created." );
    case VK_ERROR_FORMAT_NOT_SUPPORTED:
      return std::string( "A requested format is not supported on this device." );
    case VK_ERROR_SURFACE_LOST_KHR:
      return std::string( "A surface is no longer available." );
    case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR:
      return std::string( "The requested window is already connected to a" ) +
        std::string( " VkSurfaceKHR, or to some other non-Vulkan API." );
    case VK_ERROR_OUT_OF_DATE_KHR:
      return std::string( "A surface has changed in such a way that it is" ) +
        std::string( " no longer compatible with the swapchain, and further" ) +
        std::string( " presentation requests using the swapchain will fail." ) +
        std::string( " Applications must query the new surface properties" ) +
        std::string( " and recreate their swapchain if they wish to" ) +
        std::string( " continue presenting to the surface." );
    case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR:
      return std::string( "The display used by a swapchain doesn't use" ) +
        std::string( " the same presentable image layout, or is" ) +
        std::string( " incompatible in a way that prevents sharing an image." );
    case VK_ERROR_VALIDATION_FAILED_EXT:
      return std::string( "A validation layer found an error." );
    default:
      std::stringstream ss;
      ss << "Unknown [" << uint32_t( result ) << "]";
      return ss.str( );
    }
	}
  
  void utilities::transitionImageLayout( const std::shared_ptr<CommandBuffer>& cmd,
    std::shared_ptr<Image> image,
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
    transitionImageLayout( cmd, image, oldImageLayout, newImageLayout, 
      subresourceRange, srcStageMask, dstStageMask );
  }
  void utilities::insertImageMemoryBarrier( 
    const std::shared_ptr<CommandBuffer> cmdbuffer, 
    std::shared_ptr<Image> image, 
    vk::AccessFlags srcAccessMask, vk::AccessFlags dstAccessMask, 
    vk::ImageLayout oldImageLayout, vk::ImageLayout newImageLayout, 
    vk::PipelineStageFlags srcStageMask, vk::PipelineStageFlags dstStageMask, 
    vk::ImageSubresourceRange subresourceRange )
  {
    pompeii::ImageMemoryBarrier imr( 
      srcAccessMask, dstAccessMask, 
      oldImageLayout, newImageLayout, 
      0, 0, image, subresourceRange
    );
    cmdbuffer->pipelineBarrier( srcStageMask, dstStageMask, {}, {}, {}, imr );
  }

  void utilities::transitionImageLayout( const std::shared_ptr<CommandBuffer>& cmd,
    std::shared_ptr<Image> image,
    vk::ImageLayout oldImageLayout,
    vk::ImageLayout newImageLayout,
    vk::ImageSubresourceRange subresourceRange,
    vk::PipelineStageFlags srcStageMask,
    vk::PipelineStageFlags dstStageMask )
  {
    vk::AccessFlags srcAccessMask, dstAccessMask;

    // Source layouts (old)
    // Source access mask controls actions that have to be finished
    //  on the old layout before it will be transitioned to the new layout
    switch ( oldImageLayout )
    {
      case vk::ImageLayout::eUndefined:
        // Image layout is undefined (or doesn't matter)
        // Only valid as initial layout
        // No flags required, listed only for completeness
        srcAccessMask = { };
        break;

      case vk::ImageLayout::ePreinitialized:
        // Image is preinitialized
        // Only valid as initial layout for linear images, preserves memory contents
        // Make sure host writes have been finished
        srcAccessMask = vk::AccessFlagBits::eHostWrite;
        break;

      case vk::ImageLayout::eColorAttachmentOptimal:
        // Image is a color attachment
        // Make sure any writes to the color buffer have been finished
        srcAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
        break;

      case vk::ImageLayout::eDepthStencilAttachmentOptimal:
        // Image is a depth/stencil attachment
        // Make sure any writes to the depth/stencil buffer have been finished
        srcAccessMask = vk::AccessFlagBits::eDepthStencilAttachmentWrite;
        break;

      case vk::ImageLayout::eTransferSrcOptimal:
        // Image is a transfer source 
        // Make sure any reads from the image have been finished
        srcAccessMask = vk::AccessFlagBits::eTransferRead;
        break;

      case vk::ImageLayout::eTransferDstOptimal:
        // Image is a transfer destination
        // Make sure any writes to the image have been finished
        srcAccessMask = vk::AccessFlagBits::eTransferWrite;
        break;

      case vk::ImageLayout::eShaderReadOnlyOptimal:
        // Image is read by a shader
        // Make sure any shader reads from the image have been finished
        srcAccessMask = vk::AccessFlagBits::eShaderRead;
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
        dstAccessMask = vk::AccessFlagBits::eTransferWrite;
        break;

      case vk::ImageLayout::eTransferSrcOptimal:
        // Image will be used as a transfer source
        // Make sure any reads from the image have been finished
        dstAccessMask = vk::AccessFlagBits::eTransferRead;
        break;

      case vk::ImageLayout::eColorAttachmentOptimal:
        // Image will be used as a color attachment
        // Make sure any writes to the color buffer have been finished
        dstAccessMask = 
          vk::AccessFlagBits::eColorAttachmentWrite;
        break;

      case vk::ImageLayout::eDepthStencilAttachmentOptimal:
        // Image layout will be used as a depth/stencil attachment
        // Make sure any writes to depth/stencil buffer have been finished
        dstAccessMask = dstAccessMask |
          vk::AccessFlagBits::eDepthStencilAttachmentWrite;
        break;

      case vk::ImageLayout::eShaderReadOnlyOptimal:
        // Image will be read in a shader (sampler, input attachment)
        // Make sure any writes to the image have been finished
        if ( srcAccessMask == vk::AccessFlags( ) )
        {
          srcAccessMask = vk::AccessFlagBits::eHostWrite |
            vk::AccessFlagBits::eTransferWrite;
        }
        dstAccessMask = vk::AccessFlagBits::eShaderRead;
        break;
      default:
        // Other source layouts aren't handled (yet)
        break;
    }

    // Create an image barrier object
    ImageMemoryBarrier imageMemoryBarrier(
      srcAccessMask, dstAccessMask,
      oldImageLayout, newImageLayout,
      VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED,
      image, subresourceRange );

    cmd->pipelineBarrier( 
      srcStageMask, dstStageMask, { }, { }, { },
      imageMemoryBarrier );
  }
}