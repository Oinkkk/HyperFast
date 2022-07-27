#pragma once

#include "Buffer.h"
#include "CommandBufferManager.h"

namespace HyperFast
{
	class BufferCopyManager : public Infra::Unique
	{
	public:
		BufferCopyManager(
			Vulkan::Device &device, const uint32_t queueFamilyIndex,
			BufferManager &bufferManager, MemoryManager &memoryManager) noexcept;

		void begin() noexcept;

		void copy(
			const VkPipelineStageFlags2 srcStageMask, const VkAccessFlags2 srcAccessMask,
			const VkPipelineStageFlags2 dstStageMask, const VkAccessFlags2 dstAccessMask,
			Buffer &dst, const void *const pSrc, const VkDeviceSize srcSize,
			const uint32_t regionCount, const VkBufferCopy *const pRegions) noexcept;

	private:
		CommandBufferManager __commandBufferManager;
		BufferManager &__bufferManager;
		MemoryManager &__memoryManager;

		std::map<VkDeviceSize, Buffer *> __idleStagingBufferMap;
		std::vector<Buffer *> __pendingStagingBuffers;

		[[nodiscard]]
		Buffer &__getStagingBuffer(const VkDeviceSize size) noexcept;

		[[nodiscard]]
		Buffer *__createStagingBuffer(const VkDeviceSize size) noexcept;
	};
}
