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
  class Swapchain : public VulkanResource
  {
  public:
    LAVA_API
    Swapchain( const std::shared_ptr<Device>& device, 
      const std::shared_ptr< Surface >& surface,
      uint32_t numImageCount, vk::Format imageFormat, vk::ColorSpaceKHR colorSpace,
      const vk::Extent2D& imageExtent, uint32_t imageArrayLayers,
      vk::ImageUsageFlags imageUsage, vk::SharingMode imageSharingMode,
      const std::vector< uint32_t >& queueFamilyIndices,
      vk::SurfaceTransformFlagBitsKHR preTransform,
      vk::CompositeAlphaFlagBitsKHR compAlpha, vk::PresentModeKHR presentMode,
      bool clipped, const std::shared_ptr< Swapchain >& oldSwapchain );
    LAVA_API
    virtual ~Swapchain( void );

    LAVA_API
    const std::vector< std::shared_ptr< Image > >& 
      getImages( void ) const;
    LAVA_API
    const std::vector< std::shared_ptr< Semaphore > >& 
      getPresentCompleteSemaphores( void ) const
    {
      return _presentCompleteSemaphores;
    }

    inline operator vk::SwapchainKHR( void ) const
    {
      return _swapchain;
    }

    LAVA_API
    uint32_t acquireNextImage( uint64_t timeout = UINT64_MAX,
      const std::shared_ptr< Fence >& fence = { } );

  protected:
    std::vector<std::shared_ptr<Image>> _images;
    std::vector<std::shared_ptr<Semaphore>> _presentCompleteSemaphores;
    vk::SwapchainKHR _swapchain;
    std::shared_ptr<Semaphore> _freeSemaphore;
  };
}

#endif /* __LAVA_SWAPCHAIN__ */