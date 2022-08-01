#pragma once

#include "Device.h"

namespace Vulkan
{
	class Fence final : public Handle<VkFence>
	{
	public:
		Fence(Device &device, const VkFenceCreateInfo &createInfo);
		virtual ~Fence() noexcept;

		VkResult wait(const uint64_t timeout) noexcept;
		VkResult reset() noexcept;

	private:
		Device &__device;

		void __destroy() noexcept;

		[[nodiscard]]
		static VkFence __create(
			Device &device, const VkFenceCreateInfo &createInfo);
	};
}