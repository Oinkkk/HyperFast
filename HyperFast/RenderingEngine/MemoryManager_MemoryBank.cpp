#include "MemoryManager.h"
#include "../Infrastructure/Math.h"

namespace HyperFast
{
	MemoryManager::MemoryBank::MemoryBank(
		const VkDevice device, const VKL::DeviceProcedure &deviceProc,
		const uint32_t memoryTypeIndex, const VkMemoryPropertyFlags props,
		const VkDeviceSize nonCoherentAtomSize, const VkDeviceSize size) :
		__device{ device }, __deviceProc{ deviceProc },
		__memoryTypeIndex{ memoryTypeIndex }, __props{ props },
		__nonCoherentAtomSize{ nonCoherentAtomSize }, __size{ size }
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

	void MemoryManager::MemoryBank::map(const VkDeviceAddress offset, const VkDeviceSize size) noexcept
	{
		/*
			If the device memory was allocated without the VK_MEMORY_PROPERTY_HOST_COHERENT_BIT set,
			these guarantees must be made for an extended range:
			the application must round down the start of the range to the
			nearest multiple of VkPhysicalDeviceLimits::nonCoherentAtomSize,
			and round the end of the range up to the
			nearest multiple of VkPhysicalDeviceLimits::nonCoherentAtomSize
		*/
		VkDeviceAddress rangeStart{ offset };
		VkDeviceAddress rangeEnd{ offset + size };

		const bool hostCoherent{ isHostCoherent() };
		if (!hostCoherent)
		{
			rangeStart = Infra::Math::floorAlign(rangeStart, __nonCoherentAtomSize);
			rangeEnd = Infra::Math::ceilAlign(rangeEnd, __nonCoherentAtomSize);
		}

		__deviceProc.vkMapMemory(
			__device, __handle, rangeStart, rangeEnd - rangeStart,
			0U, &(reinterpret_cast<void *&>(__mapped)));

		if (!hostCoherent)
			__mapped += (offset - rangeStart);
	}

	void MemoryManager::MemoryBank::unmap() noexcept
	{
		__deviceProc.vkUnmapMemory(__device, __handle);
	}

	void MemoryManager::MemoryBank::write(
		const VkDeviceSize srcOffset, const VkDeviceSize size, const void *pData) noexcept
	{
		std::memcpy(__mapped + srcOffset, pData, size);

		const bool hostCoherent{ isHostCoherent() };
		if (!hostCoherent)
		{
			//__deviceProc.vkFlushMappedMemoryRanges(__device, );
		}
	}

	void MemoryManager::MemoryBank::read(
		const VkDeviceAddress srcOffset, const VkDeviceSize size, void *const pBuffer) noexcept
	{

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

