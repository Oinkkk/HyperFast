#include "CommandBufferManager.h"

namespace HyperFast
{
	CommandBufferManager::CommandBufferManager(
		Vulkan::Device &device, const uint32_t queueFamilyIndex,
		const VkCommandBufferLevel level, const size_t numMaxBuffers) noexcept :
		__device{ device }, __queueFamilyIndex { queueFamilyIndex },
		__level{ level }, __numMaxBuffers{ numMaxBuffers }
	{
		__createCommandPool();
		__allocateCommandBuffers();
	}

	void CommandBufferManager::advance() noexcept
	{
		if (__cursor >= __numMaxBuffers)
		{
			__pCommandPool->vkResetCommandPool(0U);
			__cursor = 0ULL;
		}
		else
			__cursor++;
	}

	Vulkan::CommandBuffer &CommandBufferManager::get() noexcept
	{
		return *(__commandBuffers[__cursor]);
	}

	void CommandBufferManager::__createCommandPool()
	{
		const VkCommandPoolCreateInfo createInfo
		{
			.sType = VkStructureType::VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
			.queueFamilyIndex = __queueFamilyIndex
		};

		__pCommandPool = std::make_unique<Vulkan::CommandPool>(__device, createInfo);
	}

	void CommandBufferManager::__allocateCommandBuffers()
	{
		std::vector<VkCommandBuffer> handles;
		handles.resize(__numMaxBuffers);

		const VkResult result
		{
			__pCommandPool->vkAllocateCommandBuffers(
				nullptr, __level, uint32_t(__numMaxBuffers), handles.data())
		};

		if (result != VK_SUCCESS)
			throw std::exception{ "Cannot allocate command buffers." };

		for (const VkCommandBuffer handle : handles)
		{
			__commandBuffers.emplace_back(
				std::make_unique<Vulkan::CommandBuffer>(__device, handle));
		}
	}
}