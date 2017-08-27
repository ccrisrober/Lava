#pragma once

#include "includes.hpp"
#include "VulkanResource.h"

#include "noncopyable.hpp"

namespace lava
{
	class Device;

	class ShaderModule: public VulkanResource, private NonCopyable<ShaderModule>
	{
	public:
		ShaderModule(const std::shared_ptr<Device>& device, const std::string& filePath);
		ShaderModule(const std::shared_ptr<Device>& device, vk::ArrayProxy<const uint32_t> code);
		~ShaderModule();

		inline operator vk::ShaderModule() const { return _shaderModule; }

	private:
		vk::ShaderModule _shaderModule;

		const std::vector<uint32_t> readFile(const std::string& filename);
	};

	struct PipelineVertexInputStateCreateInfo
	{
		PipelineVertexInputStateCreateInfo(
			vk::ArrayProxy<const vk::VertexInputBindingDescription> vertexBindingDescriptions,
			vk::ArrayProxy<const vk::VertexInputAttributeDescription> vertexAttributeDesriptions);
		PipelineVertexInputStateCreateInfo(const PipelineVertexInputStateCreateInfo& rhs);
		PipelineVertexInputStateCreateInfo & operator=(const PipelineVertexInputStateCreateInfo& rhs);

		std::vector<vk::VertexInputBindingDescription> vertexBindingDescriptions;
		std::vector<vk::VertexInputAttributeDescription> vertexAttributeDesriptions;
	};

	struct PipelineDynamicStateCreateInfo
	{
		PipelineDynamicStateCreateInfo(vk::ArrayProxy<const vk::DynamicState> dynamicStates);
		PipelineDynamicStateCreateInfo(const PipelineDynamicStateCreateInfo& rhs);
		PipelineDynamicStateCreateInfo& operator=(const PipelineDynamicStateCreateInfo& rhs);

		std::vector<vk::DynamicState> dynamicStates;
	};

	struct PipelineViewportStateCreateInfo
	{
		PipelineViewportStateCreateInfo(
			vk::ArrayProxy<const vk::Viewport> viewports_, 
			vk::ArrayProxy<const vk::Rect2D> scissors_);
		PipelineViewportStateCreateInfo(const PipelineViewportStateCreateInfo& rhs);
		PipelineViewportStateCreateInfo & operator=(const PipelineViewportStateCreateInfo& rhs);

		std::vector<vk::Viewport> viewports;
		std::vector<vk::Rect2D> scissors;
	};

	class PipelineCache: private NonCopyable<PipelineCache>
	{
	public:
		PipelineCache(const std::shared_ptr<Device>& device, vk::PipelineCacheCreateFlags flags, 
			size_t initialSize, void const* initialData);
		~PipelineCache();

		std::vector<uint8_t> getData() const;
		void merge(vk::ArrayProxy<const std::shared_ptr<PipelineCache>> srcCaches) const;

		inline operator vk::PipelineCache() const
		{
			return _pipelineCache;
		}
	private:
		std::shared_ptr<Device> _device;
		vk::PipelineCache _pipelineCache;
	};

	class Pipeline: public VulkanResource, private NonCopyable<Pipeline>
	{
	public:
		~Pipeline();

		inline operator vk::Pipeline() const
		{
			return _pipeline;
		}

	protected:
		Pipeline(const std::shared_ptr<Device>& device);
		void setPipeline(const vk::Pipeline & pipeline);

	protected:
		vk::Pipeline _pipeline;
	};

	class ComputePipeline : public Pipeline
	{
	public:
		ComputePipeline(const std::shared_ptr<Device>& device, 
			const std::shared_ptr<PipelineCache>& pipelineCache);
	};

	class GraphicsPipeline : public Pipeline
	{
	public:
		GraphicsPipeline(const std::shared_ptr<Device>& device,
			const std::shared_ptr<PipelineCache>& pipelineCache, 
			vk::Optional<const PipelineVertexInputStateCreateInfo> vertexInputState,
			vk::Optional<const vk::PipelineInputAssemblyStateCreateInfo> inputAssemblyState,
			vk::Optional<const PipelineViewportStateCreateInfo> viewportState,
			vk::Optional<const PipelineDynamicStateCreateInfo> dynamicState,
			vk::Optional<const vk::PipelineRasterizationStateCreateInfo> rasterizationState,
			vk::Optional<const vk::PipelineMultisampleStateCreateInfo> multisampleState, 
			vk::Optional<const vk::PipelineDepthStencilStateCreateInfo> depthStencilState);
	};
}