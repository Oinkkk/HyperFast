#pragma once

#include "../Infrastructure/Unique.h"
#include "../Vulkan/CommandPool.h"
#include <memory>
#include <vector>

namespace HyperFast
{
	class CommandBufferManager : public Infra::Unique
	{
	public:
		CommandBufferManager(
			Vulkan::Device &device, const uint32_t queueFamilyIndex,
			const size_t numMaxBuffers) noexcept;
		
		[[nodiscard]]
		VkCommandBuffer getNextBuffer();

	private:
		const uint32_t __queueFamilyIndex;
		const size_t __numMaxBuffers;

		std::unique_ptr<Vulkan::CommandPool> __pCommandPool;

		size_t __cursor{};
		std::vector<VkCommandBuffer> __commandBuffers{};

		void __createCommandPool(Vulkan::Device &device);
		void __allocateCommandBuffers();
	};
}
