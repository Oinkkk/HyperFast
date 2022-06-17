#include "MemoryManager.h"

namespace HyperFast
{
	MemoryManager::MemoryImpl::MemoryImpl(
		MemoryBank &bank, const VkDeviceSize size,
		const VkDeviceSize alignment, const bool linearity) noexcept :
		__bank{ bank }, __size{ size }, __offset{ bank.allocate(size, alignment, linearity) }
	{}

	MemoryManager::MemoryImpl::~MemoryImpl() noexcept
	{
		__bank.free(__offset);
	}
}

