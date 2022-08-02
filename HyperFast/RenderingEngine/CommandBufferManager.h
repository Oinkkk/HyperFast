#pragma once

#include "../Infrastructure/TemporalDeleter.h"
#include "../Vulkan/CommandPool.h"
#include "../Vulkan/CommandBuffer.h"
#include <memory>

namespace HyperFast
{
	class CommandBufferManager : public Infra::Unique
	{
	public:
		CommandBufferManager(
			Vulkan::Device &device, const uint32_t queueFamilyIndex,
			const VkCommandBufferLevel level, Infra::TemporalDeleter &resourceDeleter) noexcept;
		
		virtual ~CommandBufferManager() noexcept;

		void advance() noexcept;

		[[nodiscard]]
		Vulkan::CommandBuffer &get() noexcept;

	private:
		static constexpr inline uint32_t __numCommandPools{ 3U };
		static constexpr inline uint32_t __numCommandBuffersPerPool{ 8U };
		static constexpr inline uint32_t __totalNumCommandBuffers{ __numCommandPools * __numCommandBuffersPerPool };

		Vulkan::Device &__device;
		const uint32_t __queueFamilyIndex;
		const VkCommandBufferLevel __level;
		Infra::TemporalDeleter &__resourceDeleter;

		Vulkan::CommandPool *__commandPools[__numCommandPools];
		std::unique_ptr<Vulkan::CommandBuffer> __commandBuffers[__totalNumCommandBuffers];
		uint32_t __cursor{};

		void __createCommandPools();
		void __allocateCommandBuffers();
	};
}
