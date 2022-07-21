#include "BufferManager.h"
#include <exception>

namespace HyperFast
{
	BufferManager::BufferImpl::BufferImpl(
		Vulkan::Device &device, const VkDeviceSize size, const VkBufferUsageFlags usage) :
		__device{ device }, __size{ size }, __usage{ usage }
	{
		__createBuffer(size, usage);
		__queryMemoryRequirements();
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

	void BufferManager::BufferImpl::addSemaphoreDependency(
		const std::shared_ptr<SemaphoreDependency> &pDependency) noexcept
	{
		__semaphoreDependencyCluster.add(pDependency);
	}

	bool BufferManager::BufferImpl::isIdle() noexcept
	{
		return __semaphoreDependencyCluster.isIdle();
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

		__pBuffer = std::make_unique<Vulkan::Buffer>(__device, createInfo);
	}

	void BufferManager::BufferImpl::__queryMemoryRequirements() noexcept
	{
		__pBuffer->vkGetBufferMemoryRequirements(&__memRequirements);
 	}
}