#pragma once

#include "includes.hpp"
#include "noncopyable.hpp"

namespace lava
{
	class Instance;
	class Surface: private NonCopyable<Surface>
	{
	public:
		Surface(const std::shared_ptr<Instance>& instance, const vk::SurfaceKHR& surface);
		~Surface();

		operator vk::SurfaceKHR() const;
	private:
		std::shared_ptr<Instance> _instance;
		vk::SurfaceKHR _surface;
	};

	inline Surface::operator vk::SurfaceKHR() const
	{
		return _surface;
	}
}