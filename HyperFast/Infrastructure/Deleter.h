#pragma once

#include "Deletable.h"
#include <vector>
#include <unordered_set>

namespace Infra
{
	class Deleter final : public Unique
	{
	public:
		~Deleter() noexcept;

		void reserve(Deletable *const pDeletable) noexcept;
		void commit();

	private:
		std::vector<Deletable *> __reservationQueue;
		std::unordered_set<Deletable *> __reservedObjects;
	};
}
