#include "Instance.h"

#include <iostream>

PFN_vkCreateDebugReportCallbackEXT  pfnVkCreateDebugReportCallbackEXT;
PFN_vkDestroyDebugReportCallbackEXT pfnVkDestroyDebugReportCallbackEXT;

VKAPI_ATTR VkResult VKAPI_CALL vkCreateDebugReportCallbackEXT(VkInstance instance, const VkDebugReportCallbackCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugReportCallbackEXT* pCallback)
{
	return pfnVkCreateDebugReportCallbackEXT(instance, pCreateInfo, pAllocator, pCallback);
}

VKAPI_ATTR void VKAPI_CALL vkDestroyDebugReportCallbackEXT(VkInstance instance, VkDebugReportCallbackEXT callback, const VkAllocationCallbacks* pAllocator)
{
	pfnVkDestroyDebugReportCallbackEXT(instance, callback, pAllocator);
}

#include <sstream>

namespace lava
{
	VKAPI_ATTR VkBool32 VKAPI_CALL debugMsgCallback(VkDebugReportFlagsEXT flags,
		VkDebugReportObjectTypeEXT objType, uint64_t srcObject,
		size_t location, int32_t msgCode, const char* pLayerPrefix,
		const char* pMsg, void* pUserData)
	{
		std::stringstream message;

		// Determine prefix
		if (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT)
			message << "ERROR";

		if (flags & VK_DEBUG_REPORT_WARNING_BIT_EXT)
			message << "WARNING";

		if (flags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT)
			message << "PERFORMANCE";

		if (flags & VK_DEBUG_REPORT_INFORMATION_BIT_EXT)
			message << "INFO";

		if (flags & VK_DEBUG_REPORT_DEBUG_BIT_EXT)
			message << "DEBUG";

		message << ": [" << pLayerPrefix << "] Code " << msgCode << ": "
			<< pMsg << std::endl;

		if (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT)
			std::cerr << message.str() << std::endl;
		else if (flags & VK_DEBUG_REPORT_WARNING_BIT_EXT || flags &
			VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT)
			std::cerr << message.str() << std::endl;
		else
			std::cerr << message.str() << std::endl;
		assert(!message);

		// Abort calls that caused a validation message
		return VK_TRUE;
	}

	std::shared_ptr<Instance> Instance::create(const vk::InstanceCreateInfo& ci)
	{
		return std::make_shared<Instance>(ci);
	}
	Instance::Instance(const vk::InstanceCreateInfo& ci)
	{
		_instance = vk::createInstance(ci);
		_physicalDevices = _instance.enumeratePhysicalDevices();
		_physicalDevicesCache.resize(_physicalDevices.size());
		
		static bool initialized = false;
		if (!initialized)
		{
			pfnVkCreateDebugReportCallbackEXT = reinterpret_cast<PFN_vkCreateDebugReportCallbackEXT>(
				_instance.getProcAddr("vkCreateDebugReportCallbackEXT"));
			pfnVkDestroyDebugReportCallbackEXT = reinterpret_cast<PFN_vkDestroyDebugReportCallbackEXT>(
				_instance.getProcAddr("vkDestroyDebugReportCallbackEXT"));
			initialized = true;
		}
	}
	Instance::~Instance(void)
	{
#ifndef NDEBUG
		_instance.destroyDebugReportCallbackEXT(_debugCallback);
#endif
		_physicalDevices.clear();
		_instance.destroy();
	}
	void Instance::createDebugReportCallback(
		const vk::DebugReportCallbackCreateInfoEXT& debugInfo)
	{
#ifndef NDEBUG
		_debugCallback = _instance.createDebugReportCallbackEXT(debugInfo);
#endif
	}


	uint32_t Instance::getPhysicalDeviceCount() const
	{
		return(_physicalDevices.size());
	}

	std::shared_ptr<PhysicalDevice> Instance::getPhysicalDevice(uint32_t index)
	{
		assert(index < _physicalDevices.size());

		std::shared_ptr<PhysicalDevice> physicalDevice = _physicalDevicesCache[index].lock();
		if (!physicalDevice)
		{
			physicalDevice = std::make_shared<PhysicalDevice>(shared_from_this(), _physicalDevices[index]);
			_physicalDevicesCache[index] = physicalDevice;
		}
		return physicalDevice;
	}
}