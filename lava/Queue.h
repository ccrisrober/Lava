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

#ifndef __LAVA_QUEUE__
#define __LAVA_QUEUE__

#include "includes.hpp"
#include <vector>
#include <map>

#include "VulkanResource.h"

#include "noncopyable.hpp"

#include <lava/api.h>

namespace lava
{
  class CommandBuffer;
  class Swapchain;
}

namespace lava
{
  struct SubmitInfo
  {
    LAVA_API
    SubmitInfo(
      vk::ArrayProxy<const std::shared_ptr<Semaphore>> const& waitSemaphores,
      vk::ArrayProxy<const vk::PipelineStageFlags> waitDstStageMasks,
      vk::ArrayProxy<const std::shared_ptr<CommandBuffer>> const& commandBuffers,
      vk::ArrayProxy<const std::shared_ptr<Semaphore>> signalSemaphores
    );
    LAVA_API
    SubmitInfo( SubmitInfo const& rhs );
    LAVA_API
    SubmitInfo & operator=( SubmitInfo const& rhs );

    std::vector< std::shared_ptr< Semaphore > > waitSemaphores;
    std::vector< vk::PipelineStageFlags > waitDstStageMasks;
    std::vector< std::shared_ptr< CommandBuffer > > commandBuffers;
    std::vector< std::shared_ptr< Semaphore > > signalSemaphores;
  };

  class Queue
    : public VulkanResource
    , private NonCopyable<Queue>
  {
  public:
    LAVA_API
    vk::Result submit( vk::ArrayProxy<const SubmitInfo> submitInfos,
      const std::shared_ptr<Fence>& fence = std::shared_ptr<Fence>( ) );

    LAVA_API
    void submit( const std::shared_ptr<CommandBuffer>& commandBuffer,
      const std::shared_ptr<Fence>& fence = std::shared_ptr<Fence>( ) );

    LAVA_API
    std::vector<vk::Result> present(
      vk::ArrayProxy<const std::shared_ptr<Semaphore>> waitSemaphores,
      vk::ArrayProxy<const std::shared_ptr<Swapchain>> swapchains,
      vk::ArrayProxy<const uint32_t> imageIndices,
      const vk::DisplayPresentInfoKHR* displayPresentInfo = nullptr );

    LAVA_API
    std::vector<vk::Result> presentToDisplay(
      vk::ArrayProxy<const std::shared_ptr<Semaphore>> waitSemaphores,
      vk::ArrayProxy<const std::shared_ptr<Swapchain>> swapchains,
      vk::ArrayProxy<const uint32_t> imageIndices,
      const vk::Rect2D& srcRect, const vk::Rect2D& dstRect, 
      bool persistent );

    LAVA_API
    void waitIdle( void );

    LAVA_API
    void submitAndWait( std::shared_ptr<CommandBuffer>& cmd );

    LAVA_API
    inline operator vk::Queue( void ) const
    {
      return _queue;
    }

    LAVA_API
    inline uint32_t getQueueFamilyIndex( void ) const
    {
      return _queueFamilyIndex;
    }
  protected:
    friend class Device;
    Queue( const std::shared_ptr<Device>& device, vk::Queue queue, 
      uint32_t queueIndex );

    std::map<std::shared_ptr<Fence>, std::vector<SubmitInfo>> _submitInfos;
    vk::Queue _queue;
    uint32_t _queueFamilyIndex;
  };
}

#endif /* __LAVA_QUEUE__ */