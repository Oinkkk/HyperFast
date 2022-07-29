#pragma once

#include "Deleter.h"
#include <queue>

namespace Infra
{
	class TemporalDeleter final : public Unique
	{
	public:
		TemporalDeleter() noexcept;
		~TemporalDeleter() noexcept;

		void advanceTimestamp() noexcept;

		void reserve(Deletable *const pDeletable) noexcept;
		void commit(const size_t timestamp);

	private:
		size_t __oldestTimestamp{};
		Deleter *__pCurrentDeleter{};
		std::deque<Deleter *> __pendingDeleters;
		std::deque<Deleter *> __idleDeleters;
	};
}
