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

		void enqueue(
			const SubmitLayerType layerType,
			const uint32_t waitSemaphoreInfoCount,
			const VkSemaphoreSubmitInfo *const pWaitSemaphoreInfos,
			const uint32_t commandBufferInfoCount,
			const VkCommandBufferSubmitInfo *const pCommandBufferInfos,
			const uint32_t signalSemaphoreInfoCount,
			const VkSemaphoreSubmitInfo *const pSignalSemaphoreInfos) noexcept;

		void submit() noexcept;

	private:
		Vulkan::Device &__device;
		Vulkan::Queue &__queue;

		std::map<SubmitLayerType, std::vector<VkSubmitInfo2>> __layer2InfosMap;
		std::vector<VkSubmitInfo2> __infoStream;
	};
}