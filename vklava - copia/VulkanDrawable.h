#pragma once

#include <vulkan/vulkan.h>
#include <vector>

namespace lava
{
	class VulkanDrawable
	{
	public:
		// Prepares the drawing object before rendering,
		// allocate, create, record command buffer
		void prepare();
		// Renders the drawing object
		void render();
		// Initialize the viewport parameters here
		void initViewports(VkCommandBuffer* cmd);
		// Initialize the scissor parameters here
		void initScissors(VkCommandBuffer* cmd);
		// Destroy the drawing command buffer object
		void destroyCommandBuffer();
	private:
		// Command buffer for drawing
		std::vector<VkCommandBuffer> vecCmdDraw;
		// Prepares render pass instance
		void recordCommandBuffer(int currentImage,
		VkCommandBuffer* cmdDraw);
		// Viewport and Scissor variables
		VkViewport viewport;
		VkRect2D scissor;
		VkSemaphore presentCompleteSemaphore;
		VkSemaphore drawingCompleteSemaphore;
	};
}