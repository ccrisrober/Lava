#include "VulkanResource.h"

#include "Device.h"

namespace lava
{
	VulkanResource::VulkanResource( std::shared_ptr<Device> device )
		: _device( device )
	{
	}
}