#include "MemoryManager.h"

namespace HyperFast
{
	MemoryManager::MemoryImpl::MemoryImpl(
		MemoryBank &bank, const VkDeviceSize size, const VkDeviceSize alignment) noexcept :
		__bank{ bank }, __size{ size }, __offset{ bank.allocate(size, alignment) }
	{}

	MemoryManager::MemoryImpl::~MemoryImpl() noexcept
	{
		__bank.free(__offset);
	}
}

