#pragma once

#include "Unique.h"

namespace Infra
{
	template <typename $Handle>
	class ResourceAllocator : public Unique
	{
	public:
		ResourceAllocator() = default;
		virtual ~ResourceAllocator() noexcept = default;

		virtual void free(const $Handle handle) noexcept = 0;
	};
}
