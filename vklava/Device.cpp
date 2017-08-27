#include "Device.h"
#include "PhysicalDevice.h"

#include <assert.h>

namespace lava
{
	uint32_t findMemoryType(const vk::PhysicalDeviceMemoryProperties& memoryProperties, uint32_t requirementBits,
		vk::MemoryPropertyFlags wantedFlags)
	{
		for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; ++i)
		{
			if (requirementBits & (1 << i))
			{
				if ((memoryProperties.memoryTypes[i].propertyFlags &
					wantedFlags) == wantedFlags)
					return i;
			}
		}

		return -1;
	}
	std::shared_ptr<Device> Device::create(const std::shared_ptr<PhysicalDevice>& phyDev,
		const std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos,
		const std::vector<std::string>& enabledLayerNames,
		const std::vector<std::string>& enabledExtensionNames,
		const vk::PhysicalDeviceFeatures& enabledFeatures)
	{
		std::shared_ptr<Device> dev = std::make_shared<Device>(phyDev);
		dev->init(
			queueCreateInfos, 
			enabledLayerNames, enabledExtensionNames, enabledFeatures);

		return dev;
	}
	Device::Device(const std::shared_ptr<PhysicalDevice>& phyDev)
		: _physicalDevice( phyDev )
	{

	}
	Device::~Device(void )
	{
		//_queues.clear();
		_device.destroy();
	}
	void Device::init(
		const std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos,
		const std::vector<std::string>& enabledLayerNames,
		const std::vector<std::string>& enabledExtensionNames,
		const vk::PhysicalDeviceFeatures& enabledFeatures)
	{
		std::vector<vk::DeviceQueueCreateInfo> queueCIs;
		queueCIs.reserve(queueCreateInfos.size());
		for (auto const& ci : queueCreateInfos)
		{
			queueCIs.push_back(ci);
		}

		std::vector<char const*> layers;
		layers.reserve(enabledLayerNames.size());
		for (auto const& s : enabledLayerNames)
		{
			layers.push_back(s.c_str());
		}
		std::vector<const char*> extensions;
		extensions.reserve(enabledExtensionNames.size());
		for (auto const& s : enabledExtensionNames)
		{
			extensions.push_back(s.c_str());
		}
		vk::DeviceCreateInfo dci(
			{},
			queueCIs.size(),
			queueCIs.data(),
			layers.size(),
			layers.data(),
			extensions.size(),
			extensions.data(),
			&enabledFeatures);
		_device = vk::PhysicalDevice(*_physicalDevice).createDevice(dci);

		for (auto const& createInfo : queueCreateInfos)
		{
			std::vector<std::unique_ptr<Queue>> queues;
			for (uint32_t queueIndex = 0; queueIndex < static_cast<uint32_t>(createInfo.queueCount); ++queueIndex)
			{
				vk::Queue queue = _device.getQueue(createInfo.queueFamilyIndex, queueIndex);
				queues.push_back(std::move(std::unique_ptr<Queue>(new Queue(shared_from_this(), queue))));
			}
			auto it = _queues.emplace(createInfo.queueFamilyIndex, std::move(queues));
			assert(it.second && "duplicate queueFamilyIndex");
		}
		std::cout << "Device OK" << std::endl;
	}
	std::shared_ptr<Semaphore> Device::createSemaphore(void)
	{
		return std::make_shared<Semaphore>(shared_from_this());
	}

	std::shared_ptr<RenderPass> Device::createRenderPass(const std::vector<vk::AttachmentDescription>& attachments, 
		const std::vector<vk::SubpassDescription>& subpasses, const std::vector<vk::SubpassDependency>& dependencies)
	{
		return std::make_shared<RenderPass>(shared_from_this(), attachments, subpasses, dependencies);
	}

	std::shared_ptr<Queue> Device::getQueue(uint32_t familyIndex, uint32_t queueIndex)
	{
		auto it = _queues.find(familyIndex);
		assert(it != _queues.end() && "invalid queue family index");
		assert(queueIndex < it->second.size());

		return std::shared_ptr<Queue>(shared_from_this(), it->second[queueIndex].get());
	}

	std::shared_ptr<Swapchain> Device::createSwapchain(const std::shared_ptr<Surface>& surface,
		uint32_t numImageCount, vk::Format imageFormat, const vk::Extent2D& imageExtent,
		uint32_t imageArrayLayers, vk::ImageUsageFlags imageUsage, vk::SharingMode imageSharingMode,
		const std::vector<uint32_t>& queueFamilyIndices,
		vk::SurfaceTransformFlagBitsKHR preTransform, vk::CompositeAlphaFlagBitsKHR compositeAlpha,
		vk::PresentModeKHR presentMode, bool clipped, const std::shared_ptr<Swapchain>& oldSwapchain)
	{
		return std::make_shared<Swapchain>(shared_from_this(), surface, numImageCount, imageFormat, imageExtent,
			imageArrayLayers, imageUsage, imageSharingMode, queueFamilyIndices, preTransform,
			compositeAlpha, presentMode, clipped, oldSwapchain);
	}
	std::shared_ptr<Image> Device::createImage(vk::ImageCreateFlags createFlags, vk::ImageType type, vk::Format format, 
		const vk::Extent3D & extent, uint32_t mipLevels, uint32_t arraySize, vk::SampleCountFlagBits samples, vk::ImageTiling tiling,
		vk::ImageUsageFlags usageFlags, vk::SharingMode sharingMode, const std::vector<uint32_t>& queueFamilyIndices,
		vk::ImageLayout initialLayout, vk::MemoryPropertyFlags memoryPropertyFlags)
	{
		return std::make_shared<Image>(shared_from_this(), createFlags, type, format, extent, mipLevels, 
			arraySize, samples, tiling, usageFlags, sharingMode, queueFamilyIndices, initialLayout, memoryPropertyFlags);
	}
	vk::DeviceMemory Device::allocateMemReqMemory(const vk::MemoryRequirements & reqs, vk::MemoryPropertyFlags flags)
	{
		vk::MemoryAllocateInfo allocateInfo(
			reqs.size,
			findMemoryType(_physicalDevice->getMemoryProperties(), reqs.memoryTypeBits, flags)
		);

		if (allocateInfo.memoryTypeIndex == -1)
			return VK_NULL_HANDLE;

		vk::DeviceMemory memory = _device.allocateMemory(allocateInfo);

		return memory;
	}
	std::shared_ptr<Framebuffer> Device::createFramebuffer(const std::shared_ptr<RenderPass>& renderPass, const std::vector<std::shared_ptr<ImageView>>& attachments, const vk::Extent2D & extent, uint32_t layers)
	{
		return std::make_shared<Framebuffer>(shared_from_this(), renderPass, attachments, extent, layers);
	}
	std::shared_ptr<CommandPool> Device::createCommandPool(vk::CommandPoolCreateFlags flags, uint32_t familyIndex)
	{
		return std::make_shared<CommandPool>(shared_from_this(), flags, familyIndex);
	}
	std::shared_ptr<ShaderModule> Device::createShaderModule(const std::string & filePath)
	{
		return std::make_shared<ShaderModule>(shared_from_this(), filePath);
	}
	std::shared_ptr<ShaderModule> Device::createShaderModule(vk::ArrayProxy<const uint32_t> code)
	{
		return std::make_shared<ShaderModule>(shared_from_this(), code);
	}
	std::shared_ptr<Fence> Device::createFence(bool signaled)
	{
		return std::make_shared<Fence>(shared_from_this(), signaled);
	}
	std::shared_ptr<Sampler> Device::createSampler(const SamplerStateDesc & desc)
	{
		return std::make_shared<Sampler>(shared_from_this(), desc);
	}
	std::shared_ptr<DescriptorSetLayout> Device::createDescriptorSetLayout(
		vk::ArrayProxy<const DescriptorSetLayoutBinding> bindings)
	{
		return std::make_shared<DescriptorSetLayout>(shared_from_this(), bindings);
	}
	std::shared_ptr<DescriptorPool> Device::createDescriptorPool(
		vk::DescriptorPoolCreateFlags flags, uint32_t maxSets, 
		vk::ArrayProxy<const vk::DescriptorPoolSize> poolSizes)
	{
		return std::make_shared<DescriptorPool>(shared_from_this(), flags, maxSets, poolSizes);
	}
}