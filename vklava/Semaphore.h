#pragma once

#include "includes.hpp"

#include "VulkanResource.h"

namespace lava
{
	class Device;
	class Semaphore: public VulkanResource
	{
	public:
		Semaphore(const std::shared_ptr<Device>& device);
		virtual ~Semaphore(void);
	
		inline operator vk::Semaphore()
		{
			return _semaphore;
		}

		Semaphore(Semaphore const& rhs) = delete;
		Semaphore & operator=(Semaphore const& rhs) = delete;

	protected:
		vk::Semaphore _semaphore;
	};
}