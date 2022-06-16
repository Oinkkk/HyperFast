#include "Buffer.h"

namespace HyperFast
{
	Buffer::Buffer(BufferManager &manager, const VkDeviceSize memSize, const VkBufferUsageFlags usage) :
		__manager{ manager }, __pImpl{ manager.create(memSize, usage) }
	{}

	Buffer::~Buffer() noexcept
	{
		__manager.destroy(__pImpl);
	}
}