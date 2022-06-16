#include "BufferManager.h"
#include <exception>

namespace HyperFast
{
	BufferManager::BufferImpl::BufferImpl(
		const VkDevice device, const VKL::DeviceProcedure &deviceProc,
		const VkDeviceSize memSize, const VkBufferUsageFlags usage) :
		__device{ device }, __deviceProc{ deviceProc }
	{
		__createBuffer(memSize, usage);
	}

	BufferManager::BufferImpl::~BufferImpl() noexcept
	{
		__destroyBuffer();
	}

	void BufferManager::BufferImpl::__createBuffer(const VkDeviceSize memSize, const VkBufferUsageFlags usage)
	{
		const VkBufferCreateInfo createInfo
		{
			.sType = VkStructureType::VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
			.size = memSize,
			.usage = usage,
			.sharingMode = VkSharingMode::VK_SHARING_MODE_EXCLUSIVE
		};

		__deviceProc.vkCreateBuffer(__device, &createInfo, nullptr, &__buffer);
		if (!__buffer)
			throw std::exception{ "Cannot create a VkBuffer." };
	}

	void BufferManager::BufferImpl::__destroyBuffer() noexcept
	{
		__deviceProc.vkDestroyBuffer(__device, __buffer, nullptr);
	}
}