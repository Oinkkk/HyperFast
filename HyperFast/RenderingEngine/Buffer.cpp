#include "Buffer.h"

namespace HyperFast
{
	Buffer::Buffer(BufferManager &manager, const VkDeviceSize dataSize, const VkBufferUsageFlags usage) :
		__manager{ manager }, __pImpl{ manager.create(dataSize, usage) }
	{}

	Buffer::~Buffer() noexcept
	{
		__manager.destroy(__pImpl);
	}
}