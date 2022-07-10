#pragma once

#include "Device.h"

namespace Vulkan
{
	class Buffer final : public Handle<VkBuffer>
	{
	public:
		Buffer(Device &device, const VkBufferCreateInfo &createInfo);
		~Buffer() noexcept;

		void vkGetBufferMemoryRequirements(
			VkMemoryRequirements *const pMemoryRequirements) noexcept;

		VkResult vkBindBufferMemory(
			const VkDeviceMemory memory, const VkDeviceSize memoryOffset) noexcept;

	private:
		Device &__device;

		void __destroy() noexcept;

		[[nodiscard]]
		static VkBuffer __create(
			Device &device, const VkBufferCreateInfo &createInfo);
	};
}