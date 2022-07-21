#include "SemaphoreDependency.h"

namespace HyperFast
{
	bool SemaphoreDependency::isIdle() noexcept
	{
		return __manager.isIdle(__groupId);
	}
}