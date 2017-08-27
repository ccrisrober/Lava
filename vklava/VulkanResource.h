#pragma once

#include "includes.hpp"
#include <memory>

namespace lava
{
	class Device;
	class VulkanResource
	{
	protected:
		VulkanResource( std::shared_ptr<Device> device );

		std::shared_ptr<Device> _device;
	};
}