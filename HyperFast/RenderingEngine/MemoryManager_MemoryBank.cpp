#include "MemoryManager.h"
#include "../Infrastructure/Math.h"

namespace HyperFast
{
	MemoryManager::MemoryBank::MemoryBank(
		Vulkan::Device &device, const uint32_t memoryTypeIndex, const VkDeviceSize size) :
		__device{ device }, __memoryTypeIndex{ memoryTypeIndex }, __size{ size }
	{
		__allocateBank();
	}

	MemoryManager::MemoryBank::~MemoryBank() noexcept
	{
		__unmap();
	}

	VkDeviceMemory MemoryManager::MemoryBank::getHandle() noexcept
	{
		return __pBank->getHandle();
	}

	std::optional<MemoryManager::MemoryBank::MemorySegment> MemoryManager::MemoryBank::findSegment(
		const VkDeviceSize memSize, const VkDeviceSize alignment) const noexcept
	{
		if (__segmentMap.empty())
			return MemorySegment{ 0ULL, memSize };

		for (auto segmentIter = __segmentMap.begin(); segmentIter != __segmentMap.end(); )
		{
			auto nextSegmentIter{ segmentIter };
			nextSegmentIter++;

			const VkDeviceAddress curOffset{ segmentIter->first };
			const VkDeviceSize curSize{ segmentIter->second };

			const bool endOfSegment{ nextSegmentIter == __segmentMap.end() };
			const VkDeviceAddress nextOffset{ endOfSegment ? __size : nextSegmentIter->first };

			const VkDeviceAddress targetOffset{ Infra::Math::ceilAlign(curOffset + curSize, alignment) };

			if ((targetOffset + memSize) <= nextOffset)
				return MemorySegment{ targetOffset, memSize };

			segmentIter = nextSegmentIter;
		}

		return std::nullopt;
	}

	VkDeviceAddress MemoryManager::MemoryBank::allocate(const MemorySegment &segment) noexcept
	{
		__segmentMap.emplace(segment.offset, segment.size);
		return segment.offset;
	}

	void MemoryManager::MemoryBank::free(const VkDeviceAddress offset) noexcept
	{
		__segmentMap.erase(offset);
	}

	void *MemoryManager::MemoryBank::map()
	{
		if (!__mapped)
		{
			// there is no requirement that memory be unmapped before it can be used
			__pBank->vkMapMemory(0ULL, VK_WHOLE_SIZE, 0U, &__mapped);
			if (!__mapped)
				throw std::exception{ "Cannot map a memory." };
		}

		return __mapped;
	}

	void MemoryManager::MemoryBank::__allocateBank()
	{
		const VkMemoryAllocateInfo allocInfo
		{
			.sType = VkStructureType::VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
			.allocationSize = __size,
			.memoryTypeIndex = __memoryTypeIndex
		};

		__pBank = std::make_unique<Vulkan::VulkanMemory>(__device, allocInfo);
	}

	void MemoryManager::MemoryBank::__unmap() noexcept
	{
		if (!__mapped)
			return;

		__pBank->vkUnmapMemory();
	}
}