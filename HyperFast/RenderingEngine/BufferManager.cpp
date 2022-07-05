#include "BufferManager.h"

namespace HyperFast
{
	BufferManager::BufferManager(
		const VkDevice device, const VKL::DeviceProcedure &deviceProc) noexcept :
		__device{ device }, __deviceProc{ deviceProc }
	{}

	[[nodiscard]]
	BufferManager::BufferImpl *BufferManager::create(const VkDeviceSize size, const VkBufferUsageFlags usage)
	{
		return new BufferManager::BufferImpl{ __device, __deviceProc, size, usage };
	}

	void BufferManager::destroy(BufferImpl *const pImpl) noexcept
	{
		delete pImpl;
	}
}