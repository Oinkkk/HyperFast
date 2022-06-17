#include "MemoryManager.h"

namespace HyperFast
{
	MemoryManager::MemoryImpl::MemoryImpl(
		MemoryBank &bank, const MemoryBank::MemorySegment &segment) noexcept :
		__bank{ bank }, __size{ segment.size }, __offset{ bank.allocate(segment) }
	{}

	MemoryManager::MemoryImpl::~MemoryImpl() noexcept
	{
		__bank.free(__offset);
	}

	void *MemoryManager::MemoryImpl::map(const VkDeviceSize size, const VkDeviceAddress offset) noexcept
	{

	}
}

