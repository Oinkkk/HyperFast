#include "Fence.h"
#include <exception>

namespace Vulkan
{
	Fence::Fence(Device &device, const VkFenceCreateInfo &createInfo) :
		Handle{ __create(device, createInfo) }, __device{ device }
	{}

	Fence::~Fence() noexcept
	{
		__destroy();
	}

	VkResult Fence::wait(const uint64_t timeout) noexcept
	{
		return __device.vkWaitForFences(1U, &(getHandle()), VK_TRUE, timeout);
	}

	VkResult Fence::reset() noexcept
	{
		return __device.vkResetFences(1U, &(getHandle()));
	}

	void Fence::__destroy() noexcept
	{
		__device.vkDestroyFence(getHandle(), nullptr);
	}

	VkFence Fence::__create(Device &device, const VkFenceCreateInfo &createInfo)
	{
		VkFence retVal{};
		device.vkCreateFence(&createInfo, nullptr, &retVal);

		if (!retVal)
			throw std::exception{ "Cannot create a VkFence." };

		return retVal;
	}
}