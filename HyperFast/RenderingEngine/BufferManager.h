#pragma once

#include "../Infrastructure/Unique.h"
#include "../VulkanLoader/VulkanLoader.h"

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
				const VkDeviceSize memSize, const VkBufferUsageFlags usage);

			~BufferImpl() noexcept;

		private:
			const VkDevice __device;
			const VKL::DeviceProcedure &__deviceProc;

			VkBuffer __buffer{};

			void __createBuffer(const VkDeviceSize memSize, const VkBufferUsageFlags usage);
			void __destroyBuffer() noexcept;
		};

		BufferManager(const VkDevice device, const VKL::DeviceProcedure &deviceProc) noexcept;

		[[nodiscard]]
		BufferImpl *create(const VkDeviceSize memSize, const VkBufferUsageFlags usage);
		void destroy(BufferImpl *const pImpl) noexcept;

	private:
		const VkDevice __device;
		const VKL::DeviceProcedure &__deviceProc;
	};
}