#pragma once

#include "../Vulkan/Queue.h"
#include "../Vulkan/Fence.h"
#include <map>
#include <vector>
#include <queue>
#include "../Infrastructure/Event.h"

namespace HyperFast
{
	class CommandSubmitter : public Infra::Unique
	{
	public:
		CommandSubmitter(Vulkan::Device &device, Vulkan::Queue &queue) noexcept;
		~CommandSubmitter() noexcept;

		void enqueue(
			const uint32_t waitSemaphoreInfoCount,
			const VkSemaphoreSubmitInfo *const pWaitSemaphoreInfos,
			const uint32_t commandBufferInfoCount,
			const VkCommandBufferSubmitInfo *const pCommandBufferInfos,
			const uint32_t signalSemaphoreInfoCount,
			const VkSemaphoreSubmitInfo *const pSignalSemaphoreInfos) noexcept;

		bool submit();
		void flush() noexcept;

		[[nodiscard]]
		constexpr Infra::EventView<size_t> &getFinishEvent() noexcept;

	private:
		Vulkan::Device &__device;
		Vulkan::Queue &__queue;

		std::vector<VkSubmitInfo2> __submitInfos;

		size_t __timestamp{};
		std::deque<Vulkan::Fence *> __idleFences;
		std::deque<std::pair<Vulkan::Fence *, size_t>> __pendingFences;

		Infra::Event<size_t> __finishEvent;

		[[nodiscard]]
		Vulkan::Fence *__getIdleFence();

		[[nodiscard]]
		Vulkan::Fence *__createFence();
	};

	constexpr Infra::EventView<size_t> &CommandSubmitter::getFinishEvent() noexcept
	{
		return __finishEvent;

	}
}