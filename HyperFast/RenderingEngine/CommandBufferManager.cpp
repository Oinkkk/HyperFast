#include "CommandBufferManager.h"

namespace HyperFast
{
	CommandBufferManager::CommandBufferManager(
		const VkDevice device, const Vulkan::DeviceProcedure &deviceProc,
		const uint32_t queueFamilyIndex, const size_t numMaxBuffers) noexcept :
		__device{ device }, __deviceProc{ deviceProc },
		__queueFamilyIndex{ queueFamilyIndex }, __numMaxBuffers{ numMaxBuffers }
	{
		__createCommandPool();
		__allocateCommandBuffers();
	}

	CommandBufferManager::~CommandBufferManager() noexcept
	{
		__destroyCommandPool();
	}

	VkCommandBuffer CommandBufferManager::getNextBuffer()
	{
		if (__cursor >= __numMaxBuffers)
		{
			__deviceProc.vkResetCommandPool(__device, __commandPool, 0U);
			__cursor = 0ULL;
		}

		const VkCommandBuffer retVal{ __commandBuffers[__cursor] };
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

		__deviceProc.vkCreateCommandPool(__device, &createInfo, nullptr, &__commandPool);
		if (!__commandPool)
			throw std::exception{ "Cannot create the main command pool." };
	}

	void CommandBufferManager::__destroyCommandPool() noexcept
	{
		__deviceProc.vkDestroyCommandPool(__device, __commandPool, nullptr);
	}

	void CommandBufferManager::__allocateCommandBuffers()
	{
		const VkCommandBufferAllocateInfo allocInfo
		{
			.sType = VkStructureType::VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
			.commandPool = __commandPool,
			.level = VkCommandBufferLevel::VK_COMMAND_BUFFER_LEVEL_PRIMARY,
			.commandBufferCount = uint32_t(__numMaxBuffers)
		};

		__commandBuffers.resize(__numMaxBuffers);

		const VkResult result{ __deviceProc.vkAllocateCommandBuffers(__device, &allocInfo, __commandBuffers.data()) };
		if (result != VK_SUCCESS)
			throw std::exception{ "Cannot allocate command buffers." };
	}
}