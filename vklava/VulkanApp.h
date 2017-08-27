#ifndef __VKLAVA_VULKANAPP__
#define __VKLAVA_VULKANAPP__

#include "includes.hpp"
#include "Instance.h"
#include "RenderAPICapabilites.h"
#include "FramebufferSwapchain.h"

namespace lava
{
	class VulkanApp
	{
	public:
		VulkanApp(const char* title, uint32_t width, uint32_t height);
		virtual ~VulkanApp(void);

		RenderAPICapabilities caps;

		void paint();
		GLFWwindow* getWindow() const { return _window; }
	protected:
		virtual void resize(uint32_t w, uint32_t h);

		virtual void doResize(uint32_t width, uint32_t height);
		virtual void doPaint();

		virtual void cursorPosEvent(double xPos, double yPos);
		virtual void keyEvent(int key, int scancode, int action, int mods);
		virtual void mouseButtonEvent(int button, int action, int mods);
		
	private:
		static void paintCallback(GLFWwindow *window);
		static void resizeCallback(GLFWwindow *window, int width, int height);
		static void cursorPosCallback(GLFWwindow * window, double xPos, double yPos);
		static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
		static void mouseButtonCallback(GLFWwindow * window, int button, int action, int mods);

		
	protected:
		GLFWwindow* _window;
		std::shared_ptr<Instance> instance;

		std::shared_ptr<PhysicalDevice> _physicalDevice;
		std::shared_ptr<Device> _device;
		std::unique_ptr<FramebufferSwapchain> _framebufferSwapchain;
		std::shared_ptr<Queue> _graphicsQueue;
		std::shared_ptr<RenderPass> _renderPass;
		std::shared_ptr<Surface> _surface;

		uint32_t _queueFamilyIndex;

		vk::Format _colorFormat;
		vk::Format _depthFormat;
		std::shared_ptr<Semaphore> _renderComplete;

		void initCapabilities(void);
		bool checkValidationLayerSupport(
			const std::vector<const char*>& validationLayers)
		{
			uint32_t layerCount;
			vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

			std::vector<vk::LayerProperties> availableLayers = vk::enumerateInstanceLayerProperties();

			for (const char* layerName : validationLayers)
			{
				bool layerFound = false;

				for (const auto& layerProperties : availableLayers)
				{
					if (strcmp(layerName, layerProperties.layerName) == 0)
					{
						layerFound = true;
						break;
					}
				}

				if (!layerFound)
				{
					return false;
				}
			}

			return true;
		}
	};
}

#endif /* __VKLAVA_VULKANAPP__ */