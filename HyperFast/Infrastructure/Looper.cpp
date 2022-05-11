#include "Looper.h"

namespace Infra
{
	Looper::~Looper() noexcept
	{
		stop();
	}

	void Looper::start(const MessageFunc &messageFunc, const UpdateFunc &updateFunc)
	{
		stop();

		__loopFlag = true;
		__loopThread = std::thread
		{
			[this, messageFunc, updateFunc]
			{
				while (__loopFlag)
				{
					if (!(__messageQueue.isEmpty()))
					{
						const std::vector<ConcurrentMessageQueue::Message> &messages{ __messageQueue.dequeueMessages() };
						for (const ConcurrentMessageQueue::Message &message : messages)
							messageFunc(message.id, message.arguments);
					}

					__timer.end();
					const float elaped{ __timer.getElapsed() };
					__timer.start();

					updateFunc(elaped);
				}
			}
		};
	}

	void Looper::stop()
	{
		if (!__loopFlag)
			return;

		__loopFlag = false;
		__loopThread.join();

		__timer.reset();
	}
}