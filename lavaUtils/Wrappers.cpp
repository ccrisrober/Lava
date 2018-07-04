#include "Wrappers.h"

namespace lava
{
  namespace utility
  {
    OffscreenFBO::OffscreenFBO( const std::shared_ptr<Device>& device,
      uint32_t w, uint32_t h )
      : VulkanResource( device )
    {
      _width = w;
      _height = h;
      vk::SampleCountFlagBits samples = vk::SampleCountFlagBits::e1;

      
      colorAttachment.image = _device->createImage( { }, vk::ImageType::e2D,
        vk::Format::eR16G16B16A16Sfloat, vk::Extent3D( _width, _height, 1 ),
        1, 1, samples, vk::ImageTiling::eOptimal,
        vk::ImageUsageFlagBits::eColorAttachment |
        vk::ImageUsageFlagBits::eSampled |
        vk::ImageUsageFlagBits::eStorage, vk::SharingMode::eExclusive, { },
        vk::ImageLayout::eUndefined, vk::MemoryPropertyFlagBits::eDeviceLocal );
      colorAttachment.imageView = colorAttachment.image->createImageView(
        vk::ImageViewType::e2D, vk::Format::eR16G16B16A16Sfloat,
        vk::ImageAspectFlagBits::eColor );
      depthBuffer.image = _device->createImage( { }, vk::ImageType::e2D,
        vk::Format::eR16G16B16A16Sfloat, vk::Extent3D( _width, _height, 1 ),
        1, 1, samples, vk::ImageTiling::eOptimal,
        vk::ImageUsageFlagBits::eDepthStencilAttachment |
        vk::ImageUsageFlagBits::eSampled, vk::SharingMode::eExclusive, { },
        vk::ImageLayout::eUndefined, vk::MemoryPropertyFlagBits::eDeviceLocal );
      depthBuffer.imageView = depthBuffer.image->createImageView(
        vk::ImageViewType::e2D, vk::Format::eD32Sfloat,
        vk::ImageAspectFlagBits::eDepth );

      auto rpb = new RenderPassBuilder( );
      rpb->setAttachment( vk::Format::eR16G16B16A16Sfloat, samples,
        vk::ImageLayout::eUndefined, vk::ImageLayout::eGeneral,
        vk::AttachmentLoadOp::eClear );
      rpb->setAttachment( vk::Format::eD32Sfloat, samples,
        vk::ImageLayout::eUndefined, vk::ImageLayout::eGeneral,
        vk::AttachmentLoadOp::eClear );
      rpb->addColorAttachmentReference(
        0, vk::ImageLayout::eColorAttachmentOptimal );
      rpb->addDepthAttachmentReference(
        1, vk::ImageLayout::eDepthStencilAttachmentOptimal );

      rpb->setSubpassDependency( VK_SUBPASS_EXTERNAL, 0,
        vk::PipelineStageFlagBits::eBottomOfPipe,
        vk::PipelineStageFlagBits::eColorAttachmentOutput,
        vk::AccessFlagBits::eMemoryRead,
        vk::AccessFlagBits::eColorAttachmentRead |
        vk::AccessFlagBits::eColorAttachmentWrite,
        vk::DependencyFlagBits::eByRegion );
      rpb->setSubpassDependency( 0, VK_SUBPASS_EXTERNAL,
        vk::PipelineStageFlagBits::eColorAttachmentOutput,
        vk::PipelineStageFlagBits::eBottomOfPipe,
        vk::AccessFlagBits::eColorAttachmentRead |
        vk::AccessFlagBits::eColorAttachmentWrite,
        vk::AccessFlagBits::eMemoryRead,
        vk::DependencyFlagBits::eByRegion );

      rpb->createSubpass( );
      _renderPass = rpb->createRenderPass( _device );

      _framebuffer = _device->createFramebuffer( _renderPass, 
        { colorAttachment.imageView, depthBuffer.imageView }, 
        vk::Extent2D( _width, _height ), 1 );
    }

