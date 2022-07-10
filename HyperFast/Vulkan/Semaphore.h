#pragma once

#include "Device.h"

namespace Vulkan
{
	class Semaphore final : public Handle<VkSemaphore>
	{
	public:
		Semaphore(Device &device, const VkSemaphoreCreateInfo &createInfo);
		~Semaphore() noexcept;

	private:
		Device &__device;

		void __destroy() noexcept;

		[[nodiscard]]
		static VkSemaphore __create(
			Device &device, const VkSemaphoreCreateInfo &createInfo);
	};
}