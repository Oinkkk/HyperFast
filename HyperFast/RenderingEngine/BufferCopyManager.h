#pragma once

#include "Buffer.h"
#include "Memory.h"
#include "InstantCommandSubmitter.h"
#include <unordered_set>
#include <any>

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
			const uint32_t regionCount, const VkBufferCopy *const pRegions,
			const std::any &srcPlaceholder = {}) noexcept;

		void refresh() noexcept;

	private:
		class Resource
		{
		public:
			Buffer *pStagingBuffer{};
			std::shared_future<void> execution;
			std::any srcPlaceholder;
		};

		BufferManager &__bufferManager;
		MemoryManager &__memoryManager;
		InstantCommandSubmitter &__instantCommandSubmitter;

		std::vector<std::unique_ptr<Buffer>> __stagingBuffers;

		std::unordered_set<Buffer *> __idleStagingBuffers;
		std::unordered_map<VkBuffer, Resource> __buffer2ResourceMap;

		[[nodiscard]]
		Buffer *__createStagingBuffer(const VkDeviceSize size);

		[[nodiscard]]
		Buffer *__getStagingBuffer(const void *const pData, const VkDeviceSize size);
	};
}