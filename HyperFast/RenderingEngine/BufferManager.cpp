#include "BufferManager.h"

namespace HyperFast
{
	BufferManager::BufferManager(Vulkan::Device &device, Infra::Deleter &resourceDeleter) noexcept :
		__device{ device }, __resourceDeleter{ resourceDeleter }
	{}

	[[nodiscard]]
	BufferManager::BufferImpl *BufferManager::create(
		const VkDeviceSize size, const VkBufferUsageFlags usage)
	{
		return new BufferManager::BufferImpl{ __device, size, usage };
	}

	void BufferManager::destroy(BufferImpl *const pImpl) noexcept
	{
		__resourceDeleter.reserve(pImpl);
	}
}