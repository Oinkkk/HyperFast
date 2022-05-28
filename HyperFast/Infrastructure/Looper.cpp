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
				std::unique_lock emptyConditionLock{ __emptyConditionMutex };

				while (true)
				{
					__emptyCondition.wait(emptyConditionLock, [this]
					{
						return !(__loopFlag && __messageQueue.empty());
					});

					if (!__loopFlag)
						break;

					for (const Message &message : __messageQueue)
						messageFunc(message.id, message.arguments);

					__messageQueue.clear();
				}
			}
		};
	}

	void MessageLooper::stop() noexcept
	{
		if (!__loopFlag)
			return;

		std::unique_lock emptyConditionLock{ __emptyConditionMutex };
		__loopFlag = false;
		emptyConditionLock.unlock();
		__emptyCondition.notify_one();
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
				std::unique_lock messageLock{ __messageMutex, std::defer_lock };

				while (__loopFlag)
				{
					messageLock.lock();

					if (!(__messageQueue.empty()))
					{
						for (const Message &message : __messageQueue)
							messageFunc(message.id, message.arguments);

						__messageQueue.clear();
					}

					messageLock.unlock();

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

		std::unique_lock messageLock{ __messageMutex };
		__loopFlag = false;
		messageLock.unlock();
		__loopThread.join();
	}
}