#ifndef __LAVA_UTILS__
#define __LAVA_UTILS__

#include "includes.hpp"

#include "CommandBuffer.h"

namespace lava
{
	class utils
	{
  public:
    static unsigned char* loadImageTexture( const std::string& fileName,
      uint32_t& width, uint32_t& height, uint32_t& numChannels );
    static std::vector<char> readBinaryile( const std::string& fileName );
		static const std::string translateVulkanResult( vk::Result res );

    // Put an image memory barrier for setting an image layout on the 
    //    sub resource into the given command buffer
    LAVA_API
    static void setImageLayout( const std::shared_ptr<CommandBuffer>& cmd,
      vk::Image image,
      vk::ImageLayout oldImageLayout,
      vk::ImageLayout newImageLayout,
      vk::ImageSubresourceRange subresourceRange,
      vk::PipelineStageFlags srcStageMask = vk::PipelineStageFlagBits::eAllCommands,
      vk::PipelineStageFlags dstStageMask = vk::PipelineStageFlagBits::eAllCommands );
    // Uses a fixed sub resource layout with first mip level and layer
    LAVA_API
    static void setImageLayout( const std::shared_ptr<CommandBuffer>& cmd,
      vk::Image image,
      vk::ImageAspectFlags aspectMask,
      vk::ImageLayout oldImageLayout,
      vk::ImageLayout newImageLayout,
      vk::PipelineStageFlags srcStageMask = vk::PipelineStageFlagBits::eAllCommands,
      vk::PipelineStageFlags dstStageMask = vk::PipelineStageFlagBits::eAllCommands );
	};
}

#endif /* __LAVA_UTILS__ */