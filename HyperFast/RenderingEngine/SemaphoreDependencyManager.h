#pragma once

#include "../Vulkan/Semaphore.h"
#include <unordered_set>
#include <unordered_map>

namespace HyperFast
{
	class SemaphoreDependencyManager : public Infra::Unique
	{
	public:
		void setDependency(Vulkan::Semaphore &semaphore, const uint64_t value) noexcept;
		constexpr void nextGroup() noexcept;

		[[nodiscard]]
		constexpr uint64_t getCurrentGroupId() const noexcept;

		[[nodiscard]]
		bool isIdle(const uint64_t groupId) noexcept;

	private:
		using DependencyGroup = std::unordered_map<Vulkan::Semaphore *, uint64_t>;
		
		std::unordered_map<uint64_t, DependencyGroup> __dependencyGroups;
		uint64_t __currentGroupId{};
	};

	constexpr void SemaphoreDependencyManager::nextGroup() noexcept
	{
		__currentGroupId++;
	}

	constexpr uint64_t SemaphoreDependencyManager::getCurrentGroupId() const noexcept
	{
		return __currentGroupId;
	}
}