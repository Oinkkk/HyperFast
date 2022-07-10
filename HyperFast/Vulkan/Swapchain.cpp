#include "Swapchain.h"
#include <exception>

namespace Vulkan
{
	Swapchain::Swapchain(Device &device, const VkSwapchainCreateInfoKHR &createInfo) :
		Handle{ __create(device, createInfo) }, __device{ device }
	{}

	Swapchain::~Swapchain() noexcept
	{
		__destroy();
	}

	VkResult Swapchain::vkGetSwapchainImagesKHR(
		uint32_t *const pSwapchainImageCount, VkImage *const pSwapchainImages) noexcept
	{
		return __device.vkGetSwapchainImagesKHR(
			getHandle(), pSwapchainImageCount, pSwapchainImages);
	}

	VkResult Swapchain::vkAcquireNextImageKHR(
		const uint64_t timeout, const VkSemaphore semaphore,
		const VkFence fence, uint32_t *const pImageIndex) noexcept
	{
		return __device.vkAcquireNextImageKHR(
			getHandle(), timeout, semaphore, fence, pImageIndex);
	}

	void Swapchain::__destroy() noexcept
	{
		__device.vkDestroySwapchainKHR(getHandle(), nullptr);
	}

	VkSwapchainKHR Swapchain::__create(Device &device, const VkSwapchainCreateInfoKHR &createInfo)
	{
		VkSwapchainKHR retVal{};
		device.vkCreateSwapchainKHR(&createInfo, nullptr, &retVal);

		if (!retVal)
			throw std::exception{ "Cannot create a VkSwapchainKHR." };

		return retVal;
	}
}