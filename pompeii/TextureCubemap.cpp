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

#include "TextureCubemap.h"

#include <pompeii/Buffer.h>
#include <pompeii/PhysicalDevice.h>

#include "utilities.hpp"

namespace pompeii
{
  TextureCubemap::TextureCubemap( const std::shared_ptr<Device>& device_, 
      const std::array< std::string, 6 >& filePaths,
      const std::shared_ptr<CommandPool>& cmdPool,
      const std::shared_ptr<Queue>& queue, vk::Format format,
      vk::ImageUsageFlags imageUsageFlags,
      vk::ImageLayout imageLayout_, bool forceLinear )
    : Texture( device_ )
  {
    uint32_t textureWidth = 0;
    uint32_t textureHeight = 0;
    uint32_t channels = 0;

    struct Image_
    {
      unsigned char* pixels;
      uint32_t width;
      uint32_t height;
      uint32_t channels;
      uint32_t size;
    };

    std::vector<Image_> images;
    uint32_t totalSize = 0;
    images.reserve( filePaths.size( ) );
    for ( uint32_t i = 0, l = filePaths.size( ); i < l; ++i )
    {
      unsigned char* pixels = pompeii::utilities::loadImageTexture(
        filePaths[ i ].c_str( ), textureWidth, textureHeight, channels);

      // The load function returns the original channel count, 
      // but it was forced to 4 because of the last parameter
      channels = 4;

      uint32_t size = textureWidth * textureHeight * channels 
        * sizeof( unsigned char );
      images.push_back( { pixels, textureWidth, textureHeight, channels, size } );
      totalSize += size;

      auto deviceProps = _device->getPhysicalDevice( )->getDeviceProperties( );
      if ( static_cast< uint32_t >( deviceProps.limits.maxImageDimensionCube ) < textureWidth ||
        static_cast< uint32_t >( deviceProps.limits.maxImageDimensionCube ) < textureHeight )
      {
        printf( "%s is too big (%dx%d), max supported size is %dx%d.\n", 
          filePaths[ i ].c_str( ), textureWidth, textureHeight,
          deviceProps.limits.maxImageDimensionCube, 
          deviceProps.limits.maxImageDimensionCube
        );
        textureWidth = deviceProps.limits.maxImageDimensionCube;
        textureHeight = deviceProps.limits.maxImageDimensionCube;
      }
    }

    unsigned char* pixels = ( unsigned char* ) malloc( totalSize );
    unsigned char* pixelData = pixels;
    for ( uint32_t i = 0, l = images.size( ); i < l; ++i )
    {
      memcpy( pixelData, images[ i ].pixels, images[ i ].size );
      pixelData += ( images[ i ].size / sizeof( unsigned char ) );
    }
    for ( uint32_t i = 0, l = images.size( ); i < l; ++i )
    {
      free( images[ i ].pixels );
    }

    width = textureWidth;
    height = textureHeight;

    vk::FormatProperties formatProps =
      _device->getPhysicalDevice( )->getFormatProperties( format );

    // Only use linear tiling if requested (and supported by the device)
    // Support for linear tiling is mostly limited, so prefer to use
    // optimal tiling instead
    // On most implementations linear tiling will only support a very
    // limited amount of formats and features (mip maps, cubemaps, arrays, etc.)
    VkBool32 useStaging = !forceLinear;

    if ( useStaging )
    {
      // Create a host-visible staging buffer that contains the raw image data
      std::shared_ptr<Buffer> stagingBuffer = _device->createBuffer( totalSize,
        vk::BufferUsageFlagBits::eTransferSrc, vk::SharingMode::eExclusive, { },
        vk::MemoryPropertyFlagBits::eHostVisible |
        vk::MemoryPropertyFlagBits::eHostCoherent );
      stagingBuffer->writeData( 0, totalSize, pixels );

      free( pixels );

      // TODO: Generate MipLevels
      mipLevels = 1;

      // Create Image
      auto usageFlags = imageUsageFlags;

      // Ensure that the TRANSFER_DST bit is set for staging
      if ( !( usageFlags & vk::ImageUsageFlagBits::eTransferDst ) )
      {
        usageFlags |= vk::ImageUsageFlagBits::eTransferDst;
      }

      image = _device->createImage(
        // This flag is required for cube map images
        vk::ImageCreateFlagBits::eCubeCompatible,
        vk::ImageType::e2D, format,
        vk::Extent3D( textureWidth, textureHeight, 1 ), mipLevels, 
        // Cube faces count as array layers in Vulkan
        6,
        vk::SampleCountFlagBits::e1, vk::ImageTiling::eOptimal, usageFlags,
        vk::SharingMode::eExclusive, { }, vk::ImageLayout::eUndefined,
        vk::MemoryPropertyFlagBits::eDeviceLocal );

      auto copyCmd = cmdPool->allocateCommandBuffer( );
      copyCmd->begin( vk::CommandBufferUsageFlagBits::eOneTimeSubmit );

      // Setup buffer copy regions for each face including all of it's miplevels
      std::vector<vk::BufferImageCopy> bufferCopyRegions;
      uint32_t offset = 0;
      for ( uint32_t face = 0; face < 6; ++face )
      {
        for ( uint32_t mipLevel = 0; mipLevel < 1; ++mipLevel )
        {
          vk::BufferImageCopy bic;
          bic.imageSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
          bic.imageSubresource.mipLevel = mipLevel;
          bic.imageSubresource.baseArrayLayer = face;
          bic.imageSubresource.layerCount = 1;
          bic.imageExtent.width = textureWidth;
          bic.imageExtent.height = textureHeight;
          bic.imageExtent.depth = 1;
          bic.bufferOffset = offset;
          
          bufferCopyRegions.push_back( bic );
          offset += images[ face ].size;
        }
      }

      // The sub resource range describes the regions of the image we will be transition
      vk::ImageSubresourceRange subresourceRange;
      // Image only contains color data
      subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
      // Start at first mip level
      subresourceRange.baseMipLevel = 0;
      // We will transition on all mip levels
      subresourceRange.levelCount = 1;// TODO: mipLevels;
                                      // The cubemap texture has six layers
      subresourceRange.layerCount = 6;

      // Image barrier for optimal image (target)
      // Optimal image will be used as destination for the copy
      // Transition image layout VK_IMAGE_LAYOUT_UNDEFINED 
      //    to VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
      utilities::transitionImageLayout(
        copyCmd,
        image,
        vk::ImageLayout::eUndefined,          // Old layout is undefined
        vk::ImageLayout::eTransferDstOptimal, // New layout
        subresourceRange
      );

      // Copy the cube map faces from the staging buffer to the optimal tiled image
      copyCmd->copyBufferToImage( stagingBuffer, image,
        vk::ImageLayout::eTransferDstOptimal, bufferCopyRegions );

      // Change texture image layout to shader read after all mip levels have been copied
      this->imageLayout = imageLayout_;

      // Transition image layout VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
      //    to VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
      utilities::transitionImageLayout(
        copyCmd,
        image,
        vk::ImageLayout::eTransferDstOptimal, // Older layout
        imageLayout,                          // New layout
        subresourceRange
      );

      // Send command buffer
      copyCmd->end( );

      queue->submitAndWait( copyCmd );

      // Clean up staging resources
      stagingBuffer.reset( );
    }
    else
    {
      // Prefer using optimal tiling, as linear tiling 
      // may support only a small set of features 
      // depending on implementation (e.g. no mip maps, only one layer, etc.)
      assert( formatProps.linearTilingFeatures & 
        vk::FormatFeatureFlagBits::eSampledImage );

      // Check if this support is supported for linear tiling
      vk::Image mappableImage;
      vk::DeviceMemory mappableMemory;

      vk::ImageCreateInfo ici;
      ici.imageType = vk::ImageType::e2D;
      ici.format = format;
      ici.extent.width = width;
      ici.extent.height = height;
      ici.extent.depth = 1;
      ici.mipLevels = 1;
      // This flag is required for cube map images
      ici.flags = vk::ImageCreateFlagBits::eCubeCompatible;
      // Cube faces count as array layers in Vulkan
      ici.arrayLayers = 6;
      ici.samples = vk::SampleCountFlagBits::e1;
      ici.tiling = vk::ImageTiling::eLinear;
      ici.usage = imageUsageFlags;
      ici.sharingMode = vk::SharingMode::eExclusive;
      ici.initialLayout = vk::ImageLayout::eUndefined;

      /*TODOmappableImage = device.createImage( ici );
      mappableMemory = _device->allocateImageMemory( mappableImage,
        vk::MemoryPropertyFlagBits::eHostVisible | 
        vk::MemoryPropertyFlagBits::eHostCoherent );  // Allocate + bind

      vk::ImageSubresource subRes;
      subRes.aspectMask = vk::ImageAspectFlagBits::eColor;
      subRes.mipLevel = 0;

      void* data = device.mapMemory( mappableMemory, 0, totalSize );
      memcpy( data, pixels, totalSize );
      device.unmapMemory( mappableMemory );

      // Linear tiled images don't need to be staged
      // and can be directly used as textures
      image = mappableImage;
      deviceMemory = mappableMemory;*/
      this->imageLayout = imageLayout_;

      std::shared_ptr<CommandBuffer> copyCmd = cmdPool->allocateCommandBuffer( );
      copyCmd->begin( vk::CommandBufferUsageFlagBits::eOneTimeSubmit );
      
      // Setup image memory barrier
      utilities::transitionImageLayout(
        copyCmd,
        image,
        vk::ImageAspectFlagBits::eColor,
        vk::ImageLayout::eUndefined, // Older layout
        imageLayout                  // New layout
      );

      // Send command buffer
      copyCmd->end( );

      queue->submitAndWait( copyCmd );
    }

    // Create default sampler
    sampler = _device->createSampler( vk::Filter::eLinear, vk::Filter::eLinear,
      vk::SamplerMipmapMode::eLinear, vk::SamplerAddressMode::eClampToEdge,
      vk::SamplerAddressMode::eClampToEdge, vk::SamplerAddressMode::eClampToEdge,
      0.0f, true, 1.0f, false, vk::CompareOp::eNever, 0.0f, 0.0f,
      vk::BorderColor::eFloatOpaqueWhite, false );

    // Create image view
    view = image->createImageView( vk::ImageViewType::eCube, format,
      vk::ComponentMapping(
        vk::ComponentSwizzle::eR, vk::ComponentSwizzle::eG,
        vk::ComponentSwizzle::eB, vk::ComponentSwizzle::eA ),
      vk::ImageSubresourceRange(
        // 6 array layers (faces)
        vk::ImageAspectFlagBits::eColor, 0, 1, 0, 6
      )
    );

    updateDescriptor( );
  }

