#pragma once

#include "SemaphoreDependencyManager.h"

namespace HyperFast
{
	class SemaphoreDependency : public Infra::Unique
	{
	public:
		constexpr SemaphoreDependency(SemaphoreDependencyManager &manager) noexcept;

		[[nodiscard]]
		constexpr uint64_t getGroupId() const noexcept;

		[[nodiscard]]
		bool isIdle() noexcept;

	private:
		SemaphoreDependencyManager &__manager;
		const uint64_t __groupId;
	};

	constexpr SemaphoreDependency::SemaphoreDependency(SemaphoreDependencyManager &manager) noexcept :
		__manager{ manager }, __groupId{ manager.getCurrentGroupId() }
	{}

	constexpr uint64_t SemaphoreDependency::getGroupId() const noexcept
	{
		return __groupId;
	}
}