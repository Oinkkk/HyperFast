#pragma once

#include "Device.h"

namespace Vulkan
{
	class Swapchain final : public Handle<VkSwapchainKHR>
	{
	public:
		Swapchain(Device &device, const VkSwapchainCreateInfoKHR &createInfo);
		~Swapchain() noexcept;

		VkResult vkGetSwapchainImagesKHR(
			uint32_t *const pSwapchainImageCount, VkImage *const pSwapchainImages) noexcept;

		VkResult vkAcquireNextImageKHR(
			const uint64_t timeout, const VkSemaphore semaphore,
			const VkFence fence, uint32_t *const pImageIndex) noexcept;

	private:
		Device &__device;

		void __destroy() noexcept;

		[[nodiscard]]
		static VkSwapchainKHR __create(
			Device &device, const VkSwapchainCreateInfoKHR &createInfo);
	};
}