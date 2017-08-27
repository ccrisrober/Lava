#pragma once

#include "includes.hpp"
#include "VulkanResource.h"

namespace lava
{
	class Device;
	class RenderPass: public VulkanResource
	{
	public:
		RenderPass(const std::shared_ptr<Device>& device, const std::vector<vk::AttachmentDescription>& attachments, 
			const std::vector<vk::SubpassDescription>& subpasses, const std::vector<vk::SubpassDependency>& dependencies);
		~RenderPass();

		inline operator vk::RenderPass() const
		{
			return _renderPass;
		}

	protected:
		vk::RenderPass _renderPass;
	};
}