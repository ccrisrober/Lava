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
  void Texture2D::loadFromFile( const DeviceRef& device, const std::string& filename,
    vk::Format format, vk::Queue copyQueue, vk::ImageUsageFlagBits, 
    vk::ImageLayout layout, bool forceLinear )
  {
    this->_device = device;

    int texWidth, texHeight, texChannels;
    stbi_uc* pixels = stbi_load( filename.c_str( ),
      &texWidth, &texHeight, &texChannels, STBI_rgb_alpha );
    if ( pixels == nullptr )
    {
      std::cerr << stbi_failure_reason( ) << std::endl;
      throw new std::runtime_error( stbi_failure_reason( ) );
    }

    width = texWidth;
    height = texHeight;
    mipLevels = texChannels;

    vk::FormatProperties formatProps = _device->_physicalDevice->getFormatProperties( format );

    vk::Bool32 useStaging = !forceLinear;

    vk::MemoryRequirements memReqs;
    vk::MemoryAllocateInfo memAllocInfo;

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



      // Setup buffer copy regions for each mip level
      std::vector<vk::BufferImageCopy> bufferCopyRegions;
      uint32_t offset = 0;





      _device->freeMemory( stagingMemory );
      static_cast< vk::Device >( *_device ).destroyBuffer( stagingBuffer );
    }
    else
    {
      // Check if this support is supported for linear tiling
      assert(formatProps.linearTilingFeatures & vk::FormatFeatureFlagBits::eSampledImage);
    
      vk::Image mappableImage;
      vk::Devicememory mappableMemory;

      vk::ImageCreateInfo ici;
      ici.imageType = vk::ImageType::e2D;
      ici.format = format;
      ici.extent = { width, height, 1 };
      ici.mipLevels = 1;
      ici.arrayLayers = 1;
      ici.samples = vk::SampleCountFlagBits::e1;
      ici.tiling = vk::ImageTiling::eLinear;
      ici.SharingMode = vk::SharingMode::eExclusive;
      ici.initialLayout = vk::ImageLayout::eUndefined;

      // Load mip map level 0 to linear image
      mappableImage = _static_cast<vk::Device>(*_device).createImage( ici );

      // Get memory requirements for this image like size and aligments
      memReqs = static_cast< vk::Device >( *_device ).getBufferMemoryRequirements( mappableImage );
      // Set memory allocation size to required memory size
      memAllocInfo.setAllocationSize( memReqs.size );
      // Get memory type that can be mapped to host memory
      memAllocInfo.memoryTpeIndex = ....

      // Allocate host memory
      mappableMemory = static_cast<vk::Device>(*_device).allocateMemory( memAllocInfo );

      // Bind allocate image for use
      static_cast<vk::Device>(*_device).bindImageMemory(mappableImage, mappableMemory, 0);

      

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