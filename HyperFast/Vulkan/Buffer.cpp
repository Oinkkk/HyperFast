#include "Buffer.h"
#include <exception>

namespace Vulkan
{
	Buffer::Buffer(Device &device, const VkBufferCreateInfo &createInfo) :
		Handle{ __create(device, createInfo) }, __device{ device }
	{}

	Buffer::~Buffer() noexcept
	{
		__destroy();
	}

	void Buffer::vkGetBufferMemoryRequirements(
		VkMemoryRequirements *const pMemoryRequirements) noexcept
	{
		__device.vkGetBufferMemoryRequirements(getHandle(), pMemoryRequirements);
	}

	VkResult Buffer::vkBindBufferMemory(
		const VkDeviceMemory memory, const VkDeviceSize memoryOffset) noexcept
	{
		return __device.vkBindBufferMemory(getHandle(), memory, memoryOffset);
	}

	void Buffer::__destroy() noexcept
	{
		__device.vkDestroyBuffer(getHandle(), nullptr);
	}

	VkBuffer Buffer::__create(Device &device, const VkBufferCreateInfo &createInfo)
	{
		VkBuffer retVal{};
		device.vkCreateBuffer(&createInfo, nullptr, &retVal);

		if (!retVal)
			throw std::exception{ "Cannot create a VkBuffer." };

		return retVal;
	}
}