#include "IdAllocator.h"

namespace Infra
{
	IdAllocator::id_t IdAllocator::allocate() noexcept
	{
		id_t retVal;

		if (__unusedIds.empty())
		{
			retVal = __nextId;
			__nextId++;
		}
		else
		{
			retVal = __unusedIds.front();
			__unusedIds.pop();
		}

		return retVal;
	}

	void IdAllocator::free(const id_t id) noexcept
	{
		__unusedIds.emplace(id);
	}
}