    void OffscreenFBO::resize( uint32_t w, uint32_t h )
    {
      vk::SampleCountFlagBits samples = vk::SampleCountFlagBits::e1;

      _width = w;
      _height = h;

      colorAttachment.image = _device->createImage( { }, vk::ImageType::e2D,
        vk::Format::eR16G16B16A16Sfloat, vk::Extent3D( _width, _height, 1 ),
        1, 1, samples, vk::ImageTiling::eOptimal,
        vk::ImageUsageFlagBits::eColorAttachment |
        vk::ImageUsageFlagBits::eSampled |
        vk::ImageUsageFlagBits::eStorage, vk::SharingMode::eExclusive, { },
        vk::ImageLayout::eUndefined, vk::MemoryPropertyFlagBits::eDeviceLocal );
      colorAttachment.imageView = colorAttachment.image->createImageView(
        vk::ImageViewType::e2D, vk::Format::eR16G16B16A16Sfloat,
        vk::ImageAspectFlagBits::eColor );
      depthBuffer.image = _device->createImage( { }, vk::ImageType::e2D,
        vk::Format::eR16G16B16A16Sfloat, vk::Extent3D( _width, _height, 1 ),
        1, 1, samples, vk::ImageTiling::eOptimal,
        vk::ImageUsageFlagBits::eDepthStencilAttachment |
        vk::ImageUsageFlagBits::eSampled, vk::SharingMode::eExclusive, { },
        vk::ImageLayout::eUndefined, vk::MemoryPropertyFlagBits::eDeviceLocal );
      depthBuffer.imageView = depthBuffer.image->createImageView(
        vk::ImageViewType::e2D, vk::Format::eD32Sfloat,
        vk::ImageAspectFlagBits::eDepth );

      _framebuffer = _device->createFramebuffer( _renderPass,
      { colorAttachment.imageView, depthBuffer.imageView },
        vk::Extent2D( _width, _height ), 1 );
    }

    ComputeCmdBuffer::ComputeCmdBuffer( std::shared_ptr<CommandPool> cmdPool,
      const std::shared_ptr<ComputePipeline> pipeline,
      const std::shared_ptr<PipelineLayout> pipelineLayout,
      vk::ArrayProxy<const std::shared_ptr<DescriptorSet>> dSets,
      int groupCountX, int groupCountY, int groupCountZ,
      vk::CommandBufferUsageFlags usage, void* pushData )
      : CommandBuffer( cmdPool, vk::CommandBufferLevel::ePrimary )
    {
      record( pipeline, pipelineLayout, dSets, 
        groupCountX, groupCountY, groupCountZ, usage, pushData );
    }
    void ComputeCmdBuffer::record( std::shared_ptr<ComputePipeline> pipeline,
      const std::shared_ptr<PipelineLayout> pipelineLayout,
      vk::ArrayProxy<const std::shared_ptr<DescriptorSet>> dSets,
      int groupCountX, int groupCountY, int groupCountZ,
      vk::CommandBufferUsageFlags usage, void* pushConstantsData )
    {
      begin( usage );
      if ( pushConstantsData != nullptr ) {
        this->pushConstant( pipelineLayout, vk::ShaderStageFlagBits::eCompute,
          pushConstantsData );
      }
      bindPipeline( vk::PipelineBindPoint::eCompute, pipeline );
      bindDescriptorSets( 
        vk::PipelineBindPoint::eCompute, pipelineLayout, 0, dSets, { } );
      dispatch( groupCountX, groupCountY, groupCountZ );
      end( );
    }
      
