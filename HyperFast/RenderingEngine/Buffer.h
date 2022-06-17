#pragma once

#include "BufferManager.h"

namespace HyperFast
{
	class Buffer : public Infra::Unique
	{
	public:
		Buffer(BufferManager &manager, const VkDeviceSize dataSize, const VkBufferUsageFlags usage);
		~Buffer() noexcept;

		[[nodiscard]]
		constexpr const VkMemoryRequirements &getMemoryRequirements() const noexcept;
		void bindMemory(Memory &memory, const VkDeviceAddress offset) noexcept;

	private:
		BufferManager &__manager;
		BufferManager::BufferImpl *const __pImpl;
	};

	[[nodiscard]]
	constexpr const VkMemoryRequirements &Buffer::getMemoryRequirements() const noexcept
	{
		return __pImpl->getMemoryRequirements();
	}
}
