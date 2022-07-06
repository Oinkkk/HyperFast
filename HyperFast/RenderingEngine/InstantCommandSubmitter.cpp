#include "InstantCommandSubmitter.h"

namespace HyperFast
{
	InstantCommandSubmitter::InstantCommandSubmitter(
		const VkDevice device, const VKL::DeviceProcedure &deviceProc,
		const uint32_t queueFamilyIndex, const VkQueue queue) noexcept :
		__device{ device }, __deviceProc{ deviceProc },
		__queueFamilyIndex{ queueFamilyIndex }, __queue{ queue }
	{
		__createCommandPool();
	}

	InstantCommandSubmitter::~InstantCommandSubmitter() noexcept
	{
		__destroyFences();
		__destroyCommandPool();
	}

	void InstantCommandSubmitter::begin() noexcept
	{
		if (__initState)
			return;

		static const VkCommandBufferBeginInfo beginInfo
		{
			.sType = VkStructureType::VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
			.flags = VkCommandBufferUsageFlagBits::VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
		};

		__retrieveResources();
		__deviceProc.vkBeginCommandBuffer(__currentCommandBuffer, &beginInfo);
		__pCurrentPromise = std::make_unique<std::promise<void>>();
		__currentFuture = __pCurrentPromise->get_future();

		__initState = true;
	}

	void InstantCommandSubmitter::end() noexcept
	{
		if (__initState)
			return;

		__deviceProc.vkEndCommandBuffer(__currentCommandBuffer);
	}

	void InstantCommandSubmitter::submit() noexcept
	{
		if (__initState)
			return;

		const VkCommandBufferSubmitInfo commandBufferInfo
		{
			.sType = VkStructureType::VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
			.commandBuffer = __currentCommandBuffer
		};

		const VkSubmitInfo2 submitInfo
		{
			.sType = VkStructureType::VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
			.commandBufferInfoCount = 1U,
			.pCommandBufferInfos = &commandBufferInfo
		};

		__deviceProc.vkQueueSubmit2(__queue, 1U, &submitInfo, __currentFence);

		__busyResources.emplace(
			__currentFence, std::make_pair(__currentCommandBuffer, std::move(__pCurrentPromise)));

		__currentFuture = {};
	}

	void InstantCommandSubmitter::refresh() noexcept
	{
		for (auto iter = __busyResources.begin(); iter != __busyResources.end(); )
		{
			const VkFence fence{ iter->first };
			const auto &[commandBuffer, pPromise] { iter->second };
			
			if (__isFinished(fence))
			{
				__resetFence(fence);
				__resetCommandBuffer(commandBuffer);
				pPromise->set_value();

				__idleResources.emplace_back(fence, commandBuffer);
				iter = __busyResources.erase(iter);
			}
			else
				iter++;
		}
	}

	std::shared_future<void> InstantCommandSubmitter::getCurrentExecution() noexcept
	{
		return __currentFuture;
	}

	void InstantCommandSubmitter::vkCmdCopyBuffer(
		const VkPipelineStageFlags2 srcStageMask, const VkAccessFlags2 srcAccessMask,
		const VkPipelineStageFlags2 dstStageMask, const VkAccessFlags2 dstAccessMask,
		const VkBuffer srcBuffer, const VkBuffer dstBuffer,
		const uint32_t regionCount, const VkBufferCopy *const pRegions) noexcept
	{
		std::vector<VkBufferMemoryBarrier2> memoryBarriers;
		memoryBarriers.reserve(regionCount);

		for (uint32_t regionIter = 0U; regionIter < regionCount; regionIter++)
		{
			const VkBufferCopy &region{ pRegions[regionIter] };

			VkBufferMemoryBarrier2 &memoryBarrier{ memoryBarriers.emplace_back() };
			memoryBarrier.sType = VkStructureType::VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2;
			memoryBarrier.srcStageMask = srcStageMask;
			memoryBarrier.srcAccessMask = srcAccessMask;
			memoryBarrier.dstStageMask = dstStageMask;
			memoryBarrier.dstAccessMask = dstAccessMask;
			memoryBarrier.buffer = dstBuffer;
			memoryBarrier.offset = region.dstOffset;
			memoryBarrier.size = region.size;
		}

		const VkDependencyInfo barrierInfo
		{
			.sType = VkStructureType::VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
			.bufferMemoryBarrierCount = uint32_t(memoryBarriers.size()),
			.pBufferMemoryBarriers = memoryBarriers.data()
		};

		__deviceProc.vkCmdPipelineBarrier2(__currentCommandBuffer, &barrierInfo);
		__deviceProc.vkCmdCopyBuffer(__currentCommandBuffer, srcBuffer, dstBuffer, regionCount, pRegions);

		__initState = false;
	}

