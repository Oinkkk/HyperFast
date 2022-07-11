#pragma once

#include "Device.h"

namespace Vulkan
{
	class Semaphore final : public Handle<VkSemaphore>
	{
	public:
		Semaphore(Device &device, const VkSemaphoreCreateInfo &createInfo);
		~Semaphore() noexcept;

		VkResult signal(const uint64_t value) noexcept;
		VkResult wait(const uint64_t value, const uint64_t timeout) noexcept;

		VkResult vkGetSemaphoreCounterValue(uint64_t *const pValue) noexcept;

	private:
		Device &__device;

		void __destroy() noexcept;

		[[nodiscard]]
		static VkSemaphore __create(
			Device &device, const VkSemaphoreCreateInfo &createInfo);
	};
}