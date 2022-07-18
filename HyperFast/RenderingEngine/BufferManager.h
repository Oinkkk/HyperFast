#pragma once

#include "Memory.h"
#include <memory>
#include "../Vulkan/Buffer.h"

namespace HyperFast
{
	class BufferManager final : public Infra::Unique
	{
	public:
		class BufferImpl final : public Infra::Unique
		{
		public:
			BufferImpl(
				Vulkan::Device &device,
				const VkDeviceSize size, const VkBufferUsageFlags usage);

			[[nodiscard]]
			constexpr VkDeviceSize getSize() const noexcept;

			[[nodiscard]]
			constexpr VkBufferUsageFlags getUsage() const noexcept;

			[[nodiscard]]
			VkBuffer getHandle() noexcept;

			[[nodiscard]]
			constexpr const VkMemoryRequirements &getMemoryRequirements() const noexcept;
			void bindMemory(const std::shared_ptr<Memory> &pMemory, const VkDeviceAddress offset) noexcept;

			[[nodiscard]]
			constexpr const std::shared_ptr<Memory> &getMemory() const noexcept;

			[[nodiscard]]
			constexpr VkDeviceAddress getMemoryOffset() const noexcept;

		private:
			Vulkan::Device &__device;
			const VkDeviceSize __size;
			const VkBufferUsageFlags __usage;

			std::unique_ptr<Vulkan::Buffer> __pBuffer;
			VkMemoryRequirements __memRequirements{};

			std::shared_ptr<Memory> __pMemory;
			VkDeviceAddress __memoryOffset{};

			void __createBuffer(const VkDeviceSize dataSize, const VkBufferUsageFlags usage);
			void __queryMemoryRequirements() noexcept;
		};

		BufferManager(Vulkan::Device &device) noexcept;

		[[nodiscard]]
		BufferImpl *create(const VkDeviceSize dataSize, const VkBufferUsageFlags usage);
		void destroy(BufferImpl *const pImpl) noexcept;

	private:
		Vulkan::Device &__device;
		std::vector<BufferImpl *> __destroyReserved;
	};

	constexpr VkDeviceSize BufferManager::BufferImpl::getSize() const noexcept
	{
		return __size;
	}

	constexpr VkBufferUsageFlags BufferManager::BufferImpl::getUsage() const noexcept
	{
		return __usage;
	}

	constexpr const VkMemoryRequirements &BufferManager::BufferImpl::getMemoryRequirements() const noexcept
	{
		return __memRequirements;
	}

	constexpr const std::shared_ptr<Memory> &BufferManager::BufferImpl::getMemory() const noexcept
	{
		return __pMemory;
	}

	constexpr VkDeviceAddress BufferManager::BufferImpl::getMemoryOffset() const noexcept
	{
		return __memoryOffset;
	}
}