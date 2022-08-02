#include "Buffer.h"

namespace HyperFast
{
	Buffer::Buffer(BufferManager &manager, const VkDeviceSize size, const VkBufferUsageFlags usage) :
		__pImpl{ manager.create(size, usage) }
	{}

	VkBuffer Buffer::getHandle() noexcept
	{
		return __pImpl->getHandle();
	}

	void Buffer::bindMemory(const std::shared_ptr<Memory> &pMemory, const VkDeviceAddress offset) noexcept
	{
		__pImpl->bindMemory(pMemory, offset);
	}
}