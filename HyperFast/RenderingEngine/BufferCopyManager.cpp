#include "BufferCopyManager.h"

namespace HyperFast
{
	BufferCopyManager::BufferCopyManager(
		BufferManager &bufferManager, MemoryManager &memoryManager,
		InstantCommandSubmitter &instantCommandSubmitter) noexcept :
		__bufferManager{ bufferManager }, __memoryManager{ memoryManager },
		__instantCommandSubmitter{ instantCommandSubmitter }
	{}

	BufferCopyManager::~BufferCopyManager() noexcept
	{}

	void BufferCopyManager::copy(
		const VkPipelineStageFlags2 srcStageMask, const VkAccessFlags2 srcAccessMask,
		const VkPipelineStageFlags2 dstStageMask, const VkAccessFlags2 dstAccessMask,
		const VkBuffer dst, const void *const pSrc, const VkDeviceSize srcBufferSize,
		const uint32_t regionCount, const VkBufferCopy *const pRegions,
		const std::any &srcPlaceholder) noexcept
	{
		Buffer *const pStagingBuffer{ __getStagingBuffer(pSrc, srcBufferSize) };

		__instantCommandSubmitter.vkCmdCopyBuffer(
			srcStageMask, srcAccessMask, dstStageMask, dstAccessMask,
			pStagingBuffer->getHandle(), dst, regionCount, pRegions);

		__buffer2ResourceMap.emplace(
			dst, Resource{ pStagingBuffer, __instantCommandSubmitter.getCurrentExecution(), srcPlaceholder });
	}

	void BufferCopyManager::refresh() noexcept
	{
		for (auto mapIter = __buffer2ResourceMap.begin(); mapIter != __buffer2ResourceMap.end(); )
		{
			auto &[pStagingBuffer, execution, _] { mapIter->second };
			if (execution.wait_for(std::chrono::seconds{ 0 }) == std::future_status::ready)
			{
				__idleStagingBuffers.emplace(pStagingBuffer);
				mapIter = __buffer2ResourceMap.erase(mapIter);
			}
			else
				mapIter++;
		}
	}

	bool BufferCopyManager::isBusy(const VkBuffer buffer) noexcept
	{
		return __buffer2ResourceMap.contains(buffer);
	}

	Buffer *BufferCopyManager::__createStagingBuffer(const VkDeviceSize size)
	{
		Buffer *const pRetVal
		{
			__stagingBuffers.emplace_back(
				std::make_unique<Buffer>(
					__bufferManager, size,
					VkBufferUsageFlagBits::VK_BUFFER_USAGE_TRANSFER_SRC_BIT)).get()
		};

		static constexpr VkMemoryPropertyFlags memoryProps
		{
			VkMemoryPropertyFlagBits::VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
			VkMemoryPropertyFlagBits::VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
		};

		const std::shared_ptr<Memory> pMemory
		{
			std::make_shared<Memory>(
				__memoryManager, pRetVal->getMemoryRequirements(),
				memoryProps, true)
		};

		pRetVal->bindMemory(pMemory, 0ULL);
		return pRetVal;
	}

	Buffer *BufferCopyManager::__getStagingBuffer(const void *const pData, const VkDeviceSize size)
	{
		Buffer *pRetVal{};

		for (
			auto idleBufferIter = __idleStagingBuffers.begin();
			idleBufferIter != __idleStagingBuffers.end();
			idleBufferIter++)
		{
			Buffer *const pBuffer{ *idleBufferIter };
			if (pBuffer->getSize() < size)
				continue;

			pRetVal = pBuffer;
			__idleStagingBuffers.erase(idleBufferIter);
			break;
		}

		if (!pRetVal)
			pRetVal = __createStagingBuffer(size);

		const std::shared_ptr<HyperFast::Memory> &pMemory{ pRetVal->getMemory() };
		std::memcpy(pMemory->map(), pData, size);
		pMemory->unmap();

		return pRetVal;
	}
}