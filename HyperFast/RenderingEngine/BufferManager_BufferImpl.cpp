#include "BufferManager.h"
#include <exception>

namespace HyperFast
{
	BufferManager::BufferImpl::BufferImpl(
		Vulkan::Device &device, Infra::TemporalDeleter &resourceDeleter,
		const VkDeviceSize size, const VkBufferUsageFlags usage) :
		__device{ device }, __resourceDeleter{ resourceDeleter },
		__size{ size }, __usage{ usage }
	{
		__createBuffer(size, usage);
		__queryMemoryRequirements();
	}

	BufferManager::BufferImpl::~BufferImpl() noexcept
	{
		__resourceDeleter.reserve(__pBuffer);
	}

	VkBuffer BufferManager::BufferImpl::getHandle() noexcept
	{
		return __pBuffer->getHandle();
	}

	void BufferManager::BufferImpl::bindMemory(
		const std::shared_ptr<Memory> &pMemory, const VkDeviceAddress offset) noexcept
	{
		__pMemory = pMemory;
		__memoryOffset = offset;

		if (pMemory)
		{
			__pBuffer->vkBindBufferMemory(
				pMemory->getBank(), pMemory->getOffset() + offset);
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

		__pBuffer = new Vulkan::VulkanBuffer{ __device, createInfo };
	}

	void BufferManager::BufferImpl::__queryMemoryRequirements() noexcept
	{
		__pBuffer->vkGetBufferMemoryRequirements(&__memRequirements);
 	}
}