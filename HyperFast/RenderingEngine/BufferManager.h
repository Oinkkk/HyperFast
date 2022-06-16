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
				const VkDevice device, const VKL::DeviceProcedure &deviceProc, MemoryManager &memoryManager,
				const VkDeviceSize dataSize, const VkBufferUsageFlags usage);

			~BufferImpl() noexcept;

		private:
			const VkDevice __device;
			const VKL::DeviceProcedure &__deviceProc;

			MemoryManager &__memoryManager;

			VkBuffer __buffer{};
			VkMemoryRequirements __memRequirements{};
			std::unique_ptr<Memory> __pMemory;

			void __createBuffer(const VkDeviceSize dataSize, const VkBufferUsageFlags usage);
			void __destroyBuffer() noexcept;
			void __queryMemoryRequirements() noexcept;
			void __allocMemory();
			void __freeMemory() noexcept;
		};

		BufferManager(
			const VkDevice device, const VKL::DeviceProcedure &deviceProc, MemoryManager &memoryManager) noexcept;

		[[nodiscard]]
		BufferImpl *create(const VkDeviceSize dataSize, const VkBufferUsageFlags usage);
		void destroy(BufferImpl *const pImpl) noexcept;

	private:
		const VkDevice __device;
		const VKL::DeviceProcedure &__deviceProc;

		MemoryManager &__memoryManager;
	};
}