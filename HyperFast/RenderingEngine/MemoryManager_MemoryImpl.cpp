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

	void *MemoryManager::MemoryImpl::map()
	{
		return (reinterpret_cast<std::byte *>(__bank.map()) + __offset);
	}

	void MemoryManager::MemoryImpl::unmap() noexcept
	{
		return __bank.unmap();
	}
}
