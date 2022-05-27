#pragma once

#include "Unique.h"
#include <cstdint>
#include <queue>

namespace Infra
{
	class IdAllocator : public Unique
	{
	public:
		using id_t = uint64_t;

		[[nodiscard]]
		id_t allocate() noexcept;
		void free(const id_t id) noexcept;

	private:
		std::queue<id_t> __unusedIds;
		id_t __nextId{};
	};
}
