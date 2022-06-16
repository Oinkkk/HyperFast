#include "BufferManager.h"
#include <exception>

namespace HyperFast
{
	BufferManager::BufferImpl::BufferImpl(
		const VkDevice device, const VKL::DeviceProcedure &deviceProc, MemoryManager &memoryManager,
		const VkDeviceSize dataSize, const VkBufferUsageFlags usage) :
		__device{ device }, __deviceProc{ deviceProc }, __memoryManager{ memoryManager }
	{
		__createBuffer(dataSize, usage);
		__queryMemoryRequirements();
		__allocMemory();
	}

	BufferManager::BufferImpl::~BufferImpl() noexcept
	{
		__freeMemory();
		__destroyBuffer();
	}

	void BufferManager::BufferImpl::__createBuffer(const VkDeviceSize dataSize, const VkBufferUsageFlags usage)
	{
		const VkBufferCreateInfo createInfo
		{
			.sType = VkStructureType::VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
			.size = dataSize,
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

	void BufferManager::BufferImpl::__queryMemoryRequirements() noexcept
	{
		__deviceProc.vkGetBufferMemoryRequirements(__device, __buffer, &__memRequirements);
 	}

	void BufferManager::BufferImpl::__allocMemory()
	{
		__pMemory = std::make_unique<Memory>(
			__memoryManager, __memRequirements.size,
			__memRequirements.alignment, __memRequirements.memoryTypeBits);
	}

	void BufferManager::BufferImpl::__freeMemory() noexcept
	{
		__pMemory = nullptr;
	}
}