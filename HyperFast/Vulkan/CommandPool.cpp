#include "CommandPool.h"
#include <exception>

namespace Vulkan
{
	CommandPool::CommandPool(
		Device &device, const VkCommandPoolCreateInfo &createInfo) :
		Handle{ __create(device, createInfo) }, __device{ device }
	{}

	CommandPool::~CommandPool() noexcept
	{
		__destroy();
	}

	VkResult CommandPool::vkResetCommandPool(const VkCommandPoolResetFlags flags) noexcept
	{
		return __device.vkResetCommandPool(getHandle(), flags);
	}

	VkResult CommandPool::vkAllocateCommandBuffers(
		const void *const pNext, const VkCommandBufferLevel level,
		const uint32_t commandBufferCount, VkCommandBuffer *const pCommandBuffers) noexcept
	{
		const VkCommandBufferAllocateInfo allocInfo
		{
			.sType = VkStructureType::VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
			.pNext = pNext,
			.commandPool = getHandle(),
			.level = level,
			.commandBufferCount = commandBufferCount
		};

		return __device.vkAllocateCommandBuffers(&allocInfo, pCommandBuffers);
	}

	void CommandPool::__destroy() noexcept
	{
		__device.vkDestroyCommandPool(getHandle(), nullptr);
	}

	VkCommandPool CommandPool::__create(
		Device &device, const VkCommandPoolCreateInfo &createInfo)
	{
		VkCommandPool retVal{};
		device.vkCreateCommandPool(&createInfo, nullptr, &retVal);

		if (!retVal)
			throw std::exception{ "Cannot create a VkCommandPool." };

		return retVal;
	}
}