/**
 * Copyright (c) 2017 - 2018, Lava
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
      uint32_t height_, vk::Format format_, vk::ImageUsageFlags usage_, 
      uint32_t numLayers )
    {
      FramebufferAttachment att;
      att.format = format_;

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
        if ( att.hasDepth( ) )
        {
          aspectMask = vk::ImageAspectFlagBits::eDepth;
        }
        if ( att.hasStencil( ) )
        {
          aspectMask = aspectMask | vk::ImageAspectFlagBits::eStencil;
        }
      }

      assert( aspectMask != vk::ImageAspectFlagBits( ) );

      vk::ImageCreateInfo ici;
      ici.extent.width = width_;
      ici.extent.height = height_;
      ici.extent.depth = 1;

      att.image = _device->createImage( { }, vk::ImageType::e2D, format_,
        ici.extent, 1, numLayers, vk::SampleCountFlagBits::e1, 
        vk::ImageTiling::eOptimal, usage_, ici.sharingMode, { }, 
        ici.initialLayout, { }
      );

      att.subresourceRange.aspectMask = aspectMask;
      att.subresourceRange.baseMipLevel = 0;
      att.subresourceRange.levelCount = 1;
      att.subresourceRange.baseArrayLayer = 0;
      att.subresourceRange.layerCount = numLayers;

      vk::ImageViewType imageViewType =
        ( numLayers == 1 ) ? vk::ImageViewType::e2D : vk::ImageViewType::e2DArray;

      att.imageView = att.image->createImageView(
        imageViewType, format_, vk::ComponentMapping{
          vk::ComponentSwizzle::eR, vk::ComponentSwizzle::eG,
          vk::ComponentSwizzle::eB, vk::ComponentSwizzle::eA
        }, att.subresourceRange
      );

      // Fill attachment description
      att.description.samples = vk::SampleCountFlagBits::e1;
      att.description.loadOp = vk::AttachmentLoadOp::eClear;
      att.description.storeOp =
        ( usage_ & vk::ImageUsageFlagBits::eSampled ) ?
        vk::AttachmentStoreOp::eStore : vk::AttachmentStoreOp::eDontCare;
      att.description.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
      att.description.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
      att.description.format = format_;
      att.description.initialLayout = vk::ImageLayout::eUndefined;

      // Final layout
      // If not, final layout depends on attachment type
      if ( att.hasDepth( ) || att.hasStencil( ) )
      {
        att.description.finalLayout = vk::ImageLayout::eDepthStencilReadOnlyOptimal;
      }
      else
      {
        att.description.finalLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
      }

      attachments.push_back( att );

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
      uint32_t maxLayers = 0;
      for ( const auto& att : attachments )
      {
        if ( att.subresourceRange.layerCount > maxLayers )
        {
          maxLayers = att.subresourceRange.layerCount;
        }
      }

      _framebuffer = _device->createFramebuffer( _renderPass, attViews, 
        vk::Extent2D{ _width, _height }, maxLayers );
    }
  }
}
