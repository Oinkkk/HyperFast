#pragma once

#include "Unique.h"

namespace Infra
{
	template <typename $Handle>
	class ResourceCreater : public Unique
	{
	public:
		ResourceCreater() = default;
		virtual ~ResourceCreater() noexcept = default;

		virtual void destroy(const $Handle handle) noexcept = 0;
	};
}
