#pragma once

#include "Deletable.h"
#include <vector>

namespace Infra
{
	class Deleter final : public Unique
	{
	public:
		~Deleter() noexcept;

		void reserve(Deletable *const pDeletable) noexcept;
		void commit();

	private:
		std::vector<Deletable *> __deleteReserved;
	};
}
