#include "BufferCopyManager.h"

namespace HyperFast
{
	BufferCopyManager::BufferCopyManager(
		Vulkan::Device &device, const uint32_t queueFamilyIndex,
		BufferManager &bufferManager, MemoryManager &memoryManager) noexcept :
		__commandBufferManager
		{
			device, queueFamilyIndex,
			VkCommandBufferLevel::VK_COMMAND_BUFFER_LEVEL_PRIMARY
		},
		__bufferManager{ bufferManager }, __memoryManager{ memoryManager }
	{}

	void BufferCopyManager::begin() noexcept
	{
		const VkCommandBufferBeginInfo beginInfo
		{
			.sType = VkStructureType::VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
			.flags = VkCommandBufferUsageFlagBits::VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
		};

		Vulkan::CommandBuffer &commandBuffer{ __commandBufferManager.get() };
		commandBuffer.vkBeginCommandBuffer(&beginInfo);
	}

	void BufferCopyManager::copy(
		const VkPipelineStageFlags2 srcStageMask, const VkAccessFlags2 srcAccessMask,
		const VkPipelineStageFlags2 dstStageMask, const VkAccessFlags2 dstAccessMask,
		Buffer &dst, const void *const pSrc, const VkDeviceSize srcSize,
		const uint32_t regionCount, const VkBufferCopy *const pRegions) noexcept
	{
		Vulkan::CommandBuffer &commandBuffer{ __commandBufferManager.get() };
		// commandBuffer.vkCmdCopyBuffer();

		/*
			1. srcSize를 모두 담을 수 있는 Staging 버퍼 가져오기
			2. Staging 버퍼로 host 데이터 복사
			3. 배리어 치기
			4. vkCmdCopyBuffer
		*/
	}

	[[nodiscard]]
	Buffer &BufferCopyManager::__getStagingBuffer(const VkDeviceSize size) noexcept
	{
		const auto lowerIt{ __idleStagingBufferMap.lower_bound(size) };
		if (lowerIt != __idleStagingBufferMap.end())
		{
			if (lowerIt->first == size)
			{
				Buffer *const pRetVal{ lowerIt->second };
				__idleStagingBufferMap.erase(lowerIt);
				__pendingStagingBuffers.emplace_back(pRetVal);
				return *pRetVal;
			}
		}

		const auto upperIt{ __idleStagingBufferMap.upper_bound(size) };
		if (upperIt != __idleStagingBufferMap.end())
		{
			Buffer *const pRetVal{ upperIt->second };
			__idleStagingBufferMap.erase(upperIt);
			__pendingStagingBuffers.emplace_back(pRetVal);
			return *pRetVal;
		}

		Buffer *const pRetVal{ nullptr };
		return *pRetVal;
	}

	Buffer *BufferCopyManager::__createStagingBuffer(const VkDeviceSize size) noexcept
	{
		static constexpr VkMemoryPropertyFlags memProps
		{
			VkMemoryPropertyFlagBits::VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
			VkMemoryPropertyFlagBits::VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
		};

		Buffer *const pRetVal
		{
			new Buffer(
				__bufferManager, size,
				VkBufferUsageFlagBits::VK_BUFFER_USAGE_TRANSFER_SRC_BIT)
		};

		const std::shared_ptr<Memory> pMemory
		{
			std::make_shared<Memory>(
				__memoryManager,
				pRetVal->getMemoryRequirements(), memProps, true)
		};

		pRetVal->bindMemory(pMemory, 0ULL);

		return nullptr;
	}
}