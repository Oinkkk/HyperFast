#pragma once

#include "../VulkanLoader/Procedure.h"
#include "../Infrastructure/Unique.h"
#include <queue>
#include <unordered_map>
#include "../Infrastructure/Event.h"
#include <future>

namespace HyperFast
{
	class InstantCommandSubmitter : public Infra::Unique
	{
	public:
		InstantCommandSubmitter(
			const VkDevice device, const VKL::DeviceProcedure &deviceProc,
			const uint32_t queueFamilyIndex, const VkQueue queue) noexcept;

		~InstantCommandSubmitter() noexcept;

		void begin() noexcept;
		void end() noexcept;
		void submit() noexcept;
		void refresh() noexcept;

		[[nodiscard]]
		std::shared_future<void> getCurrentExecution() noexcept;

		void vkCmdCopyBuffer(
			const VkPipelineStageFlags2 srcStageMask, const VkAccessFlags2 srcAccessMask,
			const VkPipelineStageFlags2 dstStageMask, const VkAccessFlags2 dstAccessMask,
			const VkBuffer srcBuffer, const VkBuffer dstBuffer,
			const uint32_t regionCount, const VkBufferCopy *const pRegions) noexcept;

	private:
		const VkDevice __device;
		const VKL::DeviceProcedure &__deviceProc;
		const uint32_t __queueFamilyIndex;
		const VkQueue __queue;

		VkCommandPool __commandPool{};
		std::vector<VkFence> __fences;

		std::list<std::pair<VkFence, VkCommandBuffer>> __idleResources;
		std::unordered_map<VkFence, std::pair<VkCommandBuffer, std::unique_ptr<std::promise<void>>>> __busyResources;

		VkFence __currentFence{};
		VkCommandBuffer __currentCommandBuffer{};
		std::unique_ptr<std::promise<void>> __pCurrentPromise;
		std::shared_future<void> __currentFuture;

		bool __initState{};

		Infra::Event<InstantCommandSubmitter &> __completeEvent;

		void __createCommandPool();
		void __destroyCommandPool() noexcept;
		void __destroyFences() noexcept;

		void __retrieveResources();

		[[nodiscard]]
		VkFence __createFence();

		[[nodiscard]]
		VkCommandBuffer __allocateCommandBuffer();

		[[nodiscard]]
		bool __isFinished(const VkFence fence) const noexcept;
		void __resetFence(const VkFence fence) noexcept;
		void __resetCommandBuffer(const VkCommandBuffer commandBuffer) noexcept;
	};
}