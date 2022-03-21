#pragma once

#include "Unique.h"
#include <vector>
#include <any>
#include <mutex>

namespace Infra
{
	class ConcurrentMessageQueue : public Unique
	{
	public:
		class Message
		{
		public:
			uint64_t id;
			std::vector<std::any> arguments;

			template <typename ...$Args>
			Message(const uint64_t id, $Args &&...args);
		};

		template <typename ...$Args>
		void enqueueMessage(const uint64_t id, $Args &&...args);
		std::vector<Message> dequeueMessages() noexcept;

		[[nodiscard]]
		constexpr bool isEmpty() const noexcept;

	private:
		std::mutex __mutex;
		std::vector<Message> __messageQueue;
	};

	template <typename ...$Args>
	ConcurrentMessageQueue::Message::Message(const uint64_t id, $Args &&...args) :
		id{ id }
	{
		(arguments.emplace_back(std::forward<$Args>(args)), ...);
	}

	template <typename ...$Args>
	void ConcurrentMessageQueue::enqueueMessage(const uint64_t id, $Args &&...args)
	{
		std::lock_guard lck{ __mutex };
		__messageQueue.emplace_back(id, std::forward<$Args>(args)...);
	}

	constexpr bool ConcurrentMessageQueue::isEmpty() const noexcept
	{
		return __messageQueue.empty();
	}
}
