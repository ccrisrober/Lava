#pragma once

#include "includes.hpp"

#include "VulkanResource.h"
#include "noncopyable.hpp"

namespace lava
{
	class Buffer: public VulkanResource, private NonCopyable<Buffer>
	{
	public:
		Buffer(const std::shared_ptr<Device>& device);
		virtual ~Buffer();

		uint8_t* map(vk::DeviceSize offset, vk::DeviceSize length) const;
		void unmap( void );

		inline operator vk::Buffer() const
		{
			return _buffer;
		}

	protected:
		vk::Buffer _buffer;
		vk::BufferView _view;
		vk::DeviceMemory _memory;
	};
}