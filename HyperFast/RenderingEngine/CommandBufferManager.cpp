#include "CommandBufferManager.h"

namespace HyperFast
{
	CommandBufferManager::CommandBufferManager(
		Vulkan::Device &device, const uint32_t queueFamilyIndex,
		const size_t numMaxBuffers) noexcept :
		__queueFamilyIndex{ queueFamilyIndex }, __numMaxBuffers{ numMaxBuffers }
	{
		__createCommandPool(device);
		__allocateCommandBuffers();
	}

	VkCommandBuffer CommandBufferManager::getNextBuffer()
	{
		if (__cursor >= __numMaxBuffers)
		{
			__pCommandPool->vkResetCommandPool(0U);
			__cursor = 0ULL;
		}

		const VkCommandBuffer retVal{ __commandBuffers[__cursor] };
		__cursor++;

		return retVal;
	}

	void CommandBufferManager::__createCommandPool(Vulkan::Device &device)
	{
		const VkCommandPoolCreateInfo createInfo
		{
			.sType = VkStructureType::VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
			.queueFamilyIndex = __queueFamilyIndex
		};

		__pCommandPool = std::make_unique<Vulkan::CommandPool>(device, &createInfo);
	}

	void CommandBufferManager::__allocateCommandBuffers()
	{
		__commandBuffers.resize(__numMaxBuffers);

		const VkResult result
		{
			__pCommandPool->vkAllocateCommandBuffers(
				nullptr, VkCommandBufferLevel::VK_COMMAND_BUFFER_LEVEL_PRIMARY,
				uint32_t(__numMaxBuffers), __commandBuffers.data())
		};

		if (result != VK_SUCCESS)
			throw std::exception{ "Cannot allocate command buffers." };
	}
}