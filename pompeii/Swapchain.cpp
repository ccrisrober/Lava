/**
 * Copyright (c) 2017 - 2018, Pompeii
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

#include "Swapchain.h"
#include <pompeii/Surface.h>
#include <pompeii/PhysicalDevice.h>
#include <pompeii/Log.h>

namespace pompeii
{
  Swapchain::Swapchain( const std::shared_ptr< pompeii::Device >& device,
    const std::shared_ptr< pompeii::Surface >& surface, 
    const vk::Extent2D& desiredExtent )
    : VulkanResource( device )
    , desired_extent( desiredExtent )
  {
    this->_surface = surface;

    createSwapchain( );
    createImageViews( );
  }

  Swapchain::~Swapchain( void )
  {
    cleanup( true );
  }

  void Swapchain::resize( const vk::Extent2D& extent )
  {
    this->desired_extent = extent;
    recreate( );
  }

  vk::ResultValue<uint32_t> Swapchain::acquireNextImage( uint64_t timeout,
    const std::shared_ptr< Fence >& fence )
  {
    vk::ResultValue<uint32_t> result = static_cast< vk::Device >( *_device )
      .acquireNextImageKHR( _swapchain, timeout, *_freeSemaphore,
        fence ? static_cast< vk::Fence >( *fence ) : nullptr );
    assert( result.result == vk::Result::eSuccess );  // need to handle timeout, 
                                                      // not ready, and suboptimal
                                                      // put the semaphore at the correct index and use the 
                                                      // semaphore from the new index as next free semaphore
    std::swap( _freeSemaphore, _presentCompleteSemaphores[ result.value ] );
    return result;
  }

  void Swapchain::recreate( void )
  {
    _device->waitIdle( );
    cleanup( false );
    createSwapchain( );
    createImageViews( );
  }

  void Swapchain::createSwapchain( void )
  {
    auto physicalDevice = _device->getPhysicalDevice( );
    auto surfaceCaps = physicalDevice->getSurfaceCapabilities( _surface );
    auto surfaceFormats = physicalDevice->getSurfaceFormats( _surface );
    auto surfacePresentModes = physicalDevice->getSurfacePresentModes( _surface );

    vk::Extent2D extent;
    // If width/height is 0xFFFFFFFF, we can manually specify width, height
    if ( surfaceCaps.currentExtent.width != std::numeric_limits<uint32_t>::max( ) )
    {
      extent = surfaceCaps.currentExtent;
    }
    else
    {
      vk::Extent2D actualExtent = { 1, 1 };

      actualExtent.width = std::max( surfaceCaps.minImageExtent.width,
        std::min( surfaceCaps.maxImageExtent.width, actualExtent.width ) );
      actualExtent.height = std::max( surfaceCaps.minImageExtent.height,
        std::min( surfaceCaps.maxImageExtent.height, actualExtent.height ) );

      extent = actualExtent;
    }

    desired_extent = extent; // TODO ??

    // Find present mode
    auto presentModes = _device->getPhysicalDevice( )->getSurfacePresentModes( _surface );
    vk::PresentModeKHR presentMode = vk::PresentModeKHR::eFifo;

    bool vsync = false;

    // If v-sync is not requested, try to find a mailbox mode
    // It's the lowest latency non-tearing present mode available
    if ( !vsync )
    {
      for ( size_t i = 0; i < presentModes.size( ); ++i )
      {
        if ( presentModes[ i ] == vk::PresentModeKHR::eMailbox )
        {
          presentMode = vk::PresentModeKHR::eMailbox;
          break;
        }
        if ( ( presentMode != vk::PresentModeKHR::eMailbox ) &&
          ( presentModes[ i ] == vk::PresentModeKHR::eImmediate ) )
        {
          presentMode = vk::PresentModeKHR::eImmediate;
        }
      }
    }

    auto imageCount = surfaceCaps.minImageCount + 1;
    if ( surfaceCaps.maxImageCount > 0 && imageCount > surfaceCaps.maxImageCount )
    {
      imageCount = surfaceCaps.maxImageCount;
    }
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

    _swapchainSupportsReadBack = !!( surfaceCaps.supportedUsageFlags &
      vk::ImageUsageFlagBits::eTransferSrc );

    if ( _swapchainSupportsReadBack )
    {
      usage |= vk::ImageUsageFlagBits::eTransferSrc;
    }

    vk::SwapchainKHR oldSwapchain = _swapchain;

    vk::SurfaceFormatKHR surfaceFormat = surfaceFormats[ 0 ];

    auto sci = vk::SwapchainCreateInfoKHR( )
      .setSurface( *_surface )
      .setMinImageCount( imageCount )
      .setImageFormat( surfaceFormat.format )
      .setImageColorSpace( surfaceFormat.colorSpace )
      .setImageExtent( extent )
      .setImageArrayLayers( 1 )
      .setImageUsage( usage );

    // todo. sharing mode
    sci.setImageSharingMode( vk::SharingMode::eExclusive )
      .setCompositeAlpha( compositeAlpha )
      .setPresentMode( presentMode )
      .setClipped( VK_TRUE )
      .setPreTransform( surfaceTransform )
      .setOldSwapchain( oldSwapchain );

    _swapchain = static_cast< vk::Device >( *_device ).createSwapchainKHR( sci );

    if ( oldSwapchain )
    {
      static_cast< vk::Device >( *_device ).destroySwapchainKHR( oldSwapchain );
    }
    
    _images.clear( );
    _presentCompleteSemaphores.clear( );
    std::vector<vk::Image> images =
      static_cast< vk::Device >( *_device ).getSwapchainImagesKHR( _swapchain );

    size_t numImages = images.size( );
    _images.reserve( numImages );
    _presentCompleteSemaphores.reserve( numImages + 1 );
    for ( size_t i = 0; i < numImages; ++i )
    {
      _images.push_back( std::make_shared<Image>( _device, images[ i ] ) );
      _presentCompleteSemaphores.push_back( _device->createSemaphore( ) );
    }

    _freeSemaphore = _device->createSemaphore( );

    _format = surfaceFormat.format;
  }
  
  void Swapchain::createImageViews( void )
  {
    uint32_t l = _images.size( );
    _imageViews.resize( l );
    for ( uint32_t i = 0; i < l; ++i )
    {
      _imageViews[ i ] = _images[ i ]->createImageView(
        vk::ImageViewType::e2D,
        _format,
        {
          vk::ComponentSwizzle::eR,
          vk::ComponentSwizzle::eG,
          vk::ComponentSwizzle::eB,
          vk::ComponentSwizzle::eA
        },
        { vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 }
      );
    }
  }
  
  void Swapchain::cleanup( bool destroySwapchain )
  {
    Log::info( "Destroying image views" );
    for ( auto& iv : _imageViews )
    {
      iv.reset( );
    }
    _imageViews.clear( );
    for ( auto& sem : _presentCompleteSemaphores )
    {
      sem.reset( );
    }
    _presentCompleteSemaphores.clear( );
    _freeSemaphore.reset( );

    if ( destroySwapchain )
    {
      Log::info( "Destroying swap chain" );
      static_cast< vk::Device >( *_device ).destroySwapchainKHR( _swapchain );
      _swapchain = nullptr;
    }
  }
}