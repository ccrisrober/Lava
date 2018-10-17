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

#include "Texture1D.h"

#include <pompeii/Buffer.h>
#include <pompeii/PhysicalDevice.h>

#include "utilities.hpp"

namespace pompeii
{
  Texture1D::Texture1D( const std::shared_ptr<Device>& device_, 
    const std::string& filename,
    const std::shared_ptr<CommandPool>& cmdPool,
    const std::shared_ptr<Queue>& queue, vk::Format format,
    vk::ImageUsageFlags imageUsageFlags, vk::ImageLayout imageLayout_, 
    bool forceLinear )
    : Texture( device_ )
  {
    unsigned int channels;
    unsigned char* pixels = pompeii::utilities::loadImageTexture( 
      filename, width, height, channels );

    channels = 4; // TODO: hardcoded

    vk::FormatProperties formatProps = 
      _device->getPhysicalDevice( )->getFormatProperties( format );

    // Only use linear tiling if requested (and supported by the device)
    // Support for linear tiling is mostly limited, so prefer to use
    // optimal tiling instead
    // On most implementations linear tiling will only support a very
    // limited amount of formats and features (mip maps, cubemaps, arrays, etc.)
    VkBool32 useStaging = !forceLinear;

    vk::DeviceSize texSize = width * height * channels;

    if ( useStaging )
    {
      // Create a host-visible staging buffer that contains the raw image data
      std::shared_ptr<Buffer> stagingBuffer = _device->createBuffer( texSize,
        vk::BufferUsageFlagBits::eTransferSrc, vk::SharingMode::eExclusive, {},
        vk::MemoryPropertyFlagBits::eHostVisible | 
        vk::MemoryPropertyFlagBits::eHostCoherent );
      stagingBuffer->writeData( 0, texSize, pixels );

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

      image = _device->createImage( { }, vk::ImageType::e1D, format, 
        vk::Extent3D( width, 1, 1 ), mipLevels, 1,
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
      subresourceRange.levelCount = 1;// TODO: mipLevels;
      // The 1D texture only has one layer
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
      // todo: we can generate manual mip levels
      vk::BufferImageCopy region = {};
      region.bufferOffset = 0;
      region.bufferRowLength = 0;
      region.bufferImageHeight = 0;
      region.imageSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
      region.imageSubresource.mipLevel = 0;
      region.imageSubresource.baseArrayLayer = 0;
      region.imageSubresource.layerCount = 1;
      region.imageOffset = vk::Offset3D( 0, 0, 0 );
      region.imageExtent = vk::Extent3D(
        width,
        height,
        1
      );

      copyCmd->copyBufferToImage( stagingBuffer, image,
        vk::ImageLayout::eTransferDstOptimal, { region } );

      // Change texture image layout to shader read after all mip levels have been copied
      this->imageLayout = imageLayout_;
      
      // Transition image layout VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL to VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
      utilities::transitionImageLayout(
        copyCmd,
        image,
        vk::ImageLayout::eTransferDstOptimal, // Older layout
        imageLayout_,                          // New layout
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
      assert( formatProps.linearTilingFeatures & vk::FormatFeatureFlagBits::eSampledImage );

      // Check if this support is supported for linear tiling
      image = _device->createImage( { }, vk::ImageType::e1D, format, 
        vk::Extent3D( width, 1, 1 ), 1, 1, vk::SampleCountFlagBits::e1, 
        vk::ImageTiling::eLinear, imageUsageFlags, vk::SharingMode::eExclusive, 
        { }, vk::ImageLayout::eUndefined, 
        vk::MemoryPropertyFlagBits::eHostVisible | 
        vk::MemoryPropertyFlagBits::eHostCoherent
      );

      vk::ImageSubresource subRes;
      subRes.aspectMask = vk::ImageAspectFlagBits::eColor;
      subRes.mipLevel = 0;

      // Get sub resources layout 
      // Includes row pitch, size offsets, etc.
      //vk::SubresourceLayout subResLayout = device.getImageSubresourceLayout( mappableImage, subRes );

      /* TODO void* data = device.mapMemory( mappableMemory, 0, texSize );
      memcpy( data, pixels, texSize );
      device.unmapMemory( mappableMemory );*/

      // Linear tiled images don't need to be staged
      // and can be directly used as textures
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


    sampler = _device->createSampler( vk::Filter::eLinear, vk::Filter::eLinear,
      vk::SamplerMipmapMode::eLinear, vk::SamplerAddressMode::eClampToEdge,
      vk::SamplerAddressMode::eClampToEdge, vk::SamplerAddressMode::eClampToEdge,
      0.0f, true, 0.0f, false, vk::CompareOp::eNever, 0.0f, 0.0f, 
      vk::BorderColor::eFloatOpaqueWhite, false );

    // Create image view
    view = image->createImageView( vk::ImageViewType::e1D, format );

    updateDescriptor( );
  }
}
