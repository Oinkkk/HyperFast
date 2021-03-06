#include "CommandSubmitter.h"

namespace HyperFast
{
	CommandSubmitter::CommandSubmitter(Vulkan::Device &device, Vulkan::Queue &queue) noexcept :
		__device{ device }, __queue{ queue }
	{}

	CommandSubmitter::~CommandSubmitter() noexcept
	{
		flush();

		for (const auto &[pFence, _] : __pendingFences)
			delete pFence;

		for (Vulkan::Fence *const pFence : __idleFences)
			delete pFence;
	}

	void CommandSubmitter::enqueue(
		const uint32_t waitSemaphoreInfoCount,
		const VkSemaphoreSubmitInfo *const pWaitSemaphoreInfos,
		const uint32_t commandBufferInfoCount,
		const VkCommandBufferSubmitInfo *const pCommandBufferInfos,
		const uint32_t signalSemaphoreInfoCount,
		const VkSemaphoreSubmitInfo *pSignalSemaphoreInfos) noexcept
	{
		VkSubmitInfo2 &infoPlaceholder{ __submitInfos.emplace_back() };
		infoPlaceholder.sType = VkStructureType::VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
		infoPlaceholder.waitSemaphoreInfoCount = waitSemaphoreInfoCount;
		infoPlaceholder.pWaitSemaphoreInfos = pWaitSemaphoreInfos;
		infoPlaceholder.commandBufferInfoCount = commandBufferInfoCount;
		infoPlaceholder.pCommandBufferInfos = pCommandBufferInfos;
		infoPlaceholder.signalSemaphoreInfoCount = signalSemaphoreInfoCount;
		infoPlaceholder.pSignalSemaphoreInfos = pSignalSemaphoreInfos;
	}

	bool CommandSubmitter::submit()
	{
		if (__submitInfos.empty())
			return false;

		Vulkan::Fence *const pFence{ __getIdleFence() };
		__queue.vkQueueSubmit2(uint32_t(__submitInfos.size()), __submitInfos.data(), pFence->getHandle());

		__submitInfos.clear();
		__pendingFences.emplace_back(pFence, __timestamp);
		__timestamp++;

		return true;
	}

	void CommandSubmitter::flush() noexcept
	{
		bool valid{};
		size_t lastTimestamp{};

		while (!(__pendingFences.empty()))
		{
			const auto [pFence, timestamp] { __pendingFences.front() };

			const VkResult result{ pFence->wait(0ULL) };
			if (result != VkResult::VK_SUCCESS)
				break;

			pFence->reset();
			valid = true;
			lastTimestamp = timestamp;

			__pendingFences.pop_front();
			__idleFences.emplace_back(pFence);
		}

		if (valid)
			__finishEvent.invoke(lastTimestamp);
	}

	Vulkan::Fence *CommandSubmitter::__getIdleFence()
	{
		Vulkan::Fence *pRetVal{};

		if (__idleFences.empty())
			pRetVal = __createFence();
		else
		{
			pRetVal = __idleFences.front();
			__idleFences.pop_front();
		}

		return pRetVal;
	}

	Vulkan::Fence *CommandSubmitter::__createFence()
	{
		static constexpr VkFenceCreateInfo createInfo
		{
			.sType = VkStructureType::VK_STRUCTURE_TYPE_FENCE_CREATE_INFO
		};

		return new Vulkan::Fence{ __device, createInfo };
	}
}