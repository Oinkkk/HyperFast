#include "Looper.h"
#include "Timer.h"

namespace Infra
{
	MessageLooper::~MessageLooper() noexcept
	{
		stop();
	}

	void MessageLooper::start(const MessageFunc &messageFunc)
	{
		stop();

		__loopFlag = true;
		__loopThread = std::thread
		{
			[this, messageFunc]
			{
				std::unique_lock conditionLock{ __conditionMutex, std::defer_lock };
				std::vector<Message> messagePlaceholder;

				while (true)
				{
					conditionLock.lock();
					__emptyCondition.wait(conditionLock, [this]
					{
						return !(__loopFlag && __messageQueue.empty());
					});

					if (!__loopFlag)
						break;

					messagePlaceholder.swap(__messageQueue);
					conditionLock.unlock();

					for (const Message &message : messagePlaceholder)
						messageFunc(message.id, message.arguments);

					messagePlaceholder.clear();
				}
			}
		};
	}

	void MessageLooper::stop() noexcept
	{
		if (!__loopFlag)
			return;

		std::unique_lock conditionLock{ __conditionMutex };
		__loopFlag = false;
		conditionLock.unlock();

		__emptyCondition.notify_all();
		__loopThread.join();
	}

	UpdateLooper::~UpdateLooper() noexcept
	{
		stop();
	}

	void UpdateLooper::start(const MessageFunc &messageFunc, const UpdateFunc &updateFunc)
	{
		stop();

		__loopFlag = true;
		__loopThread = std::thread
		{
			[this, messageFunc, updateFunc]
			{
				Timer<> timer;
				std::unique_lock conditionLock{ __conditionMutex, std::defer_lock };
				std::vector<Message> messagePlaceholder;

				while (__loopFlag)
				{
					conditionLock.lock();

					if (!(__messageQueue.empty()))
						messagePlaceholder.swap(__messageQueue);

					conditionLock.unlock();

					for (const Message &message : messagePlaceholder)
						messageFunc(message.id, message.arguments);

					messagePlaceholder.clear();

					timer.end();
					const float elaped{ timer.getElapsed() };
					timer.start();

					updateFunc(elaped);
				}
			}
		};
	}

	void UpdateLooper::stop() noexcept
	{
		if (!__loopFlag)
			return;

		std::unique_lock conditionLock{ __conditionMutex };
		__loopFlag = false;
		conditionLock.unlock();

		__loopThread.join();
	}
}