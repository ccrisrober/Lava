#include "CommandBuffer.h"

#include "Device.h"

namespace lava
{
	CommandPool::CommandPool(const std::shared_ptr<Device>& device, vk::CommandPoolCreateFlags flags, uint32_t familyIndex)
		: VulkanResource( device )
	{
		vk::CommandPoolCreateInfo cci(flags, familyIndex);
		_commandPool = static_cast<vk::Device>(*_device).createCommandPool(cci);
	}
	CommandPool::~CommandPool()
	{
		static_cast<vk::Device>(*_device).destroyCommandPool(_commandPool);
	}
	std::shared_ptr<CommandBuffer> CommandPool::allocateCommandBuffer(vk::CommandBufferLevel level)
	{
		std::shared_ptr<CommandBuffer> commandBuffer = std::make_shared<CommandBuffer>(shared_from_this(), level);
		_commandBuffers.push_back(commandBuffer.get());
		return(commandBuffer);
	}
	CommandBuffer::CommandBuffer(const std::shared_ptr<CommandPool>& cmdPool, vk::CommandBufferLevel level)
		: _commandPool( cmdPool )
	{
		vk::CommandBufferAllocateInfo info(*_commandPool, level, 1);
		std::vector<vk::CommandBuffer> commandBuffers = static_cast<vk::Device>(*_commandPool->getDevice()).allocateCommandBuffers(info);
		assert(!commandBuffers.empty());
		_commandBuffer = commandBuffers[0];
	}
	CommandBuffer::~CommandBuffer(void)
	{
	}

	void CommandBuffer::clearAttachments(vk::ArrayProxy<const vk::ClearAttachment> attachments, vk::ArrayProxy<const vk::ClearRect> rects)
	{
		_commandBuffer.clearAttachments(attachments, rects);
	}

	void CommandBuffer::beginRenderPass(const std::shared_ptr<RenderPass>& renderPass,
		const std::shared_ptr<Framebuffer>& framebuffer, const vk::Rect2D& area, 
		vk::ArrayProxy<const vk::ClearValue> clearValues, vk::SubpassContents contents)
	{
		_renderPass = renderPass;
		_framebuffer = framebuffer;

		vk::RenderPassBeginInfo renderPassBeginInfo;

		renderPassBeginInfo.renderPass = *renderPass;
		renderPassBeginInfo.framebuffer = *framebuffer;
		renderPassBeginInfo.renderArea = area;
		renderPassBeginInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderPassBeginInfo.pClearValues = reinterpret_cast<vk::ClearValue const*>(clearValues.data());

		_commandBuffer.beginRenderPass(renderPassBeginInfo, contents);
	}
	void CommandBuffer::endRenderPass()
	{
		_renderPass.reset();
		_framebuffer.reset();

		_commandBuffer.endRenderPass();
	}
	void CommandBuffer::setStencilCompareMask(vk::StencilFaceFlags faceMask, uint32_t stencilCompareMask)
	{
		_commandBuffer.setStencilCompareMask(faceMask, stencilCompareMask);
	}
	void CommandBuffer::setStencilReference(vk::StencilFaceFlags faceMask, uint32_t stencilReference)
	{
		_commandBuffer.setStencilReference(faceMask, stencilReference);
	}
	void CommandBuffer::setStencilWriteMask(vk::StencilFaceFlags faceMask, uint32_t stencilWriteMask)
	{
		_commandBuffer.setStencilWriteMask(faceMask, stencilWriteMask);
	}
	void CommandBuffer::setLineWidth(float lineWidth)
	{
		_commandBuffer.setLineWidth(lineWidth);
	}
	void CommandBuffer::bindPipeline(vk::PipelineBindPoint bindingPoint, std::shared_ptr<Pipeline> const & pipeline)
	{
		_commandBuffer.bindPipeline(bindingPoint, *pipeline);
	}
	void CommandBuffer::setScissor(uint32_t first, const std::vector<vk::Rect2D>& scissors)
	{
		_commandBuffer.setScissor(first, scissors);
	}
	void CommandBuffer::setViewport(uint32_t first, const std::vector<vk::Viewport>& viewports)
	{
		_commandBuffer.setViewport(first, viewports);
	}
	void CommandBuffer::dispatch(uint32_t x, uint32_t y, uint32_t z)
	{
		_commandBuffer.dispatch(x, y, z);
	}
	void CommandBuffer::draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, 
		uint32_t firstInstance)
	{
		_commandBuffer.draw(vertexCount, instanceCount, firstVertex, firstInstance);
	}

	void CommandBuffer::drawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex,
		int32_t vertexOffset, uint32_t firstInstance)
	{
		_commandBuffer.drawIndexed(indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
	}

	void CommandBuffer::begin(vk::CommandBufferUsageFlags flags, 
		std::shared_ptr<RenderPass> const & renderPass,  uint32_t subpass, 
		std::shared_ptr<Framebuffer> const & framebuffer, vk::Bool32 occlusionQueryEnable, 
		vk::QueryControlFlags queryFlags, vk::QueryPipelineStatisticFlags pipelineStatistics)
	{
		_renderPass = renderPass;
		_framebuffer = framebuffer;

		vk::CommandBufferInheritanceInfo inheritanceInfo;
		vk::CommandBufferBeginInfo beginInfo(flags, &inheritanceInfo);

		inheritanceInfo.renderPass = renderPass ? *renderPass : vk::RenderPass();
		inheritanceInfo.subpass = subpass;
		inheritanceInfo.framebuffer = framebuffer ? *framebuffer : vk::Framebuffer();
		inheritanceInfo.occlusionQueryEnable = occlusionQueryEnable;
		inheritanceInfo.queryFlags = queryFlags;
		inheritanceInfo.pipelineStatistics = pipelineStatistics;

		_commandBuffer.begin(beginInfo);
		_isRecording = true;
	}

	void CommandBuffer::end()
	{
		_isRecording = false;
		_commandBuffer.end();
	}

}