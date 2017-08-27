#pragma once

#include "includes.hpp"
#include <vector>
#include <map>

#include "VulkanResource.h"

#include "noncopyable.hpp"

namespace lava
{
	class Device;
	class Semaphore;
	class CommandBuffer;
	class Swapchain;

	struct SubmitInfo
	{
		SubmitInfo(vk::ArrayProxy<const std::shared_ptr<Semaphore>> const& waitSemaphores, 
			vk::ArrayProxy<const vk::PipelineStageFlags> waitDstStageMasks,
			vk::ArrayProxy<const std::shared_ptr<CommandBuffer>> const& commandBuffers, 
			vk::ArrayProxy<const std::shared_ptr<Semaphore>> signalSemaphores);
		SubmitInfo(SubmitInfo const& rhs);
		SubmitInfo & operator=(SubmitInfo const& rhs);

		std::vector<std::shared_ptr<Semaphore>> waitSemaphores;
		std::vector<vk::PipelineStageFlags> waitDstStageMasks;
		std::vector<std::shared_ptr<CommandBuffer>> commandBuffers;
		std::vector<std::shared_ptr<Semaphore>> signalSemaphores;
	};
	
	class Fence: public VulkanResource, private NonCopyable<Fence>
	{
	public:
		Fence(std::shared_ptr<Device> const& device, bool signaled);
		virtual ~Fence();

		bool isSignaled() const;
		void reset();
		void wait(uint64_t timeout) const;

		operator vk::Fence() const;
	private:
		vk::Fence _fence;
	};

	/*
	void waitForFences(vk::ArrayProxy<const std::shared_ptr<Fence>> fences, bool all, uint32_t timeout);
	void resetFences(vk::ArrayProxy<const std::shared_ptr<Fence>> fences);
	*/

	inline Fence::operator vk::Fence() const
	{
		return _fence;
	}



	class Queue: public VulkanResource, private NonCopyable<Queue>
	{
	public:
		void submit(vk::ArrayProxy<const SubmitInfo> submitInfos, 
			std::shared_ptr<Fence> const& fence = std::shared_ptr<Fence>());

		void submit(std::shared_ptr<CommandBuffer> const& commandBuffer, 
			std::shared_ptr<Fence> const& fence = std::shared_ptr<Fence>());

		std::vector<vk::Result> present(
			vk::ArrayProxy<const std::shared_ptr<Semaphore>> waitSemaphores, 
			vk::ArrayProxy<const std::shared_ptr<Swapchain>> swapchains, 
			vk::ArrayProxy<const uint32_t> imageIndices);

		void waitIdle();

		inline operator vk::Queue() const
		{
			return _queue;
		}
	protected:
		friend class Device;
		Queue(const std::shared_ptr<Device>& device, vk::Queue queue);

		std::map<std::shared_ptr<Fence>, std::vector<SubmitInfo>> _submitInfos;
		vk::Queue _queue;
	};
}