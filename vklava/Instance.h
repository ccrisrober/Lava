#ifndef __VKLAVA_INSTANCE__
#define __VKLAVA_INSTANCE__

#include "includes.hpp"
#include "PhysicalDevice.h"
#include "Surface.h"
#include <memory>

#include "noncopyable.hpp"

namespace lava
{
	VkBool32 VKAPI_CALL debugMsgCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objectType,
		uint64_t object, size_t location, int32_t messageCode, const char* pLayerPrefix,
		const char* pMessage, void* pUserData);
	class Instance: public std::enable_shared_from_this<Instance>,
		private NonCopyable<Instance>
	{
	public:
		static std::shared_ptr<Instance> create(const vk::InstanceCreateInfo& ci);
		Instance(const vk::InstanceCreateInfo& ci);
		virtual ~Instance(void);

		inline operator vk::Instance(void)
		{
			return _instance;
		}

		void createDebugReportCallback(
			const vk::DebugReportCallbackCreateInfoEXT& debugInfo);

		std::shared_ptr<Surface> createSurfaceKHR(GLFWwindow* window)
		{
			// Surface KHR
			VkSurfaceKHR surface;
			if (glfwCreateWindowSurface(VkInstance(_instance),
				window, nullptr, &surface) != VK_SUCCESS)
			{
				throw std::runtime_error("failed to create window surface!");
			}
			return std::make_shared<Surface>(shared_from_this(), vk::SurfaceKHR(surface));
		}
		uint32_t getPhysicalDeviceCount( void ) const;
		std::shared_ptr<PhysicalDevice> getPhysicalDevice(uint32_t index);
	private:
		std::vector<vk::PhysicalDevice> _physicalDevices;
		std::vector<std::weak_ptr<PhysicalDevice>> _physicalDevicesCache;
		vk::Instance _instance;
#ifndef NDEBUG
		vk::DebugReportCallbackEXT _debugCallback;
#endif
	};

}

#endif /* __VKLAVA_INSTANCE__ */