#pragma once

#include "MemoryManager.h"

namespace HyperFast
{
	class Memory : public Infra::Unique
	{
	public:
		Memory(
			MemoryManager &manager, const VkDeviceSize memSize,
			const VkDeviceSize alignment, const uint32_t memoryTypeBits);
		
		~Memory() noexcept;

	private:
		MemoryManager &__manager;
		MemoryManager::MemoryImpl *const __pImpl;
	};
}

