#include "CustomFramebuffer.h"
#include "../Device.h"
#include "../PhysicalDevice.h"

namespace lava
{
  namespace extras
  {
    CustomFBO::CustomFBO( DeviceRef dev, uint32_t width, uint32_t height )
      : VulkanResource( dev )
      , _width( width )
      , _height( height )
    {
    }
    void CustomFBO::addColorAttachmentt( vk::Format format )
    {
      FramebufferAttachment att;
      createAttachment( att, format, vk::ImageUsageFlagBits::eColorAttachment );
      att.finalLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
      _colorAttachments.push_back( att );
    }
    void CustomFBO::addColorDepthAttachment( vk::Format format )
    {
      /*FramebufferAttachment att;
      createAttachment( att, format, vk::ImageUsageFlagBits::eDepthStencilAttachment |
        vk::ImageUsageFlagBits::eSampled );
      att.finalLayout = vk::ImageLayout::eDepthStencilReadOnlyOptimal;
      _colorAttachments.push_back( att );*/
      if ( hasDepth ) throw;
      createAttachment( _depthAttachment, format, 
        vk::ImageUsageFlagBits::eDepthStencilAttachment |
        vk::ImageUsageFlagBits::eSampled );
      hasDepth = true;
    }
    void CustomFBO::addDepthAttachment( vk::Format format )
    {
      if ( hasDepth ) throw;
      createAttachment( _depthAttachment, format, 
        vk::ImageUsageFlagBits::eDepthStencilAttachment );
      hasDepth = true;
    }
    void CustomFBO::build( void )
    {
      std::vector<std::shared_ptr<ImageView>> imageViewVector;
      std::vector<vk::AttachmentDescription> attDesc; // ( _colorAttachments.size( ) + 1 );
      for ( auto& att : _colorAttachments )
      {
        attDesc.push_back( vk::AttachmentDescription(
        {}, att.format, vk::SampleCountFlagBits::e1,
          vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eStore,
          vk::AttachmentLoadOp::eDontCare, vk::AttachmentStoreOp::eDontCare,
          vk::ImageLayout::eUndefined, att.finalLayout
        ) );

        imageViewVector.push_back( att.view );
      }
      if ( hasDepth )
      {
        attDesc.push_back( vk::AttachmentDescription(
        {}, _depthAttachment.format, vk::SampleCountFlagBits::e1,
          vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eStore,
          vk::AttachmentLoadOp::eDontCare, vk::AttachmentStoreOp::eDontCare,
          vk::ImageLayout::eUndefined, vk::ImageLayout::eDepthStencilReadOnlyOptimal
        ) );
        imageViewVector.push_back( _depthAttachment.view );
      }


      std::vector<vk::AttachmentReference> colorAttachments;
      for ( uint32_t i = 0, l = _colorAttachments.size( ); i < l; ++i )
      {
        colorAttachments.push_back( { i, vk::ImageLayout::eColorAttachmentOptimal } );
      }

      vk::SubpassDescription subpass;
      subpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
      subpass.colorAttachmentCount = colorAttachments.size( );
      subpass.pColorAttachments = colorAttachments.data( );

      if ( hasDepth )
      {
        vk::AttachmentReference depthRef( colorAttachments.size( ),
          vk::ImageLayout::eDepthStencilAttachmentOptimal );
        subpass.pDepthStencilAttachment = &depthRef;
      }

      // Use subpass dependencies for attachment layput transitions
      std::array<vk::SubpassDependency, 2> dependencies;

      dependencies[ 0 ].srcSubpass = VK_SUBPASS_EXTERNAL;
      dependencies[ 0 ].dstSubpass = 0;
      dependencies[ 0 ].srcStageMask = vk::PipelineStageFlagBits::eBottomOfPipe;
      dependencies[ 0 ].dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
      dependencies[ 0 ].srcAccessMask = vk::AccessFlagBits::eMemoryRead;
      dependencies[ 0 ].dstAccessMask = vk::AccessFlagBits::eColorAttachmentRead
        | vk::AccessFlagBits::eColorAttachmentWrite;
      dependencies[ 0 ].dependencyFlags = vk::DependencyFlagBits::eByRegion;

      dependencies[ 1 ].srcSubpass = 0;
      dependencies[ 1 ].dstSubpass = VK_SUBPASS_EXTERNAL;
      dependencies[ 1 ].srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
      dependencies[ 1 ].dstStageMask = vk::PipelineStageFlagBits::eBottomOfPipe;
      dependencies[ 1 ].srcAccessMask = vk::AccessFlagBits::eColorAttachmentRead
        | vk::AccessFlagBits::eColorAttachmentWrite;
      dependencies[ 1 ].dstAccessMask = vk::AccessFlagBits::eMemoryRead;
      dependencies[ 1 ].dependencyFlags = vk::DependencyFlagBits::eByRegion;

      renderPass = _device->createRenderPass( attDesc, subpass, dependencies );

      _fbo = _device->createFramebuffer( renderPass,
        imageViewVector, { _width, _height }, 1
      );



      // Create sampler to sample from the color attachments
      vk::SamplerCreateInfo sampler;
      sampler.magFilter = vk::Filter::eNearest;
      sampler.minFilter = vk::Filter::eNearest;
      sampler.mipmapMode = vk::SamplerMipmapMode::eLinear;
      sampler.addressModeU = vk::SamplerAddressMode::eClampToEdge;
      sampler.addressModeV = sampler.addressModeU;
      sampler.addressModeW = sampler.addressModeU;
      sampler.mipLodBias = 0.0f;
      sampler.maxAnisotropy = 1.0f;
      sampler.minLod = 0.0f;
      sampler.maxLod = 1.0f;
      sampler.borderColor = vk::BorderColor::eFloatOpaqueWhite;

      vk::Device device = static_cast< vk::Device >( *_device );

      colorSampler = device.createSampler( sampler );

      semaphore = _device->createSemaphore( );
    }
    void CustomFBO::createAttachment( FramebufferAttachment& fatt, 
      vk::Format format, vk::ImageUsageFlags usage )
    {
      vk::ImageAspectFlags aspectMask;
      vk::ImageLayout imageLayout;

      fatt.format = format;

      if ( usage & vk::ImageUsageFlagBits::eColorAttachment )
      {
        aspectMask = vk::ImageAspectFlagBits::eColor;
        imageLayout = vk::ImageLayout::eColorAttachmentOptimal;
      }
      if ( usage & vk::ImageUsageFlagBits::eDepthStencilAttachment )
      {
        aspectMask = vk::ImageAspectFlagBits::eDepth
          | vk::ImageAspectFlagBits::eStencil;
        imageLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;
      }
      // For only depth textures ( shadow mapping )
      if ( usage & (vk::ImageUsageFlagBits::eDepthStencilAttachment | 
        vk::ImageUsageFlagBits::eSampled ) )
      {
        aspectMask = vk::ImageAspectFlagBits::eDepth;
        imageLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;
      }

      vk::ImageCreateInfo ici;
      ici.imageType = vk::ImageType::e2D;
      ici.format = format;
      ici.extent.width = _width;
      ici.extent.height = _height;
      ici.mipLevels = 1;
      ici.extent.depth = 1;
      ici.arrayLayers = 1;
      ici.samples = vk::SampleCountFlagBits::e1;
      ici.tiling = vk::ImageTiling::eOptimal;
      ici.usage = usage | vk::ImageUsageFlagBits::eSampled;

      fatt.image = _device->createImage( {}, ici.imageType, ici.format,
        ici.extent, 1, 1, ici.samples,
        ici.tiling, ici.usage,
        ici.sharingMode, {}, ici.initialLayout, {} );

      vk::ImageSubresourceRange isr;
      isr.aspectMask = aspectMask;
      isr.baseMipLevel = 0;
      isr.levelCount = 1;
      isr.baseArrayLayer = 0;
      isr.layerCount = 1;
      fatt.view = fatt.image->createImageView( vk::ImageViewType::e2D, format,
        vk::ComponentMapping{ vk::ComponentSwizzle::eR,
        vk::ComponentSwizzle::eG,
        vk::ComponentSwizzle::eB,
        vk::ComponentSwizzle::eA }, isr );
    }
  }
}