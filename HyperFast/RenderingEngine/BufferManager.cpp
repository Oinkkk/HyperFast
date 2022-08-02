#include "BufferManager.h"

namespace HyperFast
{
	BufferManager::BufferManager(Vulkan::Device &device, Infra::TemporalDeleter &resourceDeleter) noexcept :
		__device{ device }, __resourceDeleter{ resourceDeleter }
	{}

	std::unique_ptr<BufferManager::BufferImpl> BufferManager::create(
		const VkDeviceSize size, const VkBufferUsageFlags usage)
	{
		return std::make_unique<BufferImpl>(__device, __resourceDeleter, size, usage);
	}
}