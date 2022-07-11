#include "DebugUtilsMessenger.h"
#include <exception>

namespace Vulkan
{
	DebugUtilsMessenger::DebugUtilsMessenger(
		Instance &instance, const VkDebugUtilsMessengerCreateInfoEXT &createInfo) :
		Handle{ __create(instance, createInfo) }, __instance{ instance }
	{}

	DebugUtilsMessenger::~DebugUtilsMessenger() noexcept
	{
		__destroy();
	}

	void DebugUtilsMessenger::__destroy() noexcept
	{
		__instance.vkDestroyDebugUtilsMessengerEXT(getHandle(), nullptr);
	}

	VkDebugUtilsMessengerEXT DebugUtilsMessenger::__create(
		Instance &instance, const VkDebugUtilsMessengerCreateInfoEXT &createInfo)
	{
		VkDebugUtilsMessengerEXT retVal{};
		instance.vkCreateDebugUtilsMessengerEXT(&createInfo, nullptr, &retVal);

		if (!retVal)
			throw std::exception{ "Cannot create a VkDebugUtilsMessengerEXT." };

		return retVal;
	}
}