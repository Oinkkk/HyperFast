#pragma once

#include "../Vulkan/Queue.h"
#include "../Vulkan/Fence.h"
#include "SubmitLayerType.h"
#include <map>
#include <vector>
#include <memory>
#include <future>

namespace HyperFast
{
	class CommandSubmitter : public Infra::Unique
	{
	public:
		CommandSubmitter(Vulkan::Device &device, Vulkan::Queue &queue) noexcept;

		std::shared_future<void> enqueue(
			const SubmitLayerType layerType,
			const uint32_t waitSemaphoreInfoCount,
			const VkSemaphoreSubmitInfo *const pWaitSemaphoreInfos,
			const uint32_t commandBufferInfoCount,
			const VkCommandBufferSubmitInfo *const pCommandBufferInfos,
			const uint32_t signalSemaphoreInfoCount,
			const VkSemaphoreSubmitInfo *const pSignalSemaphoreInfos) noexcept;

		void submit();

	private:
		Vulkan::Device &__device;
		Vulkan::Queue &__queue;

		std::map<SubmitLayerType, std::vector<VkSubmitInfo2>> __layer2InfosMap;
		std::vector<VkSubmitInfo2> __infoStream;

		std::vector<std::unique_ptr<Vulkan::Fence>> __submitFences;
		size_t __currentSubmitFenceIdx{};
		std::shared_future<void> __currentSubmitFuture;

		std::unordered_map<Vulkan::Fence *, std::promise<void>> __fence2PromiseMap;

		[[nodiscard]]
		Vulkan::Fence *__getCurrentSubmitFence() noexcept;

		void __appendSubmitFence();
		void __nextSubmitFenceIdx();
	};
}