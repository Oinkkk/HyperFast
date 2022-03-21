#include "ConcurrentMessageQueue.h"

namespace Infra
{
	std::vector<ConcurrentMessageQueue::Message> ConcurrentMessageQueue::dequeueMessages() noexcept
	{
		std::vector<Message> retVal;

		std::unique_lock lck{ __mutex };
		retVal.swap(__messageQueue);
		lck.unlock();

		return retVal;
	}
}