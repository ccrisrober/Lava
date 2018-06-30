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

#include "DefaultFramebuffer.h"

#include "PhysicalDevice.h"

namespace lava
{
  DefaultFramebuffer::DefaultFramebuffer( const std::shared_ptr<Device>& device,
    const std::shared_ptr<Surface>& surface, vk::Format surfaceFormat,
    vk::ColorSpaceKHR colorSpace, vk::Format depthFormat, 
    const std::shared_ptr<RenderPass>& renderPass )
    : _swapchainIndex( 0 )
  {
    vk::SurfaceCapabilitiesKHR surfaceCaps = 
      device->getPhysicalDevice( )->getSurfaceCapabilities( surface );

    // If width/height is 0xFFFFFFFF, we can manually specify width, height
    if ( surfaceCaps.currentExtent.width != std::numeric_limits<uint32_t>::max( ) )
    {
      _extent = surfaceCaps.currentExtent;
    }
    else
    {
      vk::Extent2D actualExtent = { 1, 1 };

      actualExtent.width = std::max( surfaceCaps.minImageExtent.width,
        std::min( surfaceCaps.maxImageExtent.width, actualExtent.width ) );
      actualExtent.height = std::max( surfaceCaps.minImageExtent.height,
        std::min( surfaceCaps.maxImageExtent.height, actualExtent.height ) );

      _extent = actualExtent;
    }

    // Find present mode
    auto presentModes = vk::PhysicalDevice(
      *device->getPhysicalDevice( ) ).getSurfacePresentModesKHR( *surface );
    vk::PresentModeKHR presentMode = vk::PresentModeKHR::eFifo;

    /*bool vsync = true;

    if ( !vsync )
    {
      bool founded = false;
      for ( const auto& pr : presentModes )
      {
        if ( pr == vk::PresentModeKHR::eImmediate )
        {
          presentMode = vk::PresentModeKHR::eImmediate;
          std::cout << "Selecting INMEDIATE mode" << std::endl;
          founded = true;
          break;
        }

        if ( pr == vk::PresentModeKHR::eFifoRelaxed )
        {
          presentMode = vk::PresentModeKHR::eFifoRelaxed;
        }
      }
      if ( !founded && presentMode == vk::PresentModeKHR::eFifo )
      {
        std::cout << "Selecting default FIFO mode" << std::endl;
      }
    }
    else
    {
      // Mailbox comes with lower input latency than FIFO, but can waste GPU
      // power by rendering frames that are never displayed, especially if the
      // app runs much faster than the refresh rate. This is a concern for mobiles.
      for ( const auto& pr : presentModes )
      {
        if ( pr == vk::PresentModeKHR::eMailbox )
        {
          presentMode = vk::PresentModeKHR::eMailbox;
          break;
        }
      }
    }*/
    presentModes.clear( );

    uint32_t numImages = surfaceCaps.minImageCount;

    vk::SurfaceTransformFlagBitsKHR surfaceTransform =
      ( surfaceCaps.supportedTransforms & 
        vk::SurfaceTransformFlagBitsKHR::eIdentity ) ?
      vk::SurfaceTransformFlagBitsKHR::eIdentity :
      surfaceCaps.currentTransform;

    // Find a supported composite alpha format (not all devices support alpha opaque)
    vk::CompositeAlphaFlagBitsKHR compositeAlpha = 
      vk::CompositeAlphaFlagBitsKHR::eOpaque;
    // Simply select the first composite alpha format available
    std::vector<vk::CompositeAlphaFlagBitsKHR> compositeAlphaFlags =
    {
      vk::CompositeAlphaFlagBitsKHR::eOpaque,
      vk::CompositeAlphaFlagBitsKHR::ePreMultiplied,
      vk::CompositeAlphaFlagBitsKHR::ePostMultiplied,
      vk::CompositeAlphaFlagBitsKHR::eInherit,
    };
    for ( auto& compositeAlphaFlag : compositeAlphaFlags )
    {
      if ( surfaceCaps.supportedCompositeAlpha & compositeAlphaFlag )
      {
        compositeAlpha = compositeAlphaFlag;
        break;
      };
    }

    vk::ImageUsageFlags usage = vk::ImageUsageFlagBits::eColorAttachment;

    _swapChainSupportsReadBack = !!( surfaceCaps.supportedUsageFlags & 
                  vk::ImageUsageFlagBits::eTransferSrc );

    if ( _swapChainSupportsReadBack )
    {
      usage |= vk::ImageUsageFlagBits::eTransferSrc;
    }

    _swapchain = device->createSwapchain(
      surface, numImages, surfaceFormat, colorSpace, _extent, 1, usage,
      vk::SharingMode::eExclusive, { }, surfaceTransform, compositeAlpha, 
      presentMode, true, nullptr
    );

    _colorImages = _swapchain->getImages( );

    _colorViews.reserve( _colorImages.size( ) );
    for ( uint32_t i = 0; i < _colorImages.size( ); ++i )
    {
      _colorViews.push_back(
        _colorImages[ i ]->createImageView(
          vk::ImageViewType::e2D,
          surfaceFormat,
          {
            vk::ComponentSwizzle::eR,
            vk::ComponentSwizzle::eG,
            vk::ComponentSwizzle::eB,
            vk::ComponentSwizzle::eA
          },
          { vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 }
        )
      );
    }

    // depth/stencil buffer
    // assert that a depth and/or stencil format is requested
    vk::FormatProperties formatProps = 
      device->getPhysicalDevice( )->getFormatProperties( depthFormat );
    assert( ( formatProps.linearTilingFeatures & 
      vk::FormatFeatureFlagBits::eDepthStencilAttachment ) ||
      ( formatProps.optimalTilingFeatures & 
        vk::FormatFeatureFlagBits::eDepthStencilAttachment ) );

    vk::ImageTiling tiling = ( formatProps.optimalTilingFeatures 
      & vk::FormatFeatureFlagBits::eDepthStencilAttachment )
      ? vk::ImageTiling::eOptimal : vk::ImageTiling::eLinear;

    _depthImage = device->createImage(
    {}, vk::ImageType::e2D, depthFormat,
      vk::Extent3D( _extent.width, _extent.height, 1 ), 1, 1,
      vk::SampleCountFlagBits::e1, tiling,
      vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eTransferSrc,
      vk::SharingMode::eExclusive, {},
      vk::ImageLayout::eUndefined, {} );

    // determine ImageAspect based on format
    vk::ImageAspectFlags aspectFlags;
    if ( depthFormat != vk::Format::eS8Uint )
    {
      aspectFlags |= vk::ImageAspectFlagBits::eDepth;
    }

    // add eStencil if image contains stencil
    static std::initializer_list<vk::Format> const stencilFormats{ 
      vk::Format::eD16UnormS8Uint, vk::Format::eD24UnormS8Uint, 
      vk::Format::eD32SfloatS8Uint, vk::Format::eS8Uint };
    if ( std::find( stencilFormats.begin( ), stencilFormats.end( ), 
      depthFormat ) != stencilFormats.end( ) )
    {
      aspectFlags |= vk::ImageAspectFlagBits::eStencil;
    }

    _depthView = _depthImage->createImageView(
      vk::ImageViewType::e2D, depthFormat,
      vk::ComponentMapping(
        vk::ComponentSwizzle::eR, vk::ComponentSwizzle::eG,
        vk::ComponentSwizzle::eB, vk::ComponentSwizzle::eA ),
      vk::ImageSubresourceRange( aspectFlags, 0, 1, 0, 1 ) );

    _framebuffers.reserve( _colorViews.size( ) );
    for ( size_t i = 0; i < _colorViews.size( ); ++i )
    {
      _framebuffers.push_back( device->createFramebuffer( renderPass, 
      { _colorViews[ i ], 
        _depthView }, _extent, 1 ) );
    }

    std::cout << "Framebuffer Swapchain OK" << std::endl;
  }

  void DefaultFramebuffer::rebuild( const std::shared_ptr<Device>&,
    const std::shared_ptr<Surface>&,
    vk::Format, vk::Format,
    const std::shared_ptr<RenderPass>& )
  {
    // TODO
  }
  void DefaultFramebuffer::clear( vk::SwapchainKHR swapChain )
  {
    if ( swapChain )
    {
      // TODO
    }
  }
}