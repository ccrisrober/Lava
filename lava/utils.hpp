/**
 * Copyright (c) 2017, Lava
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

#ifndef __LAVA_UTILS__
#define __LAVA_UTILS__

#include "includes.hpp"

#include "Device.h"
#include "PhysicalDevice.h"
#include "CommandBuffer.h"
#include "Queue.h"

#include <lava/Image.h>
#include <mutex>

namespace lava
{
	class utils
	{
  public:
  class Lockable
  {
  public:
    inline void lock( void ) { _guard.lock( ); }
    inline bool try_lock( void ) { return _guard.try_lock( ); }
    inline void unlock( void ) { _guard.unlock( ); }

    inline std::mutex& mutex( void ) const { return _guard; }

  protected:
    mutable std::mutex _guard;
  };

  typedef std::lock_guard<Lockable> LockableGuard;
    LAVA_API
    static short channelsFromFormat( const vk::Format& format );
    LAVA_API
    static void saveScreenshot( std::shared_ptr<Device> device,
      const char* filename, uint32_t width, uint32_t height, 
      vk::Format colorFormat, std::shared_ptr<Image> image,
      std::shared_ptr<CommandPool> cmdPool, std::shared_ptr<Queue> queue );

    static VkBool32 getSupportedDepthFormat( 
      std::shared_ptr<PhysicalDevice> physicalDevice,  vk::Format& depthFormat )
    {
      // Since all depth formats may be optional, we need to find a suitable depth format to use
      // Start with the highest precision packed format
      std::vector<vk::Format> depthFormats =
      {
        vk::Format::eD32SfloatS8Uint,
        vk::Format::eD32Sfloat,
        vk::Format::eD24UnormS8Uint,
        vk::Format::eD16UnormS8Uint,
        vk::Format::eD16Unorm
      };

      for ( auto& format : depthFormats )
      {
        auto formatProps = physicalDevice->getFormatProperties( format );
        // Format must support depth stencil attachment for optimal tiling
        if ( formatProps.optimalTilingFeatures & 
          vk::FormatFeatureFlagBits::eDepthStencilAttachment )
        {
          depthFormat = format;
          return true;
        }
      }

      return false;
    }

    static unsigned char* loadImageTexture( const std::string& fileName,
      uint32_t& width, uint32_t& height, uint32_t& numChannels );
    static std::vector<char> readBinaryile( const std::string& fileName );
		static const std::string translateVulkanResult( vk::Result res );

    // Put an image memory barrier for setting an image layout on the 
    //    sub resource into the given command buffer
    LAVA_API
    static void transitionImageLayout( const std::shared_ptr<CommandBuffer>& cmd,
      std::shared_ptr<Image> image,
      vk::ImageLayout oldImageLayout,
      vk::ImageLayout newImageLayout,
      vk::ImageSubresourceRange subresourceRange,
      vk::PipelineStageFlags srcStageMask = 
        vk::PipelineStageFlagBits::eAllCommands,
      vk::PipelineStageFlags dstStageMask = 
        vk::PipelineStageFlagBits::eAllCommands
    );
    // Uses a fixed sub resource layout with first mip level and layer
    LAVA_API
    static void transitionImageLayout( const std::shared_ptr<CommandBuffer>& cmd,
      std::shared_ptr<Image> image,
      vk::ImageAspectFlags aspectMask,
      vk::ImageLayout oldImageLayout,
      vk::ImageLayout newImageLayout,
      vk::PipelineStageFlags srcStageMask = 
        vk::PipelineStageFlagBits::eAllCommands,
      vk::PipelineStageFlags dstStageMask = 
        vk::PipelineStageFlagBits::eAllCommands
    );
    // Insert an image memory barrier into the command buffer
    LAVA_API
    static void insertImageMemoryBarrier(
      const std::shared_ptr<CommandBuffer> cmdbuffer,
      std::shared_ptr<Image> image,
      vk::AccessFlags srcAccessMask, vk::AccessFlags dstAccessMask,
      vk::ImageLayout oldImageLayout, vk::ImageLayout newImageLayout,
      vk::PipelineStageFlags srcStageMask, vk::PipelineStageFlags dstStageMask,
      vk::ImageSubresourceRange subresourceRange
     );
	};
}

#endif /* __LAVA_UTILS__ */