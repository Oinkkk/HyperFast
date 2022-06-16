#pragma once

#include "BufferManager.h"

namespace HyperFast
{
	class Buffer : public Infra::Unique
	{
	public:
		Buffer(BufferManager &manager, const VkDeviceSize memSize, const VkBufferUsageFlags usage);
		~Buffer() noexcept;

	private:
		BufferManager &__manager;
		BufferManager::BufferImpl *const __pImpl;
	};
}
