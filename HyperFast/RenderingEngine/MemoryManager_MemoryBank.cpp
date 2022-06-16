#include "MemoryManager.h"

namespace HyperFast
{
	MemoryManager::MemoryBank::MemoryBank(
		const VkDevice device, const VKL::DeviceProcedure &deviceProc, const VkDeviceSize size) :
		__device{ device }, __deviceProc{ deviceProc }, __size{ size }
	{
		__allocateBank();
	}

	MemoryManager::MemoryBank::~MemoryBank() noexcept
	{
		__freeBank();
	}

	VkDeviceAddress MemoryManager::MemoryBank::allocate(const VkDeviceSize memSize, const VkDeviceSize alignment)
	{

	}

	void MemoryManager::MemoryBank::free(const VkDeviceAddress offset) noexcept
	{

	}

	void MemoryManager::MemoryBank::__allocateBank()
	{

	}

	void MemoryManager::MemoryBank::__freeBank() noexcept
	{

	}
}

