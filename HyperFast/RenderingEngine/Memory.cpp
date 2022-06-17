#include "Memory.h"

namespace HyperFast
{
	Memory::Memory(
		MemoryManager &manager,
		const VkMemoryRequirements &memRequirements,
		const VkMemoryPropertyFlags requiredProps, const bool linearity) :
		__manager{ manager }, __pImpl{ manager.create(memRequirements, requiredProps, linearity) }
	{}

	Memory::~Memory() noexcept
	{
		__manager.destroy(__pImpl);
	}
}