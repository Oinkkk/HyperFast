#pragma once

#include "Device.h"

namespace Vulkan
{
	class Queue final : public Handle<VkQueue>
	{
	public:
		Queue(Device &device, const uint32_t queueFamilyIndex, const uint32_t queueIndex) noexcept;

		VkResult vkQueueWaitIdle() noexcept;

		VkResult vkQueueSubmit2(
			const uint32_t submitCount,
			const VkSubmitInfo2 *const pSubmits, const VkFence fence) noexcept;

		VkResult vkQueuePresentKHR(const VkPresentInfoKHR *const pPresentInfo) noexcept;

	private:
		Device &__device;

		[[nodiscard]]
		static VkQueue __retrieve(
			Device &device, const uint32_t queueFamilyIndex, const uint32_t queueIndex) noexcept;
	};
}