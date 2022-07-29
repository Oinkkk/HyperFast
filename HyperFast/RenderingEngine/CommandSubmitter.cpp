#include "CommandSubmitter.h"

namespace HyperFast
{
	CommandSubmitter::CommandSubmitter(Vulkan::Device &device, Vulkan::Queue &queue) noexcept :
		__device{ device }, __queue { queue }
	{}

	void CommandSubmitter::enqueue(
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
	}

	void CommandSubmitter::submit() noexcept
	{
		__infoStream.clear();

		for (auto &[_, infos] : __layer2InfosMap)
		{
			__infoStream.insert(__infoStream.end(), infos.begin(), infos.end());
			infos.clear();
		}

		if (__infoStream.empty())
			return;

		// deletion queue 모듈 추가
		__queue.vkQueueSubmit2(uint32_t(__infoStream.size()), __infoStream.data(), VK_NULL_HANDLE);
	}
}