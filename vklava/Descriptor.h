#pragma once

#include "includes.hpp"

#include "VulkanResource.h"
#include "noncopyable.hpp"

namespace lava
{
	class Device;
	struct DescriptorSetLayoutBinding
	{
		DescriptorSetLayoutBinding(uint32_t binding, 
			vk::DescriptorType descriptorType, 
			vk::ShaderStageFlags stageFlags);
		DescriptorSetLayoutBinding(DescriptorSetLayoutBinding const& rhs);
		DescriptorSetLayoutBinding & operator=(DescriptorSetLayoutBinding const& rhs);

		uint32_t binding;
		vk::DescriptorType descriptorType;
		vk::ShaderStageFlags stageFlags;
	};
	
	class DescriptorSetLayout: public VulkanResource, 
		private NonCopyable<DescriptorSetLayout>
	{
	public:
		DescriptorSetLayout(const std::shared_ptr<Device>& device, 
			vk::ArrayProxy<const DescriptorSetLayoutBinding> bindings);
		~DescriptorSetLayout();

		inline operator vk::DescriptorSetLayout() const
		{
			return _descriptorSetLayout;
		}
	private:
		vk::DescriptorSetLayout _descriptorSetLayout;
		std::vector<DescriptorSetLayoutBinding> _bindings;
	};
	class DescriptorPool : public VulkanResource, private NonCopyable<DescriptorPool>
	{
	public:
		DescriptorPool(std::shared_ptr<Device> const & device, vk::DescriptorPoolCreateFlags flags, uint32_t maxSets, vk::ArrayProxy<const vk::DescriptorPoolSize> poolSizes);
		~DescriptorPool();

		void reset();

		inline operator vk::DescriptorPool() const
		{
			return _descriptorPool;
		}

	private:
		vk::DescriptorPool  _descriptorPool;
	};

}