    MipMapGenerationCmdBuffer::MipMapGenerationCmdBuffer( std::shared_ptr<CommandPool> cmdPool,
      std::shared_ptr<Image> image, uint32_t width, uint32_t height, uint32_t mipLevels,
      vk::ImageLayout initialLayout, vk::AccessFlags initialSrcAccessMask,
      vk::PipelineStageFlags initialSrcStageMask,
      vk::ImageLayout finalLayout, vk::AccessFlags finalDstAccessMask,
      vk::PipelineStageFlags finalDstStageMask )
      : CommandBuffer( cmdPool, vk::CommandBufferLevel::ePrimary )
    {
      begin( vk::CommandBufferUsageFlagBits::eOneTimeSubmit/*eRenderPassContinue*/ );

      vk::ImageSubresourceRange isr;
      isr
        .setBaseMipLevel( 0 )
        .setAspectMask( vk::ImageAspectFlagBits::eColor )
        .setBaseArrayLayer( 0 )
        .setLayerCount( 1 )
        .setLevelCount( 1 );

      ImageMemoryBarrier barrier(
        initialSrcAccessMask, vk::AccessFlagBits::eTransferRead,
        initialLayout, vk::ImageLayout::eTransferSrcOptimal,
        VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED, image, isr );

      pipelineBarrier( initialSrcStageMask, vk::PipelineStageFlagBits::eTransfer, 
        { }, { }, { }, barrier );

      uint32_t mipWidth = width;
      uint32_t mipHeight = height;
      for ( uint32_t i = 1; i < mipLevels; ++i )
      {
        barrier.oldLayout = vk::ImageLayout::eUndefined;
        barrier.newLayout = vk::ImageLayout::eTransferDstOptimal;
        barrier.srcAccessMask = { };
        barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;
        barrier.subresourceRange.baseMipLevel = i;

        pipelineBarrier( 
          vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eTransfer, 
          vk::DependencyFlagBits( ), { }, { }, barrier );

        vk::Offset3D src0_offset3D;
        src0_offset3D.setX( 0 ).setY( 0 ).setZ( 0 );

        vk::Offset3D src1_offset3D;
        src1_offset3D.setX( mipWidth ).setY( mipHeight ).setZ( 1 );

        vk::Offset3D dst0_offset3D;
        dst0_offset3D.setX( 0 ).setY( 0 ).setZ( 0 );

        vk::Offset3D dst1_offset3D;
        dst1_offset3D.setX( mipWidth / 2 ).setY( mipHeight / 2 ).setZ( 1 );

        vk::ImageBlit blit;
        blit.setSrcOffsets( { src0_offset3D, src1_offset3D } );
        blit.setDstOffsets( { dst0_offset3D, dst1_offset3D } );
        blit.srcSubresource.
          setAspectMask( vk::ImageAspectFlagBits::eColor )
          .setMipLevel( i - 1 )
          .setBaseArrayLayer( 0 )
          .setLayerCount( 1 );
        blit.dstSubresource.
          setAspectMask( vk::ImageAspectFlagBits::eColor )
          .setMipLevel( i )
          .setBaseArrayLayer( 0 )
          .setLayerCount( 1 );

        blitImage( image, vk::ImageLayout::eTransferSrcOptimal, 
          image, vk::ImageLayout::eTransferDstOptimal, 
          blit, vk::Filter::eLinear );

        barrier.oldLayout = vk::ImageLayout::eTransferDstOptimal;
        barrier.newLayout = vk::ImageLayout::eTransferSrcOptimal;
        barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
        barrier.dstAccessMask = vk::AccessFlagBits::eTransferRead;

        pipelineBarrier(
          vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eTransfer,
          vk::DependencyFlagBits( ), { }, { }, barrier );

        if ( mipWidth > 1 )
        {
          mipWidth /= 2;
        }
        if ( mipHeight > 1 )
        {
          mipHeight /= 2;
        }
      }

      barrier.subresourceRange.baseMipLevel = 0;
      barrier.subresourceRange.levelCount = mipLevels;

      barrier.oldLayout = vk::ImageLayout::eTransferSrcOptimal;
      barrier.newLayout = finalLayout;
      barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
      barrier.dstAccessMask = finalDstAccessMask;

      pipelineBarrier(
        vk::PipelineStageFlagBits::eTransfer, finalDstStageMask, //vk::PipelineStageFlagBits::eTransfer,
        vk::DependencyFlagBits( ), { }, { }, barrier );

      end( );
    }

