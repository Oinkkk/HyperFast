#include "CommandBufferManager.h"

namespace HyperFast
{
	CommandBufferManager::CommandBufferManager(
		Vulkan::Device &device, const uint32_t queueFamilyIndex,
		const VkCommandBufferLevel level, Infra::TemporalDeleter &resourceDeleter) noexcept :
		__device{ device }, __queueFamilyIndex{ queueFamilyIndex },
		__level{ level }, __resourceDeleter{ resourceDeleter }
	{
		__createCommandPools();
		__allocateCommandBuffers();
	}

	CommandBufferManager::~CommandBufferManager() noexcept
	{
		for (Vulkan::CommandPool *const pCommandPool : __commandPools)
			__resourceDeleter.reserve(pCommandPool);
	}

	void CommandBufferManager::advance() noexcept
	{
		const uint32_t prevCursor{ __cursor };
		__cursor = ((__cursor + 1U) % __totalNumCommandBuffers);

		const uint32_t prevCommandPoolIdx{ prevCursor / __numCommandBuffersPerPool };
		const uint32_t curCommandPoolIdx{ __cursor / __numCommandBuffersPerPool };

		if (prevCommandPoolIdx != curCommandPoolIdx)
			__commandPools[curCommandPoolIdx]->vkResetCommandPool(0U);
	}

	Vulkan::CommandBuffer &CommandBufferManager::get() noexcept
	{
		return *(__commandBuffers[__cursor]);
	}

	void CommandBufferManager::__createCommandPools()
	{
		const VkCommandPoolCreateInfo createInfo
		{
			.sType = VkStructureType::VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
			.queueFamilyIndex = __queueFamilyIndex
		};

		for (Vulkan::CommandPool *&pCommandPool : __commandPools)
			pCommandPool = new Vulkan::CommandPool{ __device, createInfo };
	}

	void CommandBufferManager::__allocateCommandBuffers()
	{
		VkCommandBuffer handles[__numCommandBuffersPerPool];

		size_t commandBufferIter{};
		for (auto &pCommandPool : __commandPools)
		{
			const VkResult result
			{
				pCommandPool->vkAllocateCommandBuffers(
					nullptr, __level, __numCommandBuffersPerPool, handles)
			};

			if (result != VK_SUCCESS)
				throw std::exception{ "Cannot allocate command buffers." };

			for (const VkCommandBuffer handle : handles)
			{
				__commandBuffers[commandBufferIter] =
					std::make_unique<Vulkan::CommandBuffer>(__device, handle);

				commandBufferIter++;
			}
		}
	}
}