#include "SemaphoreDependency.h"

namespace HyperFast
{
	void SemaphoreDependencyManager::setDependency(Vulkan::Semaphore &semaphore, const uint64_t value) noexcept
	{
		__dependencyGroups[__currentGroupId][&semaphore] = value;
	}

	bool SemaphoreDependencyManager::isIdle(const uint64_t groupId) noexcept
	{
		const auto foundIt{ __dependencyGroups.find(groupId) };
		if (foundIt == __dependencyGroups.end())
			return true;

		DependencyGroup &group{ foundIt->second };
		for (auto iter = group.begin(); iter != group.end(); )
		{
			const auto &[pSemaphore, value] { *iter };
			const VkResult waitResult{ pSemaphore->wait(value, 0ULL) };

			if (waitResult != VkResult::VK_SUCCESS)
				return false;

			iter = group.erase(iter);
		}

		__dependencyGroups.erase(foundIt);
		return true;
	}
}