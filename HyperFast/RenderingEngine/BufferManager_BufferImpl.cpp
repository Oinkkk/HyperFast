#include "BufferManager.h"
#include <exception>

namespace HyperFast
{
	BufferManager::BufferImpl::BufferImpl(
		const VkDevice device, const VKL::DeviceProcedure &deviceProc,
		const VkDeviceSize size, const VkBufferUsageFlags usage) :
		__device{ device }, __deviceProc{ deviceProc }, __size{ size }, __usage{ usage }
	{
		__createBuffer(size, usage);
		__queryMemoryRequirements();
	}

	BufferManager::BufferImpl::~BufferImpl() noexcept
	{
		__destroyBuffer();
	}

	void BufferManager::BufferImpl::bindMemory(
		const std::shared_ptr<Memory> &pMemory, const VkDeviceAddress offset) noexcept
	{
		__pMemory = pMemory;
		if (pMemory)
		{
			__deviceProc.vkBindBufferMemory(
				__device, __buffer, pMemory->getBank(), pMemory->getOffset() + offset);
		}
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
}