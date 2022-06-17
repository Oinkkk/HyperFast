#include "MemoryManager.h"
#include "../Infrastructure/Math.h"

namespace HyperFast
{
	MemoryManager::MemoryBank::MemoryBank(
		const VkDevice device, const VKL::DeviceProcedure &deviceProc,
		const uint32_t memoryTypeIndex, const VkDeviceSize size) :
		__device{ device }, __deviceProc{ deviceProc },
		__memoryTypeIndex{ memoryTypeIndex }, __size{ size }
	{
		__allocateBank();
	}

	MemoryManager::MemoryBank::~MemoryBank() noexcept
	{
		__freeBank();
	}

	std::optional<MemoryManager::MemoryBank::MemorySegment> MemoryManager::MemoryBank::findSegment(
		const VkDeviceSize memSize, const VkDeviceSize alignment) const noexcept
	{
		for (auto segmentIter = __segmentMap.begin(); segmentIter != __segmentMap.end(); )
		{
			auto nextSegmentIter{ segmentIter };
			nextSegmentIter++;

			const VkDeviceAddress curOffset{ segmentIter->first };
			const VkDeviceSize curSize{ segmentIter->second };

			const bool endOfSegment{ nextSegmentIter != __segmentMap.end() };
			const VkDeviceAddress nextOffset{ endOfSegment ? nextSegmentIter->first : __size };

			const VkDeviceAddress targetOffset{ Infra::Math::ceilAlign(curOffset + curSize, alignment) };

			if ((targetOffset + memSize) <= nextOffset)
				return MemorySegment{ targetOffset, memSize };
		}

		return std::nullopt;
	}

	VkDeviceAddress MemoryManager::MemoryBank::allocate(const MemorySegment &segment)
	{
		__segmentMap.emplace(segment.offset, segment.size);
		return segment.offset;
	}

	void MemoryManager::MemoryBank::free(const VkDeviceAddress offset) noexcept
	{
		__segmentMap.erase(offset);
	}

	void MemoryManager::MemoryBank::__allocateBank()
	{
		const VkMemoryAllocateInfo allocInfo
		{
			.sType = VkStructureType::VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
			.allocationSize = __size,
			.memoryTypeIndex = __memoryTypeIndex
		};

		__deviceProc.vkAllocateMemory(__device, &allocInfo, nullptr, &__handle);
		if (!__handle)
			throw std::exception{ "Cannot create a VkDeviceMemory." };
	}

	void MemoryManager::MemoryBank::__freeBank() noexcept
	{
		__deviceProc.vkFreeMemory(__device, __handle, nullptr);
	}
}

