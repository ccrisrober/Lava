#include "TextureCubemap.h"

#include "Device.h"
#include "PhysicalDevice.h"

#include "utils.hpp"

namespace lava
{
  TextureCubemap::TextureCubemap( const DeviceRef& device_, 
      const std::array< std::string, 6 >& filePaths,
      const std::shared_ptr<CommandPool>& cmdPool,
      const std::shared_ptr<Queue>& queue, vk::Format format,
      vk::ImageUsageFlags imageUsageFlags,
      vk::ImageLayout imageLayout, bool forceLinear )
    : Texture( device_ )
  {
    uint32_t textureWidth = 0;
    uint32_t textureHeight = 0;
    uint32_t textureChannels = 0;

    struct Image
    {
      unsigned char* pixels;
      uint32_t width;
      uint32_t height;
      uint32_t textureChannels;
      uint32_t size;
    };

    std::vector<Image> images;
    uint32_t totalSize = 0;
    images.reserve( filePaths.size( ) );
    for ( uint32_t i = 0, l = filePaths.size( ); i < l; ++i )
    {
      unsigned char* pixels = lava::utils::loadImageTexture(
        filePaths[ i ].c_str( ), textureWidth, textureHeight, textureChannels);

      // The load function returns the original channel count, 
      // but it was forced to 4 because of the last parameter
      textureChannels = 4;

      uint32_t size = textureWidth * textureHeight * textureChannels * sizeof( unsigned char );
      images.push_back( { pixels, textureWidth, textureHeight, textureChannels, size } );
      totalSize += size;
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
      _device->_physicalDevice->getFormatProperties( format );

    // Only use linear tiling if requested (and supported by the device)
    // Support for linear tiling is mostly limited, so prefer to use
    // optimal tiling instead
    // On most implementations linear tiling will only support a very
    // limited amount of formats and features (mip maps, cubemaps, arrays, etc.)
    VkBool32 useStaging = !forceLinear;

    vk::Device device = static_cast< vk::Device >( *_device );

    if ( useStaging )
    {
      // Create a host-visible staging buffer that contains the raw image data
      vk::Buffer stagingBuffer;
      vk::DeviceMemory stagingMemory;

      vk::BufferCreateInfo bci;
      bci.size = totalSize;
      bci.usage = vk::BufferUsageFlagBits::eTransferSrc;
      bci.sharingMode = vk::SharingMode::eExclusive;

      stagingBuffer = device.createBuffer( bci );
      stagingMemory = _device->allocateBufferMemory( stagingBuffer,
        vk::MemoryPropertyFlagBits::eHostVisible
        | vk::MemoryPropertyFlagBits::eHostCoherent );  // Allocate + bind

      // Copy texture data into staging buffer
      void* data = device.mapMemory( stagingMemory, 0, totalSize );
      memcpy( data, pixels, totalSize );
      free( pixels );
      device.unmapMemory( stagingMemory );

      // TODO: Generate MipLevels

      // Create optimal tiled target image
      vk::ImageCreateInfo ici;
      ici.imageType = vk::ImageType::e2D;
      ici.format = format;
      ici.mipLevels = 1;  // TODO: Generate MipLevels
      ici.samples = vk::SampleCountFlagBits::e1;
      ici.tiling = vk::ImageTiling::eOptimal;
      ici.usage = imageUsageFlags;
      ici.sharingMode = vk::SharingMode::eExclusive;
      ici.initialLayout = vk::ImageLayout::eUndefined;
      ici.extent.width = textureWidth;
      ici.extent.height = textureHeight;
      ici.extent.depth = 1u;

      // Ensure that the TRANSFER_DST bit is set for staging
      if ( !( ici.usage & vk::ImageUsageFlagBits::eTransferDst ) )
      {
        ici.usage |= vk::ImageUsageFlagBits::eTransferDst;
      }
      // Cube faces count as array layers in Vulkan
      ici.arrayLayers = 6;
      // This flag is required for cube map images
      ici.flags = vk::ImageCreateFlagBits::eCubeCompatible;

      image = device.createImage( ici );
      deviceMemory = _device->allocateImageMemory( image,
        vk::MemoryPropertyFlagBits::eDeviceLocal );  // Allocate + bind

      std::shared_ptr<CommandBuffer> copyCmd = cmdPool->allocateCommandBuffer( );
      copyCmd->beginSimple( vk::CommandBufferUsageFlagBits::eOneTimeSubmit );


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
          bic.imageExtent = vk::Extent3D( textureWidth, textureHeight, 1 );
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
      // Transition image layout VK_IMAGE_LAYOUT_UNDEFINED to VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
      utils::setImageLayout(
        copyCmd,
        image,
        vk::ImageLayout::eUndefined,          // Old layout is undefined
        vk::ImageLayout::eTransferDstOptimal, // New layout
        subresourceRange
      );

      // Copy the cube map faces from the staging buffer to the optimal tiled image
      static_cast<vk::CommandBuffer>( *copyCmd ).copyBufferToImage(
        stagingBuffer, image,
        vk::ImageLayout::eTransferDstOptimal, bufferCopyRegions
      );

      // Change texture image layout to shader read after all mip levels have been copied
      this->imageLayout = imageLayout;

      // Transition image layout VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL to VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
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

      mappableImage = device.createImage( ici );
      mappableMemory = _device->allocateImageMemory( mappableImage,
        vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent );  // Allocate + bind

      vk::ImageSubresource subRes;
      subRes.aspectMask = vk::ImageAspectFlagBits::eColor;
      subRes.mipLevel = 0;

      // Get sub resources layout 
      // Includes row pitch, size offsets, etc.
      vk::SubresourceLayout subResLayout = device.getImageSubresourceLayout( mappableImage, subRes );

      void* data = device.mapMemory( mappableMemory, 0, totalSize );
      memcpy( data, pixels, totalSize );
      device.unmapMemory( mappableMemory );

      // Linear tiled images don't need to be staged
      // and can be directly used as textures
      image = mappableImage;
      deviceMemory = mappableMemory;
      imageLayout = imageLayout;

      std::shared_ptr<CommandBuffer> copyCmd = cmdPool->allocateCommandBuffer( );
      copyCmd->beginSimple( vk::CommandBufferUsageFlagBits::eOneTimeSubmit );
      // Setup image memory barrier
      utils::setImageLayout(
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
    sci.setMaxLod( /*useStaging ? mipLevels : 0.0f*/0.0f );
    sci.setMaxAnisotropy( 1.0f );
    sci.setAnisotropyEnable( VK_TRUE );
    sci.setBorderColor( vk::BorderColor::eFloatOpaqueWhite );

    sampler = device.createSampler( sci );


    // Create image view
    vk::ImageViewCreateInfo vci;
    vci.setViewType( vk::ImageViewType::eCube );
    vci.setFormat( format );
    vci.setComponents( {
      vk::ComponentSwizzle::eR,
      vk::ComponentSwizzle::eG,
      vk::ComponentSwizzle::eB,
      vk::ComponentSwizzle::eA
    } );
    vci.setSubresourceRange( { vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 } );
    // 6 array layers (faces)
    vci.subresourceRange.layerCount = 6;
    vci.subresourceRange.levelCount = 1;
    vci.image = image;

    view = device.createImageView( vci );

    updateDescriptor( );
  }
}
