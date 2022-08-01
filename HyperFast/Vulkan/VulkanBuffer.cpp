#include "VulkanBuffer.h"
#include <exception>

namespace Vulkan
{
	VulkanBuffer::VulkanBuffer(Device &device, const VkBufferCreateInfo &createInfo) :
		Handle{ __create(device, createInfo) }, __device{ device }
	{}

	VulkanBuffer::~VulkanBuffer() noexcept
	{
		__destroy();
	}

	void VulkanBuffer::vkGetBufferMemoryRequirements(
		VkMemoryRequirements *const pMemoryRequirements) noexcept
	{
		__device.vkGetBufferMemoryRequirements(getHandle(), pMemoryRequirements);
	}

	VkResult VulkanBuffer::vkBindBufferMemory(
		const VkDeviceMemory memory, const VkDeviceSize memoryOffset) noexcept
	{
		return __device.vkBindBufferMemory(getHandle(), memory, memoryOffset);
	}

	void VulkanBuffer::__destroy() noexcept
	{
		__device.vkDestroyBuffer(getHandle(), nullptr);
	}

	VkBuffer VulkanBuffer::__create(Device &device, const VkBufferCreateInfo &createInfo)
	{
		VkBuffer retVal{};
		device.vkCreateBuffer(&createInfo, nullptr, &retVal);

		if (!retVal)
			throw std::exception{ "Cannot create a VkBuffer." };

		return retVal;
	}
}