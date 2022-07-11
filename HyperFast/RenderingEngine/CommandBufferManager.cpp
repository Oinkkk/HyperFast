#include "CommandBufferManager.h"

namespace HyperFast
{
	CommandBufferManager::CommandBufferManager(
		Vulkan::Device &device, const uint32_t queueFamilyIndex,
		const size_t numMaxBuffers) noexcept :
		__device{ device }, __queueFamilyIndex { queueFamilyIndex },
		__numMaxBuffers{numMaxBuffers}
	{
		__createCommandPool();
		__allocateCommandBuffers();
	}

	Vulkan::CommandBuffer &CommandBufferManager::getNextBuffer() noexcept
	{
		if (__cursor >= __numMaxBuffers)
		{
			__pCommandPool->vkResetCommandPool(0U);
			__cursor = 0ULL;
		}

		Vulkan::CommandBuffer &retVal{ *__commandBuffers[__cursor] };
		__cursor++;

		return retVal;
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
				nullptr, VkCommandBufferLevel::VK_COMMAND_BUFFER_LEVEL_PRIMARY,
				uint32_t(__numMaxBuffers), handles.data())
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