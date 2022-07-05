#include "Buffer.h"

namespace HyperFast
{
	Buffer::Buffer(BufferManager &manager, const VkDeviceSize size, const VkBufferUsageFlags usage) :
		__manager{ manager }, __pImpl{ manager.create(size, usage) }
	{}

	Buffer::~Buffer() noexcept
	{
		__manager.destroy(__pImpl);
	}

	void Buffer::bindMemory(const std::shared_ptr<Memory> &pMemory, const VkDeviceAddress offset) noexcept
	{
		__pImpl->bindMemory(pMemory, offset);
	}
}