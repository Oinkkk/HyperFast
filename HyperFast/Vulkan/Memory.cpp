#include "Memory.h"
#include <exception>

namespace Vulkan
{
	Memory::Memory(Device &device, const VkMemoryAllocateInfo &allocInfo) :
		Handle{ __alloc(device, allocInfo) }, __device{ device }
	{}

	Memory::~Memory() noexcept
	{
		__destroy();
	}

	VkResult Memory::vkMapMemory(
		const VkDeviceSize offset, const VkDeviceSize size,
		const VkMemoryMapFlags flags, void **const ppData) noexcept
	{
		return __device.vkMapMemory(getHandle(), offset, size, flags, ppData);
	}

	void Memory::vkUnmapMemory() noexcept
	{
		__device.vkUnmapMemory(getHandle());
	}

	void Memory::__destroy() noexcept
	{
		__device.vkFreeMemory(getHandle(), nullptr);
	}

	VkDeviceMemory Memory::__alloc(Device &device, const VkMemoryAllocateInfo &allocInfo)
	{
		VkDeviceMemory retVal{};
		device.vkAllocateMemory(&allocInfo, nullptr, &retVal);

		if (!retVal)
			throw std::exception{ "Cannot allocate a VkDeviceMemory." };

		return retVal;
	}
}