#include "Texture.h"

#include "Device.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "PhysicalDevice.h"

namespace lava
{
  Texture::Texture( const DeviceRef & device )
    : VulkanResource( device )
  {
  }
  void Texture::updateDescriptor( )
  {
    descriptor.setSampler( sampler );
    descriptor.setImageView( view );
    descriptor.setImageLayout( imageLayout );
  }

  void Texture::destroy( )
  {
    vk::Device device = static_cast< vk::Device >( *_device );
    device.destroyImageView( view );
    device.destroyImage( image );

    if ( sampler )
    {
      device.destroySampler( sampler );
    }
    device.freeMemory( memory );
  }
  void Texture2D::loadFromFile( const DeviceRef & device, const std::string & filename,
    vk::Format format, vk::Queue copyQueue, vk::ImageUsageFlagBits, vk::ImageLayout layout,
    bool forceLinear )
  {
    this->_device = device;

    int texWidth, texHeight, texChannels;
    stbi_uc* pixels = stbi_load( filename.c_str( ),
      &texWidth, &texHeight, &texChannels, STBI_rgb_alpha );
    if ( pixels == nullptr )
    {
      std::cerr << stbi_failure_reason( ) << std::endl;
      throw new std::exception( stbi_failure_reason( ) );
    }

    width = texWidth;
    height = texHeight;
    mipLevels = texChannels;

    vk::FormatProperties formatProps = _device->_physicalDevice->getFormatProperties( format );

    vk::Bool32 useStaging = !forceLinear;

    vk::MemoryRequirements memReqs;

    if ( useStaging )
    {
      vk::Buffer stagingBuffer;
      vk::DeviceMemory stagingMemory;

      vk::BufferCreateInfo bci;
      bci.setSize( texWidth * texHeight * 4 );
      bci.setUsage( vk::BufferUsageFlagBits::eTransferSrc );
      bci.setSharingMode( vk::SharingMode::eExclusive );

      stagingBuffer = static_cast< vk::Device >( *_device ).createBuffer( bci );

      memReqs = static_cast< vk::Device >( *_device ).getBufferMemoryRequirements( stagingBuffer );

      stagingMemory = _device->allocateMemReqMemory( memReqs, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent );
      static_cast< vk::Device >( *_device ).bindBufferMemory( stagingBuffer, stagingMemory, 0 );


      // Copy texture data into staging buffer
      uint8_t* data;
      static_cast< vk::Device >( *_device ).mapMemory( stagingMemory, {}, memReqs.size, {}, ( void ** ) &data );
      memcpy( data, pixels, bci.size );
      static_cast< vk::Device >( *_device ).unmapMemory( stagingMemory );








      _device->freeMemory( stagingMemory );
      static_cast< vk::Device >( *_device ).destroyBuffer( stagingBuffer );
    }
    else
    {

    }

    // Create default sampler
    vk::SamplerCreateInfo sci;
    sci.setMagFilter( vk::Filter::eLinear );
    sci.setMinFilter( vk::Filter::eLinear );
    sci.setMipmapMode( vk::SamplerMipmapMode::eLinear );
    sci.setAddressModeU( vk::SamplerAddressMode::eRepeat );
    sci.setAddressModeV( vk::SamplerAddressMode::eRepeat );
    sci.setAddressModeW( vk::SamplerAddressMode::eRepeat );
    sci.setMipLodBias( 0.0f );
    sci.setCompareOp( vk::CompareOp::eNever );
    sci.setMinLod( 0.0f );
    sci.setMaxLod( useStaging ? mipLevels : 0.0f );
    //sci.setMaxAnisotropy()
    sci.setAnisotropyEnable( VK_TRUE );
    sci.setBorderColor( vk::BorderColor::eFloatOpaqueWhite );

    sampler = static_cast< vk::Device >( *_device ).createSampler( sci );


    // Create image view
    vk::ImageViewCreateInfo vci;
    vci.setViewType( vk::ImageViewType::e2D );
    vci.setFormat( format );
    vci.setComponents( {
      vk::ComponentSwizzle::eR,
      vk::ComponentSwizzle::eG,
      vk::ComponentSwizzle::eB,
      vk::ComponentSwizzle::eA
    } );
    vci.setSubresourceRange( { vk::ImageAspectFlagBits::eColor, 0,  1, 0, 1 } );
    vci.subresourceRange.levelCount = useStaging ? mipLevels : 1;
    view = static_cast< vk::Device >( *_device ).createImageView( vci );

    // Update descriptor image info member that can be used for setting up descriptor sets
    updateDescriptor( );
  }
}