#pragma once

#include "Unique.h"
#include <vector>
#include <any>
#include <functional>
#include <condition_variable>

namespace Infra
{
	class Message
	{
	public:
		uint64_t id;
		std::vector<std::any> arguments;

		template <typename ...$Args>
		Message(const uint64_t id, $Args &&...args);
	};

	using LoopFunc = std::function<void(const std::vector<Message> &, const float)>;

	class Looper final : public Unique
	{
	public:
		Looper() = default;
		~Looper() noexcept;

		template <typename ...$Args>
		void enqueueMessage(const uint64_t id, $Args &&...args);

		void start(const LoopFunc &loopFunc);
		void stop() noexcept;

	private:
		std::vector<Message> __messageQueue;
		std::mutex __messageMutex;

		bool __loopFlag;
		std::thread __loopThread;
	};

	template <typename ...$Args>
	Message::Message(const uint64_t id, $Args &&...args) : id{ id }
	{
		(arguments.emplace_back(std::forward<$Args>(args)), ...);
	}

	template <typename ...$Args>
	void Looper::enqueueMessage(const uint64_t id, $Args &&...args)
	{
		std::lock_guard lock{ __messageMutex };
		__messageQueue.emplace_back(id, std::forward<$Args>(args)...);
	}
}
