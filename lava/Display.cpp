#include "Display.h"

#include "PhysicalDevice.h"
#include "Instance.h"

namespace lava
{
	Display::Display(std::shared_ptr</*const*/ PhysicalDevice> physicalDevice_, 
		vk::DisplayKHR handle_, uint32_t planeIndex_) noexcept
		: instance(/*std::move*/(physicalDevice->getInstance()))
		//, physicalDevice(/*std::move*/(physicalDevice))
		, planeIndex(planeIndex_)
	{
		this->physicalDevice = physicalDevice_;
		this->handle = handle_;
	}

	std::vector<vk::DisplayModePropertiesKHR> Display::getModeProperties( void ) const
	{
		vk::PhysicalDevice pd = static_cast<vk::PhysicalDevice>(*physicalDevice);
		std::vector<vk::DisplayModePropertiesKHR> displayModeProperties = pd.getDisplayModePropertiesKHR(handle);
		if (displayModeProperties.empty())
		{
			throw;
		}
		return displayModeProperties;
	}

	DisplayMode::DisplayMode(std::shared_ptr<const Display> display_,
		const vk::Extent2D & visibleRegion_, uint32_t refreshRate_)
		: instance(/*std::move*/(display_->getPhysicalDevice()->getInstance()))
		//, physicalDevice(std::move(display->getPhysicalDevice())),
		, visibleRegion(visibleRegion_)
		, refreshRate(refreshRate_)
	{
		this->physicalDevice = physicalDevice;
		vk::DisplayModeCreateInfoKHR info;
		info.parameters.visibleRegion = visibleRegion;
		info.parameters.refreshRate = refreshRate;

		vk::PhysicalDevice pd = static_cast<vk::PhysicalDevice>(*physicalDevice);
		vk::DisplayKHR display = static_cast<vk::DisplayKHR>(*display_);
		this->handle = pd.createDisplayModeKHR(display, info);
	}

	const vk::DisplayPlaneCapabilitiesKHR & DisplayMode::getPlaneCapabilities(uint32_t planeIndex) const
	{
		const auto it = capabilities.find(planeIndex);
		if (it == capabilities.end())
		{
			vk::PhysicalDevice pd = static_cast<vk::PhysicalDevice>(*physicalDevice);
			vk::DisplayPlaneCapabilitiesKHR planeCaps = pd.getDisplayPlaneCapabilitiesKHR(handle, planeIndex);
			capabilities[planeIndex] = planeCaps;
		}
		return capabilities[planeIndex];
	}
}