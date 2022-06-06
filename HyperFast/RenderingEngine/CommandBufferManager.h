#pragma once

#include "../Infrastructure/Unique.h"
#include "../VulkanLoader/Procedure.h"
#include <vector>

namespace HyperFast
{
	class CommandBufferManager : public Infra::Unique
	{
	public:
		CommandBufferManager(
			const VkDevice device, const VKL::DeviceProcedure &deviceProc,
			const uint32_t queueFamilyIndex, const size_t numMaxBuffers) noexcept;
		
		~CommandBufferManager() noexcept;

		void getNextBuffers(const size_t numBuffers, std::vector<VkCommandBuffer> &retVal);

	private:
		const VkDevice __device;
		const VKL::DeviceProcedure &__deviceProc;
		const uint32_t __queueFamilyIndex;
		const size_t __numMaxBuffers;

		VkCommandPool __commandPool{};

		size_t __cursor{};
		std::vector<VkCommandBuffer> __commandBuffers{};

		void __createCommandPool();
		void __destroyCommandPool() noexcept;
	};
}