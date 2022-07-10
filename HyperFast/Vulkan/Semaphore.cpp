#include "Semaphore.h"
#include <exception>

namespace Vulkan
{
	Semaphore::Semaphore(Device &device, const VkSemaphoreCreateInfo &createInfo) :
		Handle{ __create(device, createInfo) }, __device{ device }
	{}

	Semaphore::~Semaphore() noexcept
	{
		__destroy();
	}

	void Semaphore::__destroy() noexcept
	{
		__device.vkDestroySemaphore(getHandle(), nullptr);
	}

	VkSemaphore Semaphore::__create(Device &device, const VkSemaphoreCreateInfo &createInfo)
	{
		VkSemaphore retVal{};
		device.vkCreateSemaphore(&createInfo, nullptr, &retVal);

		if (!retVal)
			throw std::exception{ "Cannot create a VkSemaphore." };

		return retVal;
	}
}