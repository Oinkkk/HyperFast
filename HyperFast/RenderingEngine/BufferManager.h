#pragma once

#include "Memory.h"
#include <memory>

namespace HyperFast
{
	class BufferManager final : public Infra::Unique
	{
	public:
		class BufferImpl final : public Infra::Unique
		{
		public:
			BufferImpl(
				const VkDevice device, const VKL::DeviceProcedure &deviceProc,
				const VkDeviceSize size, const VkBufferUsageFlags usage);

			~BufferImpl() noexcept;

			[[nodiscard]]
			constexpr VkDeviceSize getSize() const noexcept;

			[[nodiscard]]
			constexpr VkBufferUsageFlags getUsage() const noexcept;

			[[nodiscard]]
			constexpr VkBuffer getHandle() const noexcept;

			[[nodiscard]]
			constexpr const VkMemoryRequirements &getMemoryRequirements() const noexcept;
			void bindMemory(const std::shared_ptr<Memory> &pMemory, const VkDeviceAddress offset) noexcept;

		private:
			const VkDevice __device;
			const VKL::DeviceProcedure &__deviceProc;
			const VkDeviceSize __size;
			const VkBufferUsageFlags __usage;

			VkBuffer __buffer{};
			VkMemoryRequirements __memRequirements{};

			std::shared_ptr<Memory> __pMemory;

			void __createBuffer(const VkDeviceSize dataSize, const VkBufferUsageFlags usage);
			void __destroyBuffer() noexcept;
			void __queryMemoryRequirements() noexcept;
		};

		BufferManager(
			const VkDevice device, const VKL::DeviceProcedure &deviceProc) noexcept;

		[[nodiscard]]
		BufferImpl *create(const VkDeviceSize dataSize, const VkBufferUsageFlags usage);
		void destroy(BufferImpl *const pImpl) noexcept;

	private:
		const VkDevice __device;
		const VKL::DeviceProcedure &__deviceProc;
	};

	constexpr VkDeviceSize BufferManager::BufferImpl::getSize() const noexcept
	{
		return __size;
	}

	constexpr VkBufferUsageFlags BufferManager::BufferImpl::getUsage() const noexcept
	{
		return __usage;
	}

	constexpr VkBuffer BufferManager::BufferImpl::getHandle() const noexcept
	{
		return __buffer;
	}

	constexpr const VkMemoryRequirements &BufferManager::BufferImpl::getMemoryRequirements() const noexcept
	{
		return __memRequirements;
	}
}