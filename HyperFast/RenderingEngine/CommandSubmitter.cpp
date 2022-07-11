#include "CommandSubmitter.h"

namespace HyperFast
{
	CommandSubmitter::CommandSubmitter(Vulkan::Device &device, Vulkan::Queue &queue) noexcept :
		__device{ device }, __queue { queue }
	{
		__appendSubmitFence();
		__currentSubmitFuture = __fence2PromiseMap[__getCurrentSubmitFence()].get_future();
	}

	std::shared_future<void> CommandSubmitter::enqueue(
		const SubmitLayerType layerType,
		const uint32_t waitSemaphoreInfoCount,
		const VkSemaphoreSubmitInfo *const pWaitSemaphoreInfos,
		const uint32_t commandBufferInfoCount,
		const VkCommandBufferSubmitInfo *const pCommandBufferInfos,
		const uint32_t signalSemaphoreInfoCount,
		const VkSemaphoreSubmitInfo *pSignalSemaphoreInfos) noexcept
	{
		VkSubmitInfo2 &infoPlaceholder{ __layer2InfosMap[layerType].emplace_back() };
		infoPlaceholder.sType = VkStructureType::VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
		infoPlaceholder.waitSemaphoreInfoCount = waitSemaphoreInfoCount;
		infoPlaceholder.pWaitSemaphoreInfos = pWaitSemaphoreInfos;
		infoPlaceholder.commandBufferInfoCount = commandBufferInfoCount;
		infoPlaceholder.pCommandBufferInfos = pCommandBufferInfos;
		infoPlaceholder.signalSemaphoreInfoCount = signalSemaphoreInfoCount;
		infoPlaceholder.pSignalSemaphoreInfos = pSignalSemaphoreInfos;

		return __currentSubmitFuture;
	}

	void CommandSubmitter::submit()
	{
		__infoStream.clear();

		for (auto &[_, infos] : __layer2InfosMap)
		{
			__infoStream.insert(__infoStream.end(), infos.begin(), infos.end());
			infos.clear();
		}

		if (__infoStream.empty())
			return;

		Vulkan::Fence *const pSubmitFence{ __getCurrentSubmitFence() };
		__queue.vkQueueSubmit2(uint32_t(__infoStream.size()), __infoStream.data(), pSubmitFence->getHandle());

		__nextSubmitFenceIdx();
	}

	Vulkan::Fence *CommandSubmitter::__getCurrentSubmitFence() noexcept
	{
		return __submitFences[__currentSubmitFenceIdx].get();
	}

	void CommandSubmitter::__appendSubmitFence()
	{
		const VkFenceCreateInfo createInfo
		{
			.sType = VkStructureType::VK_STRUCTURE_TYPE_FENCE_CREATE_INFO
		};

		__submitFences.emplace_back(std::make_unique<Vulkan::Fence>(__device, createInfo));
	}

	void CommandSubmitter::__nextSubmitFenceIdx()
	{
		const size_t numFences{ __submitFences.size() };
		bool found{};

		for (
			size_t fenceIter = ((__currentSubmitFenceIdx + 1ULL) % numFences);
			fenceIter != __currentSubmitFenceIdx;
			fenceIter = ((fenceIter + 1ULL) % numFences))
		{
			const VkResult waitResult{ __submitFences[fenceIter]->wait(0ULL) };
			if (waitResult == VkResult::VK_TIMEOUT)
				continue;

			__currentSubmitFenceIdx = fenceIter;
			found = true;
			break;
		}

		if (found)
		{
			Vulkan::Fence *const pNextSubmitFence{ __getCurrentSubmitFence() };
			pNextSubmitFence->reset();
			__fence2PromiseMap.extract(pNextSubmitFence).mapped().set_value();
		}
		else
		{
			__currentSubmitFenceIdx = __submitFences.size();
			__appendSubmitFence();
		}

		__currentSubmitFuture = __fence2PromiseMap[__getCurrentSubmitFence()].get_future();
	}
}