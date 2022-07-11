#pragma once

#include "Device.h"

namespace Vulkan
{
	class Memory final : public Handle<VkDeviceMemory>
	{
	public:
		Memory(Device &device, const VkMemoryAllocateInfo &allocInfo);
		~Memory() noexcept;

		VkResult vkMapMemory(
			const VkDeviceSize offset, const VkDeviceSize size,
			const VkMemoryMapFlags flags, void **const ppData) noexcept;

		void vkUnmapMemory() noexcept;

	private:
		Device &__device;

		void __destroy() noexcept;

		[[nodiscard]]
		static VkDeviceMemory __alloc(
			Device &device, const VkMemoryAllocateInfo &allocInfo);
	};
}