#pragma once

#include "Buffer.h"
#include "Memory.h"
#include "InstantCommandSubmitter.h"
#include <unordered_set>

namespace HyperFast
{
	class BufferCopyManager : public Infra::Unique
	{
	public:
		BufferCopyManager(
			BufferManager &bufferManager, MemoryManager &memoryManager,
			InstantCommandSubmitter &instantCommandSubmitter) noexcept;

		~BufferCopyManager() noexcept;

		void copy(
			const VkPipelineStageFlags2 srcStageMask, const VkAccessFlags2 srcAccessMask,
			const VkPipelineStageFlags2 dstStageMask, const VkAccessFlags2 dstAccessMask,
			const VkBuffer dst, const void *const pSrc, const VkDeviceSize srcBufferSize,
			const uint32_t regionCount, const VkBufferCopy *const pRegions) noexcept;

		[[nodiscard]]
		bool isBusy(const VkBuffer buffer) noexcept;

	private:
		BufferManager &__bufferManager;
		MemoryManager &__memoryManager;
		InstantCommandSubmitter &__instantCommandSubmitter;

		std::vector<std::unique_ptr<Buffer>> __stagingBuffers;

		std::unordered_set<Buffer *> __idleStagingBuffers;
		std::unordered_map<VkBuffer, std::pair<Buffer *, std::future<void>>> __buffer2ResourceMap;

		[[nodiscard]]
		Buffer *__createStagingBuffer(const VkDeviceSize size);

		[[nodiscard]]
		Buffer *__getStagingBuffer(const void *const pData, const VkDeviceSize size);
	};
}