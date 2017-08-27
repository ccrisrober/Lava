#include "Buffer.h"

#include "Device.h"

namespace lava
{
	Buffer::Buffer(const std::shared_ptr<Device>& device)
		: VulkanResource( device )
	{
		vk::BufferCreateInfo bci;
		_buffer = static_cast<vk::Device>(*_device).createBuffer(bci);
		vk::MemoryRequirements memReqs = static_cast<vk::Device>(*_device).getBufferMemoryRequirements(_buffer);

		//static_cast<vk::Device>(*_device).bindBufferMemory(_buffer, )
	}

	Buffer::~Buffer()
	{
		static_cast<vk::Device>(*_device).destroyBuffer(_buffer);
	}
	uint8_t * Buffer::map(vk::DeviceSize offset, vk::DeviceSize length) const
	{
		uint8_t* data;
		vk::Result result = static_cast<vk::Device>(*_device).mapMemory(_memory, offset, length, {}, (void**)&data);
		assert(result == vk::Result::eSuccess);
		return data;
	}
	void Buffer::unmap(void)
	{
		static_cast<vk::Device>(*_device).unmapMemory(_memory);
	}
}