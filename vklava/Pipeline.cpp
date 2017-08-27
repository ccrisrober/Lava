#include "Pipeline.h"

#include "Device.h"
#include "VulkanResource.h"

#include <fstream>

namespace lava
{
	ShaderModule::ShaderModule(const std::shared_ptr<Device>& device, 
		const std::string & filePath)
		: ShaderModule(device, readFile(filePath))
	{
	}

	const std::vector<uint32_t> ShaderModule::readFile(const std::string & filename)
	{
		std::ifstream file(filename, std::ios::ate | std::ios::binary);

		if (!file.is_open())
		{
			throw std::runtime_error("failed to open file!");
		}

		size_t fileSize = (size_t)file.tellg();
		std::vector<char> buffer(fileSize);

		file.seekg(0);
		file.read(buffer.data(), fileSize);

		file.close();

		const uint32_t* arr = reinterpret_cast<const uint32_t*>(buffer.data());

		return std::vector<uint32_t>(arr, arr + buffer.size());
	}

	ShaderModule::ShaderModule(const std::shared_ptr<Device>& device, 
		vk::ArrayProxy<const uint32_t> code)
		: VulkanResource(device)
	{
		vk::ShaderModuleCreateInfo sci(
			vk::ShaderModuleCreateFlags(), 
			4 * code.size(), 
			code.data()
		);
		_shaderModule = static_cast<vk::Device>(*_device).createShaderModule(sci);
	}

	ShaderModule::~ShaderModule()
	{
		static_cast<vk::Device>(*_device).destroyShaderModule(_shaderModule);
	}

	PipelineVertexInputStateCreateInfo::PipelineVertexInputStateCreateInfo(
		vk::ArrayProxy<const vk::VertexInputBindingDescription> vertexBindingDescriptions_,
		vk::ArrayProxy<const vk::VertexInputAttributeDescription> vertexAttributeDesriptions_)
		: vertexBindingDescriptions(vertexBindingDescriptions_.begin(), vertexBindingDescriptions_.end())
		, vertexAttributeDesriptions(vertexAttributeDesriptions_.begin(), vertexAttributeDesriptions_.end())
	{}

	PipelineVertexInputStateCreateInfo::PipelineVertexInputStateCreateInfo(
		const PipelineVertexInputStateCreateInfo& rhs)
		: PipelineVertexInputStateCreateInfo(rhs.vertexBindingDescriptions, rhs.vertexAttributeDesriptions)
	{}

	PipelineVertexInputStateCreateInfo & PipelineVertexInputStateCreateInfo::operator=(
		PipelineVertexInputStateCreateInfo const& rhs)
	{
		vertexBindingDescriptions = rhs.vertexBindingDescriptions;
		vertexAttributeDesriptions = rhs.vertexAttributeDesriptions;
		return *this;
	}

	PipelineDynamicStateCreateInfo::PipelineDynamicStateCreateInfo(
		vk::ArrayProxy<const vk::DynamicState> dynamicStates_)
		: dynamicStates(dynamicStates_.begin(), dynamicStates_.end())
	{}

	PipelineDynamicStateCreateInfo::PipelineDynamicStateCreateInfo(
		const PipelineDynamicStateCreateInfo& rhs)
		: PipelineDynamicStateCreateInfo(rhs.dynamicStates)
	{}

	PipelineDynamicStateCreateInfo & PipelineDynamicStateCreateInfo::operator=(
		const PipelineDynamicStateCreateInfo& rhs)
	{
		dynamicStates = rhs.dynamicStates;
		return *this;
	}



	PipelineViewportStateCreateInfo::PipelineViewportStateCreateInfo(
		vk::ArrayProxy<const vk::Viewport> viewports_, vk::ArrayProxy<const vk::Rect2D> scissors_)
		: viewports(viewports_.begin(), viewports_.end())
		, scissors(scissors_.begin(), scissors_.end())
	{}

	PipelineViewportStateCreateInfo::PipelineViewportStateCreateInfo(
		const PipelineViewportStateCreateInfo& rhs)
		: PipelineViewportStateCreateInfo(rhs.viewports, rhs.scissors)
	{}

	PipelineViewportStateCreateInfo & PipelineViewportStateCreateInfo::operator=(
		const PipelineViewportStateCreateInfo& rhs)
	{
		viewports = rhs.viewports;
		scissors = rhs.scissors;
		return *this;
	}

