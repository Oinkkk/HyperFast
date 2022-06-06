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

	using MessageFunc = std::function<void(const uint64_t, const std::vector<std::any> &)>;
	using UpdateFunc = std::function<void(const float)>;

	class MessageLooper final : public Unique
	{
	public:
		MessageLooper() = default;
		~MessageLooper() noexcept;

		template <typename ...$Args>
		void enqueueMessage(const uint64_t id, $Args &&...args);

		void start(const MessageFunc &messageFunc);
		void stop() noexcept;

	private:
		std::vector<Message> __messageQueue;
		std::mutex __conditionMutex;
		std::condition_variable __emptyCondition;

		bool __loopFlag{};
		std::thread __loopThread;
	};

	class UpdateLooper final : public Unique
	{
	public:
		UpdateLooper() = default;
		~UpdateLooper() noexcept;

		template <typename ...$Args>
		void enqueueMessage(const uint64_t id, $Args &&...args);

		void start(const MessageFunc &messageFunc, const UpdateFunc &updateFunc);
		void stop() noexcept;

	private:
		std::vector<Message> __messageQueue;
		std::mutex __conditionMutex;

		bool __loopFlag;
		std::thread __loopThread;
	};

	template <typename ...$Args>
	Message::Message(const uint64_t id, $Args &&...args) : id{ id }
	{
		(arguments.emplace_back(std::forward<$Args>(args)), ...);
	}

	template <typename ...$Args>
	void MessageLooper::enqueueMessage(const uint64_t id, $Args &&...args)
	{
		std::unique_lock conditionLock{ __conditionMutex };
		__messageQueue.emplace_back(id, std::forward<$Args>(args)...);
		conditionLock.unlock();

		__emptyCondition.notify_one();
	}

	template <typename ...$Args>
	void UpdateLooper::enqueueMessage(const uint64_t id, $Args &&...args)
	{
		std::unique_lock conditionLock{ __conditionMutex };
		__messageQueue.emplace_back(id, std::forward<$Args>(args)...);
		conditionLock.unlock();
	}
}
