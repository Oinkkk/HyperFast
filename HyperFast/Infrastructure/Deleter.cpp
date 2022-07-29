#include "Deleter.h"
#include <thread>

namespace Infra
{
	Deleter::~Deleter() noexcept
	{
		commit();
	}

	void Deleter::reserve(Deletable *const pDeletable) noexcept
	{
		__deleteReserved.emplace_back(pDeletable);
	}

	void Deleter::commit()
	{
		std::vector<Deletable *> placeholder;

		while (!(__deleteReserved.empty()))
		{
			placeholder.swap(__deleteReserved);

			for (Deletable *const pDeletable : placeholder)
				delete pDeletable;

			placeholder.clear();
		}
	}
}