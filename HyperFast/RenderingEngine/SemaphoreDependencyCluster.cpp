#include "SemaphoreDependencyCluster.h"

namespace HyperFast
{
	void SemaphoreDependencyCluster::add(const std::shared_ptr<SemaphoreDependency> &pDependency) noexcept
	{
		__dependencies.emplace(pDependency.get(), pDependency);
	}

	bool SemaphoreDependencyCluster::isIdle() noexcept
	{
		for (
			auto dependencyIter = __dependencies.begin();
			dependencyIter != __dependencies.end(); )
		{
			SemaphoreDependency *const pDependency{ dependencyIter->first };
			if (!(pDependency->isIdle()))
				return false;

			dependencyIter = __dependencies.erase(dependencyIter);
		}

		return true;
	}
}