#include "VulkanMemory.h"
#include <exception>

namespace Vulkan
{
	VulkanMemory::VulkanMemory(Device &device, const VkMemoryAllocateInfo &allocInfo) :
		Handle{ __alloc(device, allocInfo) }, __device{ device }
	{}

	VulkanMemory::~VulkanMemory() noexcept
	{
		__destroy();
	}

	VkResult VulkanMemory::vkMapMemory(
		const VkDeviceSize offset, const VkDeviceSize size,
		const VkMemoryMapFlags flags, void **const ppData) noexcept
	{
		return __device.vkMapMemory(getHandle(), offset, size, flags, ppData);
	}

	void VulkanMemory::vkUnmapMemory() noexcept
	{
		__device.vkUnmapMemory(getHandle());
	}

	void VulkanMemory::__destroy() noexcept
	{
		__device.vkFreeMemory(getHandle(), nullptr);
	}

	VkDeviceMemory VulkanMemory::__alloc(Device &device, const VkMemoryAllocateInfo &allocInfo)
	{
		VkDeviceMemory retVal{};
		device.vkAllocateMemory(&allocInfo, nullptr, &retVal);

		if (!retVal)
			throw std::exception{ "Cannot allocate a VkDeviceMemory." };

		return retVal;
	}
}