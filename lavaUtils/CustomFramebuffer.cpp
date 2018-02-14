#include "CustomFramebuffer.h"

namespace lava
{
  namespace utility
  {
    CustomFramebuffer::CustomFramebuffer( std::shared_ptr< Device > device,
      uint32_t w, uint32_t h )
      : VulkanResource( device )
      , _width( w )
      , _height( h )
    {
      semaphore = _device->createSemaphore( );
    }
    CustomFramebuffer::~CustomFramebuffer( void )
    {
      attachments.clear( );
    }

    uint32_t CustomFramebuffer::addAttachment( uint32_t width_, 
      uint32_t height_, vk::Format format_, vk::ImageUsageFlags usage_ )
    {
      FramebufferAttachment attachment;
      attachment.format = format_;

      vk::ImageAspectFlags aspectMask = { };

      // Select aspect mask and layout based on usage

      // Color attachment
      if ( usage_ & vk::ImageUsageFlagBits::eColorAttachment )
      {
        aspectMask = vk::ImageAspectFlagBits::eColor;
      }

      // Depth (and/or stencil) attachment
      if ( usage_ & vk::ImageUsageFlagBits::eDepthStencilAttachment )
      {
        if ( attachment.hasDepth( ) )
        {
          aspectMask = vk::ImageAspectFlagBits::eDepth;
        }
        if ( attachment.hasStencil( ) )
        {
          aspectMask = aspectMask | vk::ImageAspectFlagBits::eStencil;
        }
      }

      assert( aspectMask != vk::ImageAspectFlagBits( ) );

      vk::ImageCreateInfo ici;
      ici.extent.width = width_;
      ici.extent.height = height_;
      ici.extent.depth = 1;

      attachment.image = _device->createImage( { }, vk::ImageType::e2D, format_,
        ici.extent, 1, 1, vk::SampleCountFlagBits::e1, vk::ImageTiling::eOptimal, 
        usage_, ici.sharingMode, { }, ici.initialLayout, { }
      );

      vk::ImageSubresourceRange isr;
      isr.aspectMask = aspectMask;
      isr.baseMipLevel = 0;
      isr.levelCount = 1;
      isr.baseArrayLayer = 0;
      isr.layerCount = 1;

      attachment.imageView = attachment.image->createImageView(
        vk::ImageViewType::e2D, format_, vk::ComponentMapping{
          vk::ComponentSwizzle::eR, vk::ComponentSwizzle::eG,
          vk::ComponentSwizzle::eB, vk::ComponentSwizzle::eA
        }, isr
      );

      // Fill attachment description
      attachment.description.samples = vk::SampleCountFlagBits::e1;
      attachment.description.loadOp = vk::AttachmentLoadOp::eClear;
      attachment.description.storeOp =
        ( usage_ & vk::ImageUsageFlagBits::eSampled ) ?
        vk::AttachmentStoreOp::eStore : vk::AttachmentStoreOp::eDontCare;
      attachment.description.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
      attachment.description.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
      attachment.description.format = format_;
      attachment.description.initialLayout = vk::ImageLayout::eUndefined;

      // Final layout
      // If not, final layout depends on attachment type
      if ( attachment.hasDepth( ) || attachment.hasStencil( ) )
      {
        attachment.description.finalLayout = vk::ImageLayout::eDepthStencilReadOnlyOptimal;
      }
      else
      {
        attachment.description.finalLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
      }

      attachments.push_back( attachment );

      return attachments.size( ) - 1;
    }
    void CustomFramebuffer::createSampler( vk::Filter magFilter, 
      vk::Filter minFilter, vk::SamplerAddressMode addressMode )
    {
      _sampler = _device->createSampler( magFilter, minFilter, 
        vk::SamplerMipmapMode::eLinear, addressMode, addressMode, addressMode, 
        0.0f, false, 1.0f, false, { }, 0.0f, 1.0f, 
        vk::BorderColor::eFloatOpaqueWhite, false
      );
    }
    void CustomFramebuffer::createRenderPass( void )
    {
      std::vector<vk::AttachmentDescription> attDescriptions;
      for ( auto& att : attachments )
      {
        attDescriptions.push_back( att.description );
      }

      // Collect all attachment references
      std::vector<vk::AttachmentReference> colorRefs;
      vk::AttachmentReference depthRef;
      bool hasColor = false, hasDepth = false;

      uint32_t attachIdx = 0;
      for ( auto& att : attachments )
      {
        if ( att.isDepthStencil( ) )
        {
          // CustomFramebuffer only have ONE depth attachment
          assert( !hasDepth );
          depthRef.attachment = attachIdx;
          depthRef.layout = vk::ImageLayout::eDepthStencilAttachmentOptimal;
          hasDepth = true;
        }
        else
        {
          colorRefs.push_back( {
            attachIdx, vk::ImageLayout::eColorAttachmentOptimal
          } );
          hasColor = true;
        }
        ++attachIdx;
      }

      // Create default render pass with one subpass
      vk::SubpassDescription subpass;
      subpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
      if ( hasColor )
      {
        subpass.pColorAttachments = colorRefs.data( );
        subpass.colorAttachmentCount = colorRefs.size( );
      }
      if ( hasDepth )
      {
        subpass.pDepthStencilAttachment = &depthRef;
      }
      // Use subpass dependencies for attachment layout transitions
      std::array<vk::SubpassDependency, 2> dependencies;

      dependencies[ 0 ].srcSubpass = VK_SUBPASS_EXTERNAL;
      dependencies[ 0 ].dstSubpass = 0;
      dependencies[ 0 ].srcStageMask = 
        vk::PipelineStageFlagBits::eBottomOfPipe;
      dependencies[ 0 ].dstStageMask = 
        vk::PipelineStageFlagBits::eColorAttachmentOutput;
      dependencies[ 0 ].srcAccessMask = 
        vk::AccessFlagBits::eMemoryRead;
      dependencies[ 0 ].dstAccessMask = 
        vk::AccessFlagBits::eColorAttachmentRead | 
        vk::AccessFlagBits::eColorAttachmentWrite;
      dependencies[ 0 ].dependencyFlags = vk::DependencyFlagBits::eByRegion;

      dependencies[ 1 ].srcSubpass = 0;
      dependencies[ 1 ].dstSubpass = VK_SUBPASS_EXTERNAL;
      dependencies[ 1 ].srcStageMask = 
        vk::PipelineStageFlagBits::eColorAttachmentOutput;
      dependencies[ 1 ].dstStageMask = 
        vk::PipelineStageFlagBits::eBottomOfPipe;
      dependencies[ 1 ].srcAccessMask = 
        vk::AccessFlagBits::eColorAttachmentRead | 
        vk::AccessFlagBits::eColorAttachmentWrite;
      dependencies[ 1 ].dstAccessMask = 
        vk::AccessFlagBits::eMemoryRead;
      dependencies[ 1 ].dependencyFlags = vk::DependencyFlagBits::eByRegion;

      _renderPass = _device->createRenderPass( attDescriptions, subpass, 
        dependencies );

      // Create FBO
      std::vector<std::shared_ptr<lava::ImageView>> attViews;
      for ( const auto& att : attachments )
      {
        attViews.push_back( att.imageView );
      }

      // Find max of num of layers
      uint32_t maxLayers = 1; // TODO

      _framebuffer = _device->createFramebuffer( _renderPass, attViews, 
        vk::Extent2D{ _width, _height }, maxLayers );
    }
  }
}
