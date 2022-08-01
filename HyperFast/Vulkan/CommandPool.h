#pragma once

#include "Device.h"

namespace Vulkan
{
	class CommandPool final : public Handle<VkCommandPool>
	{
	public:
		CommandPool(Device &device, const VkCommandPoolCreateInfo &createInfo);
		virtual ~CommandPool() noexcept;

		VkResult vkResetCommandPool(const VkCommandPoolResetFlags flags) noexcept;
		VkResult vkAllocateCommandBuffers(
			const void *const pNext, const VkCommandBufferLevel level,
			const uint32_t commandBufferCount, VkCommandBuffer *const pCommandBuffers) noexcept;

	private:
		Device &__device;

		void __destroy() noexcept;

		[[nodiscard]]
		static VkCommandPool __create(
			Device &device, const VkCommandPoolCreateInfo &createInfo);
	};
}