    VertexInput::VertexInput( VertexLayout layout )
    {
      switch ( layout ) {
      case VertexLayout::POS2D:
        createBindingDescription( 0, 1, sizeof( float ) * 2 );
        addVertexAttributeDescription( 0, 
          vk::Format::eR32G32Sfloat, 0 );
        break;
      case VertexLayout::POS:
        createBindingDescription( 0, 1, sizeof( float ) * 3 );
        addVertexAttributeDescription( 0, 
          vk::Format::eR32G32B32A32Sfloat, 0 );
        break;
      case VertexLayout::POS_UV:
        createBindingDescription( 0, 2, sizeof( float ) * 5 );
        addVertexAttributeDescription( 0, 
          vk::Format::eR32G32B32A32Sfloat, 0 );
        addVertexAttributeDescription( 1, 
          vk::Format::eR32G32Sfloat, sizeof( float ) * 3 );
        break;
      case VertexLayout::POS2D_UV:
        createBindingDescription( 0, 2, sizeof( float ) * 4 );
        addVertexAttributeDescription( 0, 
          vk::Format::eR32G32Sfloat, 0 );
        addVertexAttributeDescription( 1, 
          vk::Format::eR32G32Sfloat, sizeof( float ) * 2 );
        break;
      case VertexLayout::POS_NORMAL:
        createBindingDescription( 0, 2, sizeof( float ) * 6 );
        addVertexAttributeDescription( 0, 
          vk::Format::eR32G32B32A32Sfloat, 0 );
        addVertexAttributeDescription( 1, 
          vk::Format::eR32G32B32A32Sfloat, sizeof( float ) * 3 );
        break;
      case VertexLayout::POS_NORMAL_UV:
        createBindingDescription( 0, 3, sizeof( float ) * 8 );
        addVertexAttributeDescription( 0, 
          vk::Format::eR32G32B32A32Sfloat, 0 );
        addVertexAttributeDescription( 1, 
          vk::Format::eR32G32B32A32Sfloat, sizeof( float ) * 3 );
        addVertexAttributeDescription( 2, 
          vk::Format::eR32G32Sfloat, sizeof( float ) * 6 );
        break;
      case VertexLayout::POS_NORMAL_UV_TAN_BITAN:
        createBindingDescription( 0, 5, sizeof( float ) * 14 );
        addVertexAttributeDescription( 0,
          vk::Format::eR32G32B32A32Sfloat, 0 );
        addVertexAttributeDescription( 1,
          vk::Format::eR32G32B32A32Sfloat, sizeof( float ) * 3 );
        addVertexAttributeDescription( 2,
          vk::Format::eR32G32Sfloat, sizeof( float ) * 6 );
        addVertexAttributeDescription( 3,
          vk::Format::eR32G32B32A32Sfloat, sizeof( float ) * 8 );
        addVertexAttributeDescription( 4,
          vk::Format::eR32G32B32A32Sfloat, sizeof( float ) * 11 );
        break;
      default:
        break;
      }
    }
    PipelineVertexInputStateCreateInfo VertexInput::
      getPipelineVertexInput( void ) const
    {
      return PipelineVertexInputStateCreateInfo(
        bindingDescription, attributeDescriptions
      );
    }
    void VertexInput::createBindingDescription( int binding_,
      int attributeCount, int stride )
    {
      this->binding = binding_;
      bindingDescription
        .setBinding( binding )
        .setStride( stride )
        .setInputRate( vk::VertexInputRate::eVertex );
      attributeDescriptions.clear( );
      attributeDescriptions.reserve( attributeCount );
    }
    void VertexInput::addVertexAttributeDescription( int location, 
      vk::Format format, int offset )
    {
      vk::VertexInputAttributeDescription attributeDescription;
      attributeDescription
        .setBinding( binding )
        .setLocation( location )
        .setFormat( format )
        .setOffset( offset );
      attributeDescriptions.push_back( attributeDescription );
    }
    ImageCopyCmdBuffer::ImageCopyCmdBuffer( std::shared_ptr<CommandPool> cmdPool )
      : CommandBuffer( cmdPool, vk::CommandBufferLevel::ePrimary )
    {
    }

    void ImageCopyCmdBuffer::record( const std::shared_ptr<Buffer>& stagingBuffer, 
      const std::shared_ptr<Image>& image, uint32_t width, uint32_t height )
    {
      begin( vk::CommandBufferUsageFlagBits::eOneTimeSubmit );

      vk::BufferImageCopy copyRegion;
      copyRegion
        .setBufferOffset( 0 )
        .setBufferRowLength( 0 )
        .setBufferImageHeight( 0 );

      vk::ImageSubresourceLayers subresource;
      subresource
        .setAspectMask( vk::ImageAspectFlagBits::eColor )
        .setMipLevel( 0 )
        .setBaseArrayLayer( 0 )
        .setLayerCount( 1 );

      vk::Extent3D extent = { width, height, 1 };

      vk::Offset3D offset = { 0, 0, 0 };

      copyRegion.setImageSubresource( subresource );
      copyRegion.setImageExtent( extent );
      copyRegion.setImageOffset( offset );

      copyBufferToImage( stagingBuffer, image, vk::ImageLayout::eTransferDstOptimal, copyRegion );
      end( );
    }
}
}