#pragma once

#include "Unique.h"
#include "ConcurrentMessageQueue.h"
#include "Timer.h"
#include <functional>

namespace Infra
{
	class Looper : public Unique
	{
	public:
		using InitFunc = std::function<void()>;
		using MessageFunc = std::function<void(const uint64_t, const std::vector<std::any> &)>;
		using UpdateFunc = std::function<void(const float)>;
		using EndFunc = std::function<void()>;

		virtual ~Looper() noexcept;

		template <typename ...$Args>
		void enqueueMessage(const uint64_t id, $Args &&...args);

		void start(
			const InitFunc &initFunc, const MessageFunc &messageFunc,
			const UpdateFunc &updateFunc, const EndFunc &endFunc);

		void stop();

	private:
		ConcurrentMessageQueue __messageQueue;

		bool __loopFlag;
		std::thread __loopThread;

		Timer<> __timer;
	};

	template <typename ...$Args>
	void Looper::enqueueMessage(const uint64_t id, $Args &&...args)
	{
		__messageQueue.enqueueMessage(id, std::forward<$Args>(args)...);
	}
}
