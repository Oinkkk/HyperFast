#pragma once

#include "MemoryManager.h"

namespace HyperFast
{
	class Memory : public Infra::Unique
	{
	public:
		Memory(
			MemoryManager &manager,
			const VkMemoryRequirements &memRequirements,
			const VkMemoryPropertyFlags requiredProps, const bool linearity);
		
		~Memory() noexcept;

		[[nodiscard]]
		VkDeviceMemory getBank() const noexcept;

		[[nodiscard]]
		constexpr VkDeviceSize getSize() const noexcept;

		[[nodiscard]]
		constexpr VkDeviceAddress getOffset() const noexcept;

		[[nodiscard]]
		void *map();

	private:
		MemoryManager &__manager;
		MemoryManager::MemoryImpl *const __pImpl;
	};

	constexpr VkDeviceSize Memory::getSize() const noexcept
	{
		return __pImpl->getSize();
	}

	constexpr VkDeviceAddress Memory::getOffset() const noexcept
	{
		return __pImpl->getOffset();
	}
}

