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
		if (__reservedObjects.contains(pDeletable))
			return;

		__reservationQueue.emplace_back(pDeletable);
		__reservedObjects.emplace(pDeletable);
	}

	void Deleter::commit()
	{
		std::vector<Deletable *> placeholder;

		while (!(__reservationQueue.empty()))
		{
			placeholder.swap(__reservationQueue);

			for (Deletable *const pDeletable : placeholder)
				delete pDeletable;

			placeholder.clear();
		}

		__reservedObjects.clear();
	}
}