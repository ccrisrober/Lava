#include "Texture.h"

#include "Device.h"
#include "PhysicalDevice.h"

#include "utils.hpp"

namespace lava
{
  Texture2D::Texture2D( const DeviceRef& device_, const std::string& filename )
    : VulkanResource( device_ )
  {
    uint32_t width, height, numChannels;
    unsigned char* pixels = lava::utils::loadImageTexture( 
      filename, width, height, numChannels );
    /*
    vk::ImageCreateInfo ici;
    ici.imageType = vk::ImageType::e2D;
    ici.extent = { width, height, 1 };
    ici.mipLevels = 1;
    ici.arrayLayers = 1;
    ici.format = vk::Format::eR8G8B8A8Unorm; //format;
    ici.tiling = vk::ImageTiling::eOptimal; //eLinear;
    ici.initialLayout = vk::ImageLayout::eUndefined;
    ici.usage = vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled;
    ici.sharingMode = vk::SharingMode::eExclusive;
    ici.samples = vk::SampleCountFlagBits::e1;

    image = static_cast< vk::Device >( *_device ).createImage( ici );

    imageMemory = _device->allocateImageMemory( 
      image, vk::MemoryPropertyFlagBits::eDeviceLocal );
    */

    vk::Format format = vk::Format::eR8G8B8A8Unorm;
    vk::FormatProperties formatProps = 
      _device->_physicalDevice->getFormatProperties( format );

    bool useStaging = false;

    VkDeviceSize texSize = width * height * 4; // todo: 4 is numChannels?

    vk::Device device = static_cast< vk::Device >( *_device );

    if ( useStaging )
    {
      // Create a host-visible staging buffer that contains the raw image data
      vk::Buffer stagingBuffer;
      vk::DeviceMemory stagingMemory;

      vk::BufferCreateInfo bci;
      bci.size = texSize;
      bci.sharingMode = vk::SharingMode::eExclusive;

      stagingBuffer = device.createBuffer( bci );
      stagingMemory = _device->allocateBufferMemory( stagingBuffer, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent );

      // Copy texture data into staging buffer
      void* data = device.mapMemory( stagingMemory, 0, texSize );
      memcpy( data, pixels, texSize );
      device.unmapMemory( stagingMemory );

      // TODO: bufferImageCopy
      
      vk::BufferImageCopy bufferCopyRegion = {};
      bufferCopyRegion.bufferOffset = 0;
      bufferCopyRegion.bufferRowLength = 0;
      bufferCopyRegion.bufferImageHeight = 0;
      bufferCopyRegion.imageSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
      bufferCopyRegion.imageSubresource.mipLevel = 0;
      bufferCopyRegion.imageSubresource.baseArrayLayer = 0;
      bufferCopyRegion.imageSubresource.layerCount = 1;
      bufferCopyRegion.imageOffset = { 0, 0, 0 };
      bufferCopyRegion.imageExtent = {
        width,
        height,
        1
      };

      // Create optimal tiled target image
      vk::ImageCreateInfo ici;
      ici.imageType = vk::ImageType::e2D;
      ici.format = format;
      ici.mipLevels = 1;// mipLevels;
      ici.arrayLayers = 1;
      ici.samples = vk::SampleCountFlagBits::e1;
      ici.tiling = vk::ImageTiling::eOptimal;
      ici.sharingMode = vk::SharingMode::eExclusive;
      ici.initialLayout = vk::ImageLayout::eUndefined;
      ici.extent = { width, height, 1 };
      ici.usage = vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits:::eSampled; // imageUsageFlags;

      // Ensure that the transfer_dst bit is set for stagging
      if ( !( ici.usage & vk::ImageUsageFlagBits::eTransferDst ) )
      {
        ici.usage |= vk::ImageUsageFlagBits::eTransferDst;
      }
      image = device.createImage( ici );
      imageMemory = _device->allocateImageMemory( image, vk::MemoryPropertyFlagBits::eDeviceLocal );
    

      std::shared_ptr<CommandPool> commandPool = _device->createCommandPool(
        vk::CommandPoolCreateFlagBits::eResetCommandBuffer, 0/*_queueFamilyIndex*/ );
      std::shared_ptr<CommandBuffer> copyCmd = commandPool->allocateCommandBuffer( );
      
      // The sub resource range describes the regions of the image we will be transition
      vk::ImageSubresourceRange ssr;
      // Image only contains color data
      ssr.aspectMask = vk::ImageAspectFlagBits::eColor;
      // Start at first mip level
      ssr.baseMipLevel = 0;
      // We will transition on all mip levels
      ssr.levelCount = 1;// mipLevels;
      // The 2D texture only has one layer
      ssr.layerCount = 1;

      // Optimal image will be used as destination for the copy, so we must transfer from our
      // initial undefined image layout to the transfer destination layout
      utils::setImageLayout(
        copyCmd,
        image,
        vk::ImageAspectFlagBits::eColor,
        vk::ImageLayout::eUndefined,
        vk::ImageLayout::eTransferDstOptimal,
        ssr
      );

      static_cast< vk::CommandBuffer >( *copyCmd ).copyBufferToImage( 
        stagingBuffer, image, vk::ImageLayout::eTransferDstOptimal, bufferCopyRegion );

      // Change texture image layout to shader read after all mip levels have been copied
      imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
      utils::setImageLayout(
        copyCmd,
        image,
        vk::ImageAspectFlagBits::eColor,
        vk::ImageLayout::eTransferDstOptimal,
        imageLayout,
        ssr
      );

      // Clean up staging resources
      device.freeMemory( stagingMemory );
      device.destroyBuffer( stagingBuffer );

      // todo: send command buffer
    }
    else
    {
      // Check if this support is supported for linear tiling
      assert( formatProps.linearTilingFeatures & vk::FormatFeatureFlagBits::eSampledImage );

      vk::ImageCreateInfo ici;
      ici.imageType = vk::ImageType::e2D;
      ici.extent = { width, height, 1 };
      ici.mipLevels = 1;
      ici.arrayLayers = 1;
      ici.format = format;
      ici.tiling = vk::ImageTiling::eOptimal; //eLinear;
      ici.initialLayout = vk::ImageLayout::eUndefined;
      ici.usage = vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled;
      ici.sharingMode = vk::SharingMode::eExclusive;
      ici.samples = vk::SampleCountFlagBits::e1;

      // Get subresource layout
      vk::ImageSubresource subRes;
      subRes.aspectMask = vk::ImageAspectFlagBits::eColor;
      subRes.mipLevel = 0;


      vk::Image mappableImage;
      vk::DeviceMemory mappableMemory;
      
      void* data;

      /*// Get subresources layout
      vk::SubresourceLayout subResLayout = static_cast< vk::Device >( *_device )
        .getImageSubresourceLayout( mappableImage, subRes );
      static_cast< vk::Device >( *_device ).mapMemory( mappableMemory, 0, memReqs.size, 0, &data );
      // Copy image memory
      memcpy( data, pixels, size );
      static_cast< vk::Device >( *_device ).unmapMemory( mappableMemory );*/

      // Linear tiled images don't need to be staged
      // and can be directly used as textures
      image = mappableImage;
      imageMemory = mappableMemory;


    }
  }
  /*Texture::Texture( const DeviceRef & device )
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
  }*/
}