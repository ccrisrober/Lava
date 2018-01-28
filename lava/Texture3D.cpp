/**
 * Copyright (c) 2017, Lava
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

#include "Texture3D.h"

#include <lava/Buffer.h>
#include <lava/PhysicalDevice.h>

#include "utils.hpp"

namespace lava
{
  Texture3D::Texture3D( const std::shared_ptr<Device>& device_, uint32_t width_, 
    uint32_t height_, uint32_t depth_, const void* src,
    const std::shared_ptr<CommandPool>& cmdPool,
    const std::shared_ptr<Queue>& queue, vk::Format format )
    : Texture( device_ )
  {
    this->width = width_;
    this->height = height_;
    this->depth = depth_;

    auto physicalDevice = device_->getPhysicalDevice( );

    // Format support check
    // 3D texture support in Vulkan is mandatory (in contrast to OpenGL) 
    //    so no need to check if it's supported
    auto formatProps = physicalDevice->getFormatProperties( format );
    // Check if format supports transfer
    if ( !formatProps.optimalTilingFeatures && VK_IMAGE_USAGE_TRANSFER_DST_BIT )
    {
      std::cout << "Error: Device does not support flag TRANSFER_DST" <<
        " for selected texture format!" << std::endl;
      throw;
    }
    // Check if GPU supports requested 3D texture dimensions
    uint32_t maxImageDimension3D = 
      physicalDevice->getDeviceProperties( ).limits.maxImageDimension3D;
    if ( width > maxImageDimension3D || height > maxImageDimension3D || 
      depth > maxImageDimension3D )
    {
      std::cout << "Error: Requested texture dimensions is greater than" <<
        " supported 3D texture dimension!" << std::endl;
      return;
    }

    // Create Image
    vk::ImageCreateInfo ici;
    ici.imageType = vk::ImageType::e3D;
    ici.format = format;
    ici.mipLevels = 1;  // TODO: Generate MipLevels
    ici.arrayLayers = 1;
    ici.samples = vk::SampleCountFlagBits::e1;
    ici.tiling = vk::ImageTiling::eOptimal;
    ici.sharingMode = vk::SharingMode::eExclusive;
    ici.initialLayout = vk::ImageLayout::eUndefined;
    ici.extent.width = width;
    ici.extent.height = height;
    ici.extent.depth = depth;
    ici.usage = vk::ImageUsageFlagBits::eTransferDst | 
      vk::ImageUsageFlagBits::eSampled;

    vk::Device device = static_cast< vk::Device >( *_device );

    image = device.createImage( ici );
    deviceMemory = _device->allocateImageMemory( image,
      vk::MemoryPropertyFlagBits::eDeviceLocal );  // Allocate + bind

    updateData( width, height, depth, 1, src, cmdPool, queue ); // TODO: 1 is hardcoded


    // Create sampler
    vk::SamplerCreateInfo sci;
    sci.setMagFilter( vk::Filter::eLinear );
    sci.setMinFilter( vk::Filter::eLinear );
    sci.setMipmapMode( vk::SamplerMipmapMode::eLinear );
    sci.setAddressModeU( vk::SamplerAddressMode::eClampToEdge );
    sci.setAddressModeV( vk::SamplerAddressMode::eClampToEdge );
    sci.setAddressModeW( vk::SamplerAddressMode::eClampToEdge );
    sci.setMipLodBias( 0.0f );
    sci.setCompareOp( vk::CompareOp::eNever );
    sci.setMinLod( 0.0f );
    sci.setMaxLod( 0.0f );
    sci.setMaxAnisotropy( 1.0f );
    sci.setAnisotropyEnable( VK_FALSE );
    sci.setBorderColor( vk::BorderColor::eFloatOpaqueWhite );

    sampler = device.createSampler( sci );


    // Create image view
    vk::ImageViewCreateInfo vci;
    vci.setViewType( vk::ImageViewType::e3D );
    vci.setFormat( format );
    vci.setComponents( {
      vk::ComponentSwizzle::eR,
      vk::ComponentSwizzle::eG,
      vk::ComponentSwizzle::eB,
      vk::ComponentSwizzle::eA
    } );
    vci.setSubresourceRange( { vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 } );
    vci.subresourceRange.levelCount = 1;
    vci.image = image;

    view = device.createImageView( vci );

    imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;

    updateDescriptor( );
  }
  void Texture3D::updateData( uint32_t width_, uint32_t height_, uint32_t depth_, 
    uint32_t numChannels, const void * pixels,
    const std::shared_ptr<CommandPool>& cmdPool,
    const std::shared_ptr<Queue>& queue )
  {
    width = width_;
    height = height_;
    depth = depth_;
    const uint32_t texSize = width * height * depth * numChannels;
    vk::Buffer stagingBuffer;
    vk::DeviceMemory stagingMemory;

    vk::BufferCreateInfo bci;
    bci.size = texSize;
    bci.usage = vk::BufferUsageFlagBits::eTransferSrc;
    bci.sharingMode = vk::SharingMode::eExclusive;


    vk::Device device = static_cast< vk::Device >( *_device );

    stagingBuffer = device.createBuffer( bci );
    stagingMemory = _device->allocateBufferMemory( stagingBuffer,
      vk::MemoryPropertyFlagBits::eHostVisible
      | vk::MemoryPropertyFlagBits::eHostCoherent );  // Allocate + bind

    // Copy texture data into staging buffer
    void* data = device.mapMemory( stagingMemory, 0, texSize );
    memcpy( data, pixels, texSize );
    device.unmapMemory( stagingMemory );

    //free( pixels );

    std::shared_ptr<CommandBuffer> copyCmd = cmdPool->allocateCommandBuffer( );
    copyCmd->begin( vk::CommandBufferUsageFlagBits::eOneTimeSubmit );

    // The sub resource range describes the regions of the image we will be transition
    vk::ImageSubresourceRange subresourceRange;
    // Image only contains color data
    subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
    // Start at first mip level
    subresourceRange.baseMipLevel = 0;
    // We will transition on all mip levels
    subresourceRange.levelCount = 1;
    subresourceRange.layerCount = 1;

    // Image barrier for optimal image (target)
    // Optimal image will be used as destination for the copy
    // Transition image layout VK_IMAGE_LAYOUT_UNDEFINED
    //    to VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
    utils::setImageLayout(
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
      depth
    );

    static_cast<vk::CommandBuffer>( *copyCmd ).copyBufferToImage(
      stagingBuffer, image,
      vk::ImageLayout::eTransferDstOptimal, { region }
    );

    // Change texture image layout to shader read after all mip levels have been copied
    this->imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;

    // Transition image layout VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
    //    to VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
    utils::setImageLayout(
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
    device.destroyBuffer( stagingBuffer );
    _device->freeMemory( stagingMemory );
  }
}
