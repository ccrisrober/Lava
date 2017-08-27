#include "Descriptor.h"

#include "Device.h"

namespace lava
{
	DescriptorSetLayoutBinding::DescriptorSetLayoutBinding(
		uint32_t binding_, vk::DescriptorType descriptorType_, 
		vk::ShaderStageFlags stageFlags_)
		: binding(binding_)
		, descriptorType(descriptorType_)
		, stageFlags(stageFlags_)
	{}

	DescriptorSetLayoutBinding::DescriptorSetLayoutBinding(DescriptorSetLayoutBinding const& rhs)
		: DescriptorSetLayoutBinding(rhs.binding, rhs.descriptorType, rhs.stageFlags)
	{}

	DescriptorSetLayoutBinding & DescriptorSetLayoutBinding::operator=(DescriptorSetLayoutBinding const& rhs)
	{
		binding = rhs.binding;
		descriptorType = rhs.descriptorType;
		stageFlags = rhs.stageFlags;
		return *this;
	}
	DescriptorSetLayout::DescriptorSetLayout(const std::shared_ptr<Device>& device,
		vk::ArrayProxy<const DescriptorSetLayoutBinding> bindings)
		: VulkanResource( device )
		, _bindings(bindings.begin(), bindings.end())
	{
		std::vector<vk::DescriptorSetLayoutBinding> dslb;
		dslb.reserve(_bindings.size());
		
		vk::DescriptorSetLayoutCreateInfo dci(
			{},
			dslb.size(),
			dslb.data()
		);
		_descriptorSetLayout = static_cast<vk::Device>(*_device).createDescriptorSetLayout(
			dci);
	}

	DescriptorSetLayout::~DescriptorSetLayout()
	{
		static_cast<vk::Device>(*_device).destroyDescriptorSetLayout(_descriptorSetLayout);
	}


	DescriptorPool::DescriptorPool(const std::shared_ptr<Device>& device, 
		vk::DescriptorPoolCreateFlags flags, uint32_t maxSets, 
		vk::ArrayProxy<const vk::DescriptorPoolSize> poolSizes)
		: VulkanResource(device)
	{
		// we need to set this bit to enable free on their descriptor sets
		flags |= vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet;
		vk::DescriptorPoolCreateInfo dci(
			flags, 
			maxSets, 
			poolSizes.size(), 
			poolSizes.data()
		);

		_descriptorPool = static_cast<vk::Device>(*_device).createDescriptorPool(dci);
	}

	DescriptorPool::~DescriptorPool()
	{
		static_cast<vk::Device>(*_device).destroyDescriptorPool(_descriptorPool);
	}

	void DescriptorPool::reset()
	{
		static_cast<vk::Device>(*_device).resetDescriptorPool(_descriptorPool);
	}


}