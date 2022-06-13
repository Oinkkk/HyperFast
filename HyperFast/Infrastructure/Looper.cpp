#include "Looper.h"
#include "Timer.h"

namespace Infra
{
	Looper::~Looper() noexcept
	{
		stop();
	}

	void Looper::start(const LoopFunc &loopFunc)
	{
		stop();

		__loopFlag = true;
		__loopThread = std::thread
		{
			[this, loopFunc]
			{
				Timer<> timer;
				std::unique_lock lock{ __messageMutex, std::defer_lock };
				std::vector<Message> messagePlaceholder;

				while (__loopFlag)
				{
					lock.lock();

					if (!(__messageQueue.empty()))
						messagePlaceholder.swap(__messageQueue);

					lock.unlock();

					timer.end();
					const float elaped{ timer.getElapsed() };
					timer.start();

					loopFunc(messagePlaceholder, elaped);
					messagePlaceholder.clear();
				}
			}
		};
	}

	void Looper::stop() noexcept
	{
		if (!__loopFlag)
			return;

		__loopFlag = false;
		__loopThread.join();
	}
}