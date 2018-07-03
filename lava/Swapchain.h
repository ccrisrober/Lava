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

#ifndef __LAVA_SWAPCHAIN__
#define __LAVA_SWAPCHAIN__

#include "includes.hpp"
#include "Image.h"
#include "Semaphore.h"
#include "VulkanResource.h"

#include <lava/Fence.h>

namespace lava
{
  class Surface;
}

namespace lava
{
  class Swapchain: public VulkanResource
  {
  public:
    LAVA_API
    Swapchain( const std::shared_ptr< lava::Device >& device,
      const std::shared_ptr< lava::Surface >& surface, 
      const vk::Extent2D& desiredExtent );

    LAVA_API
    ~Swapchain( void );

    LAVA_API
    inline operator vk::SwapchainKHR( void ) const
    {
      return _swapchain;
    }

    LAVA_API
    void resize( const vk::Extent2D& extent );

    LAVA_API
    const vk::Extent2D& extent( void ) const
    {
      return desired_extent;
    }

    LAVA_API
    vk::Format colorFormat( void ) const
    {
      return _format;
    }

    LAVA_API
    vk::ResultValue<uint32_t> acquireNextImage( uint64_t timeout = UINT64_MAX,
      const std::shared_ptr< Fence >& fence = { } );

    LAVA_API
    const std::vector< std::shared_ptr< Semaphore > >&
      getPresentCompleteSemaphores( void ) const
    {
      return _presentCompleteSemaphores;
    }

    LAVA_API
    const std::vector< std::shared_ptr< Image > >& images( void ) const
    {
      return this->_images;
    }
    
    LAVA_API
    const std::vector< std::shared_ptr< ImageView> >& imageViews( void ) const
    {
      return this->_imageViews;
    }
    
    LAVA_API
    size_t count( void ) const
    {
      return this->_images.size( );
    }
    
    LAVA_API
    void recreate( void );

    LAVA_API
    bool swapchainSupportsReadBack( void ) const
    {
      return _swapchainSupportsReadBack;
    }
  private:
    void createSwapchain( void );
    
    void createImageViews( void );
    
    void cleanup( bool destroySwapchain );
  protected:
    std::shared_ptr< lava::Surface > _surface;
    
    vk::SwapchainKHR _swapchain;
    vk::Extent2D desired_extent;
    vk::Format _format;
    bool _swapchainSupportsReadBack;

    std::vector< std::shared_ptr< Image > > _images;
    std::vector< std::shared_ptr< ImageView > > _imageViews;

    std::vector< std::shared_ptr< lava::Semaphore > > _presentCompleteSemaphores;
    std::shared_ptr< lava::Semaphore > _freeSemaphore;
  };
}

#endif /* __LAVA_SWAPCHAIN__ */