	PipelineCache::PipelineCache(const std::shared_ptr<Device>& device, vk::PipelineCacheCreateFlags flags, 
		size_t initialSize, void const* initialData)
		: _device(device)
	{
		vk::PipelineCacheCreateInfo createInfo{ flags, initialSize, initialData };
		_pipelineCache = static_cast<vk::Device>(*_device).createPipelineCache(createInfo);
	}

	PipelineCache::~PipelineCache()
	{
		static_cast<vk::Device>(*_device).destroyPipelineCache(_pipelineCache);
	}

	std::vector<uint8_t>  PipelineCache::getData() const
	{
		return static_cast<vk::Device>(*_device).getPipelineCacheData(_pipelineCache);
	}

	void PipelineCache::merge(vk::ArrayProxy<const std::shared_ptr<PipelineCache>> srcCaches) const
	{
		std::vector<vk::PipelineCache> caches;
		caches.reserve(srcCaches.size());
		for (auto const& c : srcCaches)
		{
			caches.push_back(*c);
		}
		static_cast<vk::Device>(*_device).mergePipelineCaches(_pipelineCache, caches);
	}
	Pipeline::Pipeline(std::shared_ptr<Device> const& device)
		: VulkanResource(device)
	{}

	void Pipeline::setPipeline(vk::Pipeline const& pipeline)
	{
		_pipeline = pipeline;
	}

	Pipeline::~Pipeline()
	{
		static_cast<vk::Device>(*_device).destroyPipeline(_pipeline);
	}


	ComputePipeline::ComputePipeline(const std::shared_ptr<Device>& device,
		const std::shared_ptr<PipelineCache>& pipelineCache)
		: Pipeline( device )
	{
		vk::ComputePipelineCreateInfo cci;

		setPipeline(vk::Device(*_device).createComputePipeline(
			pipelineCache ? static_cast<vk::PipelineCache>(*pipelineCache) : nullptr, 
			cci));
	}

	GraphicsPipeline::GraphicsPipeline(const std::shared_ptr<Device>& device,
		const std::shared_ptr<PipelineCache>& pipelineCache,
		vk::Optional<const PipelineVertexInputStateCreateInfo> vertexInputState,
		vk::Optional<const vk::PipelineInputAssemblyStateCreateInfo> inputAssemblyState,
		vk::Optional<const PipelineViewportStateCreateInfo> viewportState,
		vk::Optional<const PipelineDynamicStateCreateInfo> dynamicState,
		vk::Optional<const vk::PipelineRasterizationStateCreateInfo> rasterizationState,
		vk::Optional<const vk::PipelineMultisampleStateCreateInfo> multisampleState,
		vk::Optional<const vk::PipelineDepthStencilStateCreateInfo> depthStencilState)
		: Pipeline(device)
	{
		vk::PipelineVertexInputStateCreateInfo vkVertexInputState;
		if (vertexInputState)
		{
			vkVertexInputState = vk::PipelineVertexInputStateCreateInfo(
				{}, 
				vertexInputState->vertexBindingDescriptions.size(), 
				vertexInputState->vertexBindingDescriptions.data(),
				vertexInputState->vertexAttributeDesriptions.size(),
				vertexInputState->vertexAttributeDesriptions.data()
			);
		}

		vk::PipelineViewportStateCreateInfo vkViewportState;
		if (viewportState)
		{
			vkViewportState = vk::PipelineViewportStateCreateInfo(
				{}, 
				viewportState->viewports.size(),
				viewportState->viewports.data(),
				viewportState->scissors.size(),
				viewportState->scissors.data()
			);
		}

		vk::PipelineDynamicStateCreateInfo vkDynamicState;
		if (dynamicState)
		{
			vkDynamicState = vk::PipelineDynamicStateCreateInfo(
				{}, 
				dynamicState->dynamicStates.size(),
				dynamicState->dynamicStates.data()
			);
		}

		vk::GraphicsPipelineCreateInfo gci;
		gci.setPVertexInputState(vertexInputState ? &vkVertexInputState : nullptr);
		gci.setPTessellationState(nullptr);
		gci.setPInputAssemblyState(inputAssemblyState);
		gci.setPDynamicState(dynamicState ? &vkDynamicState : nullptr);
		gci.setPViewportState(viewportState ? &vkViewportState : nullptr);
		gci.setPMultisampleState(multisampleState);
		gci.setPDepthStencilState(depthStencilState);

		setPipeline(vk::Device(*_device).createGraphicsPipeline(
			pipelineCache ? static_cast<vk::PipelineCache>(*pipelineCache) : nullptr,
			gci));
	}
}