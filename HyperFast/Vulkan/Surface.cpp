#include "Surface.h"
#include <exception>

namespace Vulkan
{
	Surface::Surface(Instance &instance, const VkWin32SurfaceCreateInfoKHR &createInfo) :
		Handle{ __create(instance, createInfo) }, __instance{ instance }
	{}

	Surface::~Surface() noexcept
	{
		__destroy();
	}

	void Surface::__destroy() noexcept
	{
		__instance.vkDestroySurfaceKHR(getHandle(), nullptr);
	}

	VkSurfaceKHR Surface::__create(Instance &instance, const VkWin32SurfaceCreateInfoKHR &createInfo)
	{
		VkSurfaceKHR retVal{};
		instance.vkCreateWin32SurfaceKHR(&createInfo, nullptr, &retVal);

		if (!retVal)
			throw std::exception{ "Cannot create a VkSurfaceKHR." };

		return retVal;
	}
}