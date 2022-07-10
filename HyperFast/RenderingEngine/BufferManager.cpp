#include "BufferManager.h"

namespace HyperFast
{
	BufferManager::BufferManager(Vulkan::Device &device) noexcept :
		__device{ device }
	{}

	[[nodiscard]]
	BufferManager::BufferImpl *BufferManager::create(
		const VkDeviceSize size, const VkBufferUsageFlags usage)
	{
		return new BufferManager::BufferImpl{ __device, size, usage };
	}

	void BufferManager::destroy(BufferImpl *const pImpl) noexcept
	{
		delete pImpl;
	}
}