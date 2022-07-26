#pragma once

#include "../Infrastructure/Unique.h"
#include "../Vulkan/CommandPool.h"
#include <memory>
#include <vector>
#include "../Vulkan/CommandBuffer.h"

namespace HyperFast
{
	class CommandBufferManager : public Infra::Unique
	{
	public:
		CommandBufferManager(
			Vulkan::Device &device, const uint32_t queueFamilyIndex,
			const VkCommandBufferLevel level, const size_t numMaxBuffers = 8ULL) noexcept;
		
		void advance() noexcept;

		[[nodiscard]]
		Vulkan::CommandBuffer &get() noexcept;

	private:
		Vulkan::Device &__device;
		const uint32_t __queueFamilyIndex;
		const VkCommandBufferLevel __level;
		const size_t __numMaxBuffers;

		std::unique_ptr<Vulkan::CommandPool> __pCommandPool;

		size_t __cursor{};
		std::vector<std::unique_ptr<Vulkan::CommandBuffer>> __commandBuffers{};

		void __createCommandPool();
		void __allocateCommandBuffers();
	};
}
