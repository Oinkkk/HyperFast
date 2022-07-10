#pragma once

#include "BufferManager.h"

namespace HyperFast
{
	class Buffer : public Infra::Unique
	{
	public:
		Buffer(BufferManager &manager, const VkDeviceSize size, const VkBufferUsageFlags usage);
		~Buffer() noexcept;

		[[nodiscard]]
		constexpr VkDeviceSize getSize() const noexcept;

		[[nodiscard]]
		constexpr VkBufferUsageFlags getUsage() const noexcept;

		[[nodiscard]]
		VkBuffer getHandle() const noexcept;

		[[nodiscard]]
		constexpr const VkMemoryRequirements &getMemoryRequirements() const noexcept;
		void bindMemory(const std::shared_ptr<Memory> &pMemory, const VkDeviceAddress offset) noexcept;

		[[nodiscard]]
		constexpr const std::shared_ptr<Memory> &getMemory() const noexcept;

		[[nodiscard]]
		constexpr VkDeviceAddress getMemoryOffset() const noexcept;

	private:
		BufferManager &__manager;
		BufferManager::BufferImpl *const __pImpl;
	};

	constexpr VkDeviceSize Buffer::getSize() const noexcept
	{
		return __pImpl->getSize();
	}

	constexpr VkBufferUsageFlags Buffer::getUsage() const noexcept
	{
		return __pImpl->getUsage();
	}

	constexpr const VkMemoryRequirements &Buffer::getMemoryRequirements() const noexcept
	{
		return __pImpl->getMemoryRequirements();
	}

	constexpr const std::shared_ptr<Memory> &Buffer::getMemory() const noexcept
	{
		return __pImpl->getMemory();
	}

	constexpr VkDeviceAddress Buffer::getMemoryOffset() const noexcept
	{
		return __pImpl->getMemoryOffset();
	}
}
