#pragma once

#include <pompeii/api.h>
#include <vulkan/vulkan.hpp>

#include <memory>
#include <map>

namespace pompeii
{
	class Instance;
	class PhysicalDevice;

	/* In some environments applications can also present Vulkan rendering
	directly to display devices without using an intermediate windowing system.
	This can be useful for embedded applications, or implementing
	the rendering/presentation backend of a windowing system using Vulkan. */
	class Display
	{
		Display(std::shared_ptr</*const*/ PhysicalDevice> physicalDevice,
			vk::DisplayKHR handle,
			uint32_t planeIndex) noexcept;
		friend PhysicalDevice;

	public:
		inline operator vk::DisplayKHR(void) const
		{
			return handle;
		}
		std::shared_ptr<const PhysicalDevice> getPhysicalDevice(void)
			const noexcept { return physicalDevice; }
		uint32_t getPlaneIndex(void) const noexcept { return planeIndex; }
		std::vector<vk::DisplayModePropertiesKHR> getModeProperties( void ) const;

	private:
		std::shared_ptr<Instance> instance;
		std::shared_ptr<PhysicalDevice> physicalDevice;
		uint32_t planeIndex;
		vk::DisplayKHR handle;
	};

	/* Each display has one or more supported modes associated with it by default. */
	class DisplayMode
	{
	public:
		explicit DisplayMode(std::shared_ptr<const Display> display,
			const vk::Extent2D& visibleRegion,
			uint32_t refreshRate);
		const vk::DisplayPlaneCapabilitiesKHR& getPlaneCapabilities(uint32_t planeIndex) const;
		const vk::Extent2D& getVisibleRegion(void) const noexcept { return visibleRegion; }
		uint32_t getRefreshRate(void) const noexcept { return refreshRate; }

	private:
		std::shared_ptr<Instance> instance;
		std::shared_ptr<PhysicalDevice> physicalDevice;
		vk::Extent2D visibleRegion;
		uint32_t refreshRate;
		mutable std::map<uint32_t, vk::DisplayPlaneCapabilitiesKHR> capabilities;
		vk::DisplayModeKHR handle;
	};
}