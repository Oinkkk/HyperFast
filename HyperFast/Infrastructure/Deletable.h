#pragma once

#include "Unique.h"

namespace Infra
{
	class Deletable : public Unique
	{
	public:
		virtual ~Deletable() noexcept = default;
	};
}
