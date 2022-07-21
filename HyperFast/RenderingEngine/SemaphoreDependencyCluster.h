#pragma once

#include "SemaphoreDependency.h"
#include <memory>

namespace HyperFast
{
	class SemaphoreDependencyCluster : public Infra::Unique
	{
	public:
		void add(const std::shared_ptr<SemaphoreDependency> &pDependency) noexcept;

		[[nodiscard]]
		bool isIdle() noexcept;

	private:
		std::unordered_map<SemaphoreDependency *, std::shared_ptr<SemaphoreDependency>> __dependencies;
	};
}