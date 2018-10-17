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

#include "Texture2D.h"

#include <pompeii/Buffer.h>
#include <pompeii/PhysicalDevice.h>

#include "utilities.hpp"

namespace pompeii
{
  Texture2D::Texture2D( const std::shared_ptr<Device>& device_, 
    const std::string& filename, const std::shared_ptr<CommandPool>& cmdPool,
    const std::shared_ptr<Queue>& queue, vk::Format format,
    vk::ImageUsageFlags imageUsageFlags, vk::ImageLayout imageLayout_, 
    bool forceLinear )
    : Texture( device_ )
  {
    unsigned int channels;
    unsigned char* pixels = pompeii::utilities::loadImageTexture( 
      filename, width, height, channels );

    channels = 4; // TODO: HARCODED utils::channelsFromFormat( format );
    if ( channels < 4 ) { forceLinear = true; }

    createTexture( pixels, width, height, channels, cmdPool, queue, format,
      imageUsageFlags, imageLayout_, forceLinear );
  }
  Texture2D::Texture2D( const std::shared_ptr<Device>& device_,
    vk::ArrayProxy<void*> data, const std::shared_ptr<CommandPool>& cmdPool, 
    const std::shared_ptr<Queue>& queue, vk::Format format, 
    vk::ImageUsageFlags imageUsageFlags_, vk::ImageLayout imageLayout_, 
    bool forceLinear )
    : Texture( device_ )
  {
    short channels = 4; // TODO: HARCODED utils::channelsFromFormat( format );
    createTexture( data.data( ), width, height, channels, cmdPool, queue, format,
      imageUsageFlags_, imageLayout_, forceLinear );
  }
  void Texture2D::createTexture( void* pixels, uint32_t width_,
    uint32_t height_, short channels, const std::shared_ptr<CommandPool>& cmdPool,
    const std::shared_ptr<Queue>& queue_, vk::Format format_, 
    vk::ImageUsageFlags imageUsageFlags_, vk::ImageLayout imageLayout_, 
    bool forceLinear_ )
  {
    vk::FormatProperties formatProps =
      _device->getPhysicalDevice( )->getFormatProperties( format_ );

    // Only use linear tiling if requested (and supported by the device)
    // Support for linear tiling is mostly limited, so prefer to use
    // optimal tiling instead
    // On most implementations linear tiling will only support a very
    // limited amount of formats and features (mip maps, cubemaps, arrays, etc.)
    VkBool32 useStaging = !forceLinear_;

    vk::DeviceSize texSize = width_ * height_ * channels;

    vk::Device device = static_cast< vk::Device >( *_device );

    if ( useStaging )
    {
      // Create a host-visible staging buffer that contains the raw image data
      std::shared_ptr<Buffer> stagingBuffer = _device->createBuffer( texSize,
        vk::BufferUsageFlagBits::eTransferSrc, vk::SharingMode::eExclusive, { },
        vk::MemoryPropertyFlagBits::eHostVisible |
        vk::MemoryPropertyFlagBits::eHostCoherent );
      stagingBuffer->writeData( 0, texSize, pixels );

      free( pixels );

      // TODO: Generate MipLevels
      mipLevels = 1;

      this->width = width_;
      this->height = height_;

      /*{
        // calculate num of mip maps
        // numLevels = 1 + floor(log2(max(w, h, d)))
        // Calculated as log2(max(width, height, depth))c + 1 (see specs)
        mipLevels = floor( log2( std::max( width, height ) ) ) + 1;

        // Get device properites for the requested texture format
        auto formatProperties = _device->getPhysicalDevice( )->getFormatProperties( format_ );

        // Mip-chain generation requires support for blit source and destination
        assert( formatProperties.optimalTilingFeatures & vk::FormatFeatureFlagBits::eBlitSrc );
        assert( formatProperties.optimalTilingFeatures & vk::FormatFeatureFlagBits::eBlitDst );
      }*/

      // Create Image
      auto usageFlags = imageUsageFlags_;

      // Ensure that the TRANSFER_DST bit is set for staging
      if ( !( usageFlags & vk::ImageUsageFlagBits::eTransferDst ) )
      {
        usageFlags |= vk::ImageUsageFlagBits::eTransferDst;
      }

      const uint32_t arraySize = 1;
      image = _device->createImage( { }, vk::ImageType::e2D, format_,
        vk::Extent3D( width, height, 1 ), mipLevels, arraySize,
        vk::SampleCountFlagBits::e1, vk::ImageTiling::eOptimal, usageFlags,
        vk::SharingMode::eExclusive, { }, vk::ImageLayout::eUndefined,
        vk::MemoryPropertyFlagBits::eDeviceLocal );

      auto copyCmd = cmdPool->allocateCommandBuffer( );
      copyCmd->begin( vk::CommandBufferUsageFlagBits::eOneTimeSubmit );

      // The sub resource range describes the regions of the image we will be transition
      vk::ImageSubresourceRange subresourceRange;
      // Image only contains color data
      subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
      // Start at first mip level
      subresourceRange.baseMipLevel = 0;
      // We will transition on all mip levels
      subresourceRange.levelCount = mipLevels;
      // The 2D texture only has one layer
      subresourceRange.layerCount = 1;

      // Image barrier for optimal image (target)
      // Optimal image will be used as destination for the copy
      // Transition image layout VK_IMAGE_LAYOUT_UNDEFINED to VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
      utilities::transitionImageLayout(
        copyCmd,
        image,
        vk::ImageLayout::eUndefined,          // Old layout is undefined
        vk::ImageLayout::eTransferDstOptimal, // New layout
        subresourceRange
      );

      // Copy buffer to image
      std::vector<vk::BufferImageCopy> bufferCopyRegions;
      uint32_t offset = 0;
      for ( uint32_t i = 0; i < mipLevels; ++i )
      {
        vk::BufferImageCopy region = { };
        region.imageSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
        region.imageSubresource.mipLevel = i;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount = 1;
        region.imageExtent = vk::Extent3D(
          width,
          height,
          1
        );
        region.bufferOffset = offset;
        bufferCopyRegions.push_back( region );
        offset += texSize;
      }

      copyCmd->copyBufferToImage( stagingBuffer, image,
        vk::ImageLayout::eTransferDstOptimal, bufferCopyRegions );

      // Change texture image layout to shader read after all mip levels have been copied
      this->imageLayout = imageLayout_;

      // Transition image layout VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL to VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
      utilities::transitionImageLayout(
        copyCmd,
        image,
        vk::ImageLayout::eTransferDstOptimal, // Older layout
        imageLayout,                          // New layout
        subresourceRange
      );

      // Send command buffer
      copyCmd->end( );

      queue_->submitAndWait( copyCmd );

      // Clean up staging resources
      stagingBuffer.reset( );
    }
    else
    {
      // Prefer using optimal tiling, as linear tiling 
      // may support only a small set of features 
      // depending on implementation (e.g. no mip maps, only one layer, etc.)
      assert( formatProps.linearTilingFeatures & vk::FormatFeatureFlagBits::eSampledImage );

      // Check if this support is supported for linear tiling
      vk::Image mappableImage;
      vk::DeviceMemory mappableMemory;

      vk::ImageCreateInfo ici;
      ici.imageType = vk::ImageType::e2D;
      ici.format = format_;
      ici.extent.width = width;
      ici.extent.height = height;
      ici.extent.depth = 1;
      ici.mipLevels = 1;
      ici.arrayLayers = 1;
      ici.samples = vk::SampleCountFlagBits::e1;
      ici.tiling = vk::ImageTiling::eLinear;
      ici.usage = imageUsageFlags_;
      ici.sharingMode = vk::SharingMode::eExclusive;
      ici.initialLayout = vk::ImageLayout::eUndefined;

      mappableImage = device.createImage( ici );
      mappableMemory = _device->allocateImageMemory( mappableImage,
        vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent );  // Allocate + bind

      vk::ImageSubresource subRes;
      subRes.aspectMask = vk::ImageAspectFlagBits::eColor;
      subRes.mipLevel = 0;

      // Get sub resources layout 
      // Includes row pitch, size offsets, etc.
      //vk::SubresourceLayout subResLayout = device.getImageSubresourceLayout( mappableImage, subRes );

      /*TODOvoid* data = device.mapMemory( mappableMemory, 0, texSize );
      memcpy( data, pixels, texSize );
      device.unmapMemory( mappableMemory );

      // Linear tiled images don't need to be staged
      // and can be directly used as textures
      TODOimage = mappableImage;
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

      queue_->submitAndWait( copyCmd );
    }

    sampler = _device->createSampler( vk::Filter::eLinear, vk::Filter::eLinear,
      vk::SamplerMipmapMode::eLinear, vk::SamplerAddressMode::eRepeat,
      vk::SamplerAddressMode::eRepeat, vk::SamplerAddressMode::eRepeat,
      0.0f, true, 1.0f, false, vk::CompareOp::eNever, 0.0f, 0.0f,
      vk::BorderColor::eFloatOpaqueWhite, false );

    // Create image view
    view = image->createImageView( vk::ImageViewType::e2D, format_ );

    updateDescriptor( );
  }
}