  TextureCubemap::TextureCubemap( const std::shared_ptr<Device>& device_, 
    uint32_t dimensions, vk::Format format, vk::ImageUsageFlags imageUsageFlags,
    vk::ImageLayout imageLayout_ )
  : Texture( device_ )
  {
    width = dimensions;
    height = dimensions;

    mipLevels = 1;

    this->image = _device->createImage( 
      // This flag is required for cube map images
      vk::ImageCreateFlagBits::eCubeCompatible, vk::ImageType::e2D,
      format, vk::Extent3D( dimensions, dimensions, 1 ), mipLevels,
      // Cube faces count as array layers in Vulkan
      6,
      vk::SampleCountFlagBits::e1, vk::ImageTiling::eOptimal,
      imageUsageFlags, vk::SharingMode::eExclusive, { }, 
      vk::ImageLayout::eUndefined, vk::MemoryPropertyFlagBits::eDeviceLocal );

    this->imageLayout = imageLayout_;

    // Create default sampler
    this->sampler = _device->createSampler( vk::Filter::eLinear, vk::Filter::eLinear,
      vk::SamplerMipmapMode::eLinear, vk::SamplerAddressMode::eClampToEdge,
      vk::SamplerAddressMode::eClampToEdge, vk::SamplerAddressMode::eClampToEdge,
      0.0f, true, 1.0f, false, vk::CompareOp::eNever, 0.0f, 0.0f,
      vk::BorderColor::eFloatOpaqueWhite, false );

    // Create image view
    this->view = image->createImageView( vk::ImageViewType::eCube, format,
      vk::ComponentMapping(
        vk::ComponentSwizzle::eR, vk::ComponentSwizzle::eG,
        vk::ComponentSwizzle::eB, vk::ComponentSwizzle::eA ),
      vk::ImageSubresourceRange(
        // 6 array layers (faces)
        vk::ImageAspectFlagBits::eColor, 0, 1, 0, 6
      )
    );
  }
}
