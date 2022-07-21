#include "BufferManager.h"

namespace HyperFast
{
	BufferManager::BufferManager(Vulkan::Device &device, Infra::EventView<> &gcEvent) noexcept :
		__device{ device }
	{
		__pGCEventListener = Infra::EventListener<>::bind(&BufferManager::__onGarbageCollect, this);
		gcEvent += __pGCEventListener;
	}

	BufferManager::~BufferManager() noexcept
	{
		for (BufferImpl *const pImpl : __destroyReserved)
			delete pImpl;
	}

	[[nodiscard]]
	BufferManager::BufferImpl *BufferManager::create(const VkDeviceSize size, const VkBufferUsageFlags usage)
	{
		return new BufferManager::BufferImpl{ __device, size, usage };
	}

	void BufferManager::destroy(BufferImpl *const pImpl) noexcept
	{
		__destroyReserved.emplace_back(pImpl);
	}

	void BufferManager::__onGarbageCollect() noexcept
	{
		for (auto iter = __destroyReserved.begin(); iter != __destroyReserved.end(); )
		{
			BufferImpl *const pImpl{ *iter };
			
			if (pImpl->isIdle())
			{
				delete pImpl;
				iter = __destroyReserved.erase(iter);
				continue;
			}

			iter++;
		}
	}
}