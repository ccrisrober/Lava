#include "CustomFramebuffer.h"
#include "../Device.h"
#include "../PhysicalDevice.h"

namespace lava
{
  namespace extras
  {
    CustomFramebuffer::CustomFramebuffer( const DeviceRef & device )
      : VulkanResource( device )
    {
    }
    CustomFramebuffer::~CustomFramebuffer( void )
    {
      vk::Device d = static_cast< vk::Device >( *_device );
      for ( auto& att : attachments )
      {
        d.destroyImage( att.image );
        d.destroyImageView( att.view );
        d.freeMemory( att.memory );
      }
      d.destroySampler( sampler );
      d.destroyRenderPass( renderPass );
      d.destroyFramebuffer( framebuffer );
    }
    uint32_t CustomFramebuffer::addAttachment( const AttachmentCreateInfo ci )
    {
      FramebufferAttachment fatt;
      fatt.format = ci.format;
      vk::ImageAspectFlags aspectMask;

      // Select aspect mask and layout depending on usage

      // Color attachment
      if ( ci.usage & vk::ImageUsageFlagBits::eColorAttachment )
      {
        aspectMask = vk::ImageAspectFlagBits::eColor;
      }

      // Depth (and/or stencil) attachment
      if ( ci.usage & vk::ImageUsageFlagBits::eDepthStencilAttachment )
      {
        if ( fatt.hasDepth( ) )
        {
          aspectMask = vk::ImageAspectFlagBits::eDepth;
        }
        if ( fatt.hasStencil( ) )
        {
          aspectMask = aspectMask | vk::ImageAspectFlagBits::eStencil;
        }
      }

      vk::ImageCreateInfo image;
      image.setImageType( vk::ImageType::e2D );
      image.setFormat( ci.format );
      image.setExtent({ ci.width, ci.height, 1 });
      image.setMipLevels( 1 );
      image.setSamples( vk::SampleCountFlagBits::e1 );
      image.setTiling( vk::ImageTiling::eOptimal );
      image.setUsage( ci.usage );

      vk::MemoryAllocateInfo memAlloc;

      vk::Device d = static_cast< vk::Device >( *_device );
      fatt.image = d.createImage( image );


      vk::MemoryRequirements memReqs = static_cast< vk::Device >( *_device ).getImageMemoryRequirements( fatt.image );
      uint32_t memoryTypeIndex = findMemoryType( _device->_physicalDevice->getMemoryProperties( ), memReqs.memoryTypeBits, vk::MemoryPropertyFlagBits::eDeviceLocal );
      assert( memoryTypeIndex != -1 );

      memAlloc.allocationSize = memReqs.size;
      memAlloc.memoryTypeIndex = memoryTypeIndex;

      fatt.memory = _device->allocateMemReqMemory( memReqs, vk::MemoryPropertyFlagBits::eDeviceLocal );
      vk::Device( *_device ).bindImageMemory( fatt.image, fatt.memory, 0 );

      fatt.subresourceRange = {};
      fatt.subresourceRange.aspectMask = aspectMask;
      fatt.subresourceRange.levelCount = 1;
      fatt.subresourceRange.layerCount = ci.layerCount;

      vk::ImageViewCreateInfo imageView;
      imageView.viewType = ( ci.layerCount == 1 ) ? 
        vk::ImageViewType::e2D : vk::ImageViewType::e2DArray;
      imageView.format = ci.format;
      imageView.subresourceRange = fatt.subresourceRange;
      //todo: workaround for depth+stencil attachments
      imageView.subresourceRange.aspectMask = ( fatt.hasDepth( ) ) ?
        vk::ImageAspectFlagBits::eDepth : aspectMask;
      imageView.image = fatt.image;
      fatt.view = d.createImageView( imageView );


      // Fill attachment description
      fatt.description = {};
      fatt.description.samples = vk::SampleCountFlagBits::e1;
      fatt.description.loadOp = vk::AttachmentLoadOp::eClear;
      fatt.description.storeOp = ( ci.usage & vk::ImageUsageFlagBits::eSampled ) ? 
        vk::AttachmentStoreOp::eStore : vk::AttachmentStoreOp::eDontCare;
      fatt.description.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
      fatt.description.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
      fatt.description.format = ci.format;
      fatt.description.initialLayout = vk::ImageLayout::eUndefined;

      // Final layout
      // If not, final layout depends on attachment type
      if ( fatt.hasDepth( ) || fatt.hasStencil( ) )
      {
        fatt.description.finalLayout = vk::ImageLayout::eDepthStencilReadOnlyOptimal;
      }
      else
      {
        fatt.description.finalLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
      }

      attachments.push_back( fatt );

      return attachments.size( ) - 1;
    }
    void CustomFramebuffer::createRenderPass( void )
    {
      vk::Device d = static_cast< vk::Device >( *_device );

      std::vector<vk::AttachmentDescription> attDescriptions( attachments.size( ) );
      for ( auto& att : attachments )
      {
        attDescriptions.push_back( att.description );
      }

      // Collection attachments references
      std::vector<vk::AttachmentReference> colorReferences;
      vk::AttachmentReference depthRef;
      bool hasDepth = false;

      uint32_t attachmentIndex = 0;

      for ( auto& att : attachments )
      {
        if ( att.isDepthStencil( ) )
        {
          // Only one depth attachment allowed
          assert( !hasDepth );
          depthRef.attachment = attachmentIndex;
          depthRef.layout = vk::ImageLayout::eDepthStencilAttachmentOptimal;
          hasDepth = true;
        }
        else
        {
          colorReferences.push_back( vk::AttachmentReference
          { 
            attachmentIndex, 
            vk::ImageLayout::eColorAttachmentOptimal 
          } );
          ++attachmentIndex;
        }
      }

      // Default render pass setup uses only one subpass
      vk::SubpassDescription subpass = {};
      subpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
      if ( !colorReferences.empty( ) )
      {
        subpass.pColorAttachments = colorReferences.data( );
        subpass.colorAttachmentCount = static_cast<uint32_t>( colorReferences.size( ) );
      }
      if ( hasDepth )
      {
        subpass.pDepthStencilAttachment = &depthRef;
      }

      // Use subpass dependencies for attachment layout transitions
      std::array<vk::SubpassDependency, 2> dependencies;

      dependencies[ 0 ].srcSubpass = VK_SUBPASS_EXTERNAL;
      dependencies[ 0 ].dstSubpass = 0;
      dependencies[ 0 ].srcStageMask = vk::PipelineStageFlagBits::eBottomOfPipe;
      dependencies[ 0 ].dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
      dependencies[ 0 ].srcAccessMask = vk::AccessFlagBits::eMemoryRead;
      dependencies[ 0 ].dstAccessMask = vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite;
      dependencies[ 0 ].dependencyFlags = vk::DependencyFlagBits::eByRegion;

      dependencies[ 1 ].srcSubpass = 0;
      dependencies[ 1 ].dstSubpass = VK_SUBPASS_EXTERNAL;
      dependencies[ 1 ].srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
      dependencies[ 1 ].dstStageMask = vk::PipelineStageFlagBits::eBottomOfPipe;
      dependencies[ 1 ].srcAccessMask = vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite;
      dependencies[ 1 ].dstAccessMask = vk::AccessFlagBits::eMemoryRead;
      dependencies[ 1 ].dependencyFlags = vk::DependencyFlagBits::eByRegion;

      // Create render pass
      vk::RenderPassCreateInfo renderPassInfo;
      renderPassInfo.pAttachments = attDescriptions.data( );
      renderPassInfo.attachmentCount = static_cast<uint32_t>( attDescriptions.size( ) );
      renderPassInfo.subpassCount = 1;
      renderPassInfo.pSubpasses = &subpass;
      renderPassInfo.dependencyCount = 2;
      renderPassInfo.pDependencies = dependencies.data( );
      renderPass = d.createRenderPass( renderPassInfo );

      std::vector<vk::ImageView> attachmentViews;
      for ( auto attachment : attachments )
      {
        attachmentViews.push_back( attachment.view );
      }

      // Find. max number of layers across attachments
      uint32_t maxLayers = 0;
      for ( auto attachment : attachments )
      {
        if ( attachment.subresourceRange.layerCount > maxLayers )
        {
          maxLayers = attachment.subresourceRange.layerCount;
        }
      }

      vk::FramebufferCreateInfo fci;
      fci.setRenderPass( renderPass );
      fci.setAttachmentCount( attachmentViews.size( ) );
      fci.setPAttachments( attachmentViews.data( ) );

      fci.setWidth( width );
      fci.setHeight( height );
      fci.setLayers( maxLayers );

      framebuffer = d.createFramebuffer( fci );
    }
  }
}