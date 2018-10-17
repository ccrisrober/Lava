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

#ifndef __POMPEII_SWAPCHAIN__
#define __POMPEII_SWAPCHAIN__

#include "includes.hpp"
#include "Image.h"
#include "Semaphore.h"
#include "VulkanResource.h"

#include <pompeii/Fence.h>

#include "utilities.hpp"

namespace pompeii
{
  class Surface;
}

namespace pompeii
{
  class Swapchain:
    public VulkanResource,
    public utilities::NonCopyable
  {
  public:
    POMPEII_API
    Swapchain( const std::shared_ptr< pompeii::Device >& device,
      const std::shared_ptr< pompeii::Surface >& surface, 
      const vk::Extent2D& desiredExtent );

    POMPEII_API
    ~Swapchain( void );

    POMPEII_API
    inline operator vk::SwapchainKHR( void ) const
    {
      return _swapchain;
    }

    POMPEII_API
    void resize( const vk::Extent2D& extent );

    POMPEII_API
    const vk::Extent2D& extent( void ) const
    {
      return desired_extent;
    }

    POMPEII_API
    vk::Format colorFormat( void ) const
    {
      return _format;
    }

    POMPEII_API
    vk::ResultValue<uint32_t> acquireNextImage( uint64_t timeout = UINT64_MAX,
      const std::shared_ptr< Fence >& fence = { } );

    POMPEII_API
    const std::vector< std::shared_ptr< Semaphore > >&
      getPresentCompleteSemaphores( void ) const
    {
      return _presentCompleteSemaphores;
    }

    POMPEII_API
    const std::vector< std::shared_ptr< Image > >& images( void ) const
    {
      return this->_images;
    }
    
    POMPEII_API
    const std::vector< std::shared_ptr< ImageView> >& imageViews( void ) const
    {
      return this->_imageViews;
    }
    
    POMPEII_API
    size_t count( void ) const
    {
      return this->_images.size( );
    }
    
    POMPEII_API
    void recreate( void );

    POMPEII_API
    bool swapchainSupportsReadBack( void ) const
    {
      return _swapchainSupportsReadBack;
    }
  private:
    void createSwapchain( void );
    
    void createImageViews( void );
    
    void cleanup( bool destroySwapchain );
  protected:
    std::shared_ptr< pompeii::Surface > _surface;
    
    vk::SwapchainKHR _swapchain;
    vk::Extent2D desired_extent;
    vk::Format _format;
    bool _swapchainSupportsReadBack;

    std::vector< std::shared_ptr< Image > > _images;
    std::vector< std::shared_ptr< ImageView > > _imageViews;

    std::vector< std::shared_ptr< pompeii::Semaphore > > _presentCompleteSemaphores;
    std::shared_ptr< pompeii::Semaphore > _freeSemaphore;
  };
}

#endif /* __POMPEII_SWAPCHAIN__ */