	void InstantCommandSubmitter::__createCommandPool()
	{
		const VkCommandPoolCreateInfo createInfo
		{
			.sType = VkStructureType::VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
			.flags = VkCommandPoolCreateFlagBits::VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
			.queueFamilyIndex = __queueFamilyIndex
		};

		__deviceProc.vkCreateCommandPool(__device, &createInfo, nullptr, &__commandPool);
		if (!__commandPool)
			throw std::exception{ "Cannot create the main command pool." };
	}

	void InstantCommandSubmitter::__destroyCommandPool() noexcept
	{
		__deviceProc.vkDestroyCommandPool(__device, __commandPool, nullptr);
	}

	void InstantCommandSubmitter::__destroyFences() noexcept
	{
		for (const VkFence fence : __fences)
			__deviceProc.vkDestroyFence(__device, fence, nullptr);
	}

	void InstantCommandSubmitter::__retrieveResources()
	{
		if (__idleResources.empty())
		{
			__currentFence = __createFence();
			__currentCommandBuffer = __allocateCommandBuffer();
		}
		else
		{
			const auto [fence, commandBuffer] { __idleResources.front() };
			__currentFence = fence;
			__currentCommandBuffer = commandBuffer;

			__idleResources.pop_front();
		}
	}

	VkFence InstantCommandSubmitter::__createFence()
	{
		const VkFenceCreateInfo createInfo
		{
			.sType = VkStructureType::VK_STRUCTURE_TYPE_FENCE_CREATE_INFO
		};

		VkFence retVal{};
		__deviceProc.vkCreateFence(__device, &createInfo, nullptr, &retVal);

		if (!retVal)
			throw std::exception{ "Cannot create a VkFence" };

		__fences.emplace_back(retVal);
		return retVal;
	}

	VkCommandBuffer InstantCommandSubmitter::__allocateCommandBuffer()
	{
		const VkCommandBufferAllocateInfo allocInfo
		{
			.sType = VkStructureType::VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
			.commandPool = __commandPool,
			.level = VkCommandBufferLevel::VK_COMMAND_BUFFER_LEVEL_PRIMARY,
			.commandBufferCount = 1U
		};

		VkCommandBuffer retVal{};
		__deviceProc.vkAllocateCommandBuffers(__device, &allocInfo, &retVal);

		if (!retVal)
			throw std::exception{ "Cannot allocate a VkCommandBuffer" };

		return retVal;
	}

	bool InstantCommandSubmitter::__isFinished(const VkFence fence) const noexcept
	{
		const VkResult result{ __deviceProc.vkWaitForFences(__device, 1U, &fence, VK_TRUE, 0ULL) };
		return (result == VkResult::VK_SUCCESS);
	}

	void InstantCommandSubmitter::__resetFence(const VkFence fence) noexcept
	{
		__deviceProc.vkResetFences(__device, 1U, &fence);
	}

	void InstantCommandSubmitter::__resetCommandBuffer(const VkCommandBuffer commandBuffer) noexcept
	{
		__deviceProc.vkResetCommandBuffer(commandBuffer, 0U);
	}
}