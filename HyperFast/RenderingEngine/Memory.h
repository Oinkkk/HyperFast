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

		[[nodiscard]]
		constexpr VkDeviceMemory getBank() const noexcept;

		[[nodiscard]]
		constexpr VkDeviceSize getSize() const noexcept;

		[[nodiscard]]
		constexpr VkDeviceAddress getOffset() const noexcept;

	private:
		MemoryManager &__manager;
		MemoryManager::MemoryImpl *const __pImpl;
	};

	constexpr VkDeviceMemory Memory::getBank() const noexcept
	{
		return __pImpl->getBank();
	}

	constexpr VkDeviceSize Memory::getSize() const noexcept
	{
		return __pImpl->getSize();
	}

	constexpr VkDeviceAddress Memory::getOffset() const noexcept
	{
		return __pImpl->getOffset();
	}
}

