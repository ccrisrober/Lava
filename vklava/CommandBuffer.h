#pragma once

#include "includes.hpp"
#include "Pipeline.h"

#include "VulkanResource.h"

namespace lava
{
	class Device;
	class RenderPass;
	class Framebuffer;
	class CommandBuffer;
	class CommandPool: public VulkanResource, public std::enable_shared_from_this<CommandPool>
	{
	public:
		CommandPool(const std::shared_ptr<Device>& device,
			vk::CommandPoolCreateFlags flags = vk::CommandPoolCreateFlags(),
			uint32_t familyIndex = 0);
		virtual ~CommandPool();

		inline operator vk::CommandPool() const
		{
			return _commandPool;
		}
		const std::shared_ptr<Device>& getDevice()
		{
			return _device;
		}
		std::shared_ptr<CommandBuffer> allocateCommandBuffer(
			vk::CommandBufferLevel level = vk::CommandBufferLevel::ePrimary);
	protected:
		vk::CommandPool _commandPool;
		std::vector<CommandBuffer*> _commandBuffers;
	};
	class CommandBuffer
	{
	public:
		CommandBuffer(const std::shared_ptr<CommandPool>& cmdPool, 
			vk::CommandBufferLevel level = vk::CommandBufferLevel::ePrimary);
		virtual ~CommandBuffer(void);

		inline operator vk::CommandBuffer() const
		{
			return _commandBuffer;
		}
		
		void clearAttachments(vk::ArrayProxy< const vk::ClearAttachment> attachments, vk::ArrayProxy<const vk::ClearRect> rects);

		void beginRenderPass(const std::shared_ptr<RenderPass>& renderPass, 
			const std::shared_ptr<Framebuffer>& framebuffer, const vk::Rect2D& area, 
			vk::ArrayProxy<const vk::ClearValue> clearValues, vk::SubpassContents contents);
		void endRenderPass();

		void setStencilCompareMask(vk::StencilFaceFlags faceMask, uint32_t stencilCompareMask);
		void setStencilReference(vk::StencilFaceFlags faceMask, uint32_t stencilReference);
		void setStencilWriteMask(vk::StencilFaceFlags faceMask, uint32_t stencilWriteMask);

		void begin(vk::CommandBufferUsageFlags               flags = vk::CommandBufferUsageFlags(),
			std::shared_ptr<RenderPass> const&   renderPass = std::shared_ptr<RenderPass>(),
			uint32_t                                  subpass = 0,
			std::shared_ptr<Framebuffer> const&  framebuffer = std::shared_ptr<Framebuffer>(),
			vk::Bool32                                occlusionQueryEnable = false,
			vk::QueryControlFlags                     queryFlags = vk::QueryControlFlags(),
			vk::QueryPipelineStatisticFlags           pipelineStatistics = vk::QueryPipelineStatisticFlags());
		void end();

		inline bool isRecording(void) const
		{
			return _isRecording;
		}

		void bindPipeline(vk::PipelineBindPoint bindingPoint, std::shared_ptr<Pipeline> const& pipeline);

		void setScissor(uint32_t first, const std::vector<vk::Rect2D>& scissors);
		void setViewport(uint32_t first, const std::vector<vk::Viewport>& viewports);
		void setLineWidth(float lineWidth);
		void dispatch(uint32_t x, uint32_t y, uint32_t z);
		void draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, 
			uint32_t firstInstance);
		void drawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex,
			int32_t vertexOffset, uint32_t firstInstance);
	protected:
		std::shared_ptr<CommandPool> _commandPool;
		vk::CommandBuffer _commandBuffer;
		std::shared_ptr<RenderPass> _renderPass;
		std::shared_ptr<Framebuffer> _framebuffer;
		bool _isRecording;
	};
}