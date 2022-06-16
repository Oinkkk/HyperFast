#include "Memory.h"

namespace HyperFast
{
	Memory::Memory(
		MemoryManager &manager, const VkDeviceSize memSize,
		const VkDeviceSize alignment, const uint32_t memoryTypeBits) :
		__manager{ manager }, __pImpl{ manager.create(memSize, alignment, memoryTypeBits) }
	{}

	Memory::~Memory() noexcept
	{
		__manager.destroy(__pImpl);